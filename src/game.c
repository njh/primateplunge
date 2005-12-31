/*
 *  game.c
 *  Primate Plunge
 *
 *  Created by Joseph Humfrey on Fri Jul 18 2003.
 *  Copyright (c) 2003 aelius productions. All rights reserved.
 *
 *  Licenced under uDevGame LICENSE Version 4b1
 *  See included COPYING file for details
 *
 */

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>

#include "game.h"

#define START_PLATFORM 5
#define sound Mix_Chunk
#define debugLevelProperties 1

extern SDL_Surface *mainScreen;


/* TEMP - testing with slow processor */
int slowProcessor=0;

/* when SDL_Quit is found anywhere, any system (menu or game) has to be able to quit completely */
int quitApp=0;

/* Menu variables */
menuName currentMenu = MAIN;
int gamePaused=0;

/* Toggling sound/music */
int soundOnOff=1;
int musicOnOff=1;

/* Gameplay variables */
int framePeriod;
float levelDistance;
float levelDY;
int lastFrameTime;
int worldCompleteTime;

/* All worlds */
world* worlds[32];
int numWorlds=0;	// Number tallied up as worlds are loaded
world* currentWorld;

platform platforms[9];
int lastCorrodeTime=0;	// for corroding platforms
int warned=0;	// ticking clock before powerup runs out
int diedTime;	// so that moving level distance knows where to draw when you die
int keyDownLeftArrow=0;
int keyDownRightArrow=0;
int keyDownDownArrow=0;
int keyDownUpArrow=0;
int keyDownEscape=0;
int keyDownActionKey=0;

/* The number of stars the player has completed the whole game on */
int starMedalLevel=0;

/* Powerup gameplay variables */
powerupType currentPowerup = NO_POWERUP;
int powerupElapsedTime=0;		// if player has paused the game a few times, it is not enough to simply store the start time
int powerupStartTime=0;
int powerupPickupY=-1;			// Location of pickup-able powerup, -1 for both if there isn't one on screen
int prevPickupY=0;
powerupType powerupPickup=NO_POWERUP;	// type of powerup available for pickup
int powerupLastFrameTime=0;
int powerupAnimFrame=0;
    
/* Player position/fall speed */
float playerX;
float playerY;
float playerDY;
int playerOn=1;				// Whether player is falling or standing
int playerAbove=START_PLATFORM;		// Platform player is just above, whether standing or falling
int playerAnimFrame=0;			// Frame number of current frame of animation
int playerLastFrameTime=0;		// Time player's animation was last updated
int playerHealth=10;

/* Music */
Mix_Music* titleMusic;
sound* gameMusic;
int musicLoopChannel=-1;

/* Sounds */
sound* death;
sound* jetpack;
sound* parachuteSnd;
sound* timeUp;
sound* congrats;
sound* congratsEnding;
sound* getStarRating;
sound* interface;
sound* powerupBeanSnd;
sound* powerupHealthSnd;
sound* powerupParachuteSnd;
sound* powerupJetpackSnd;
sound* starRating;
soundSet ah;
soundSet boing;
soundSet ouch;
soundSet ouchSmall;
soundSet land;
int jetpackChannel=-1;	// sound loops, so we must know when to stop it
int ambientSndChannel=-1;
int parachuteOpen=0;	// so sound doesn't play more than once

/* Character animations */
animation walkRightAnim;
animation walkLeftAnim;
animation fallRightAnim;
animation fallLeftAnim;
animation fallAnim;
animation monkeyJetAnim;
animation standAnim;
animation* playerAnimation;

/* Powerup animations */
animation healAnim;
animation paraBagAnim;
animation jetpackAnim;
animation jumpAnim;
animation shieldAnim;
SDL_Surface* parachuteImg;

/* Game interface items */
SDL_Surface* healthBarFull;
SDL_Surface* healthBarEmpty;
SDL_Surface* worldUnlocked;
SDL_Surface* plunged;
SDL_Surface* pausedImg;
SDL_Surface* clockImg;
animation numbersMapAnim;
int nextStarTarget;
int starCompleteTime;

/* Menu system items */
SDL_Surface* logoImg;
SDL_Surface* newGameImg;
SDL_Surface* newGameOverImg;
SDL_Surface* optionsImg;
SDL_Surface* optionsOverImg;
SDL_Surface* instructionsImg;
SDL_Surface* instructionsOverImg;
SDL_Surface* quitImg;
SDL_Surface* quitOverImg;
SDL_Surface* resetImg;
SDL_Surface* resetOverImg;
SDL_Surface* backImg;
SDL_Surface* backOverImg;
SDL_Surface* yesImg;
SDL_Surface* yesOverImg;
SDL_Surface* cancelImg;
SDL_Surface* cancelOverImg;
SDL_Surface* sureImg;
SDL_Surface* soundImg;
SDL_Surface* soundOverImg;
SDL_Surface* musicImg;
SDL_Surface* musicOverImg;
SDL_Surface* onImg;
SDL_Surface* offImg;

SDL_Surface* loadingImg;
SDL_Surface* instructions;
SDL_Surface* instructions2;

SDL_Surface* gameCompleteScreen;
SDL_Surface* starRating1Screen;
SDL_Surface* starRating2Screen;
SDL_Surface* starRating3Screen;
SDL_Surface* starRating4Screen;
SDL_Surface* starRating5Screen;

SDL_Surface* prevArrowImg;
SDL_Surface* nextArrowImg;
SDL_Surface* backtoMainImg;
SDL_Surface* lockedImg;
SDL_Surface* recordImg;
SDL_Surface* needImg;
SDL_Surface* needCompleteImg;
SDL_Surface* starImg;
SDL_Surface* medalImg;
int mouseOverOption=1;
int mouseX;
int mouseY;

SDL_Surface* voteMonkey;


/* Special function needed when reading 4 byte ints on a windows machine since
   endianess is different */
int readInt(FILE* filePointer, int* n)
{
    char* charPointer, temp;
    
    /* load n the wrong way round*/
    //if(fread(n,4,1,filePointer) != 1) return 0;
    fread(n,4,1,filePointer);
    
    /* Find the first byte of n */
    charPointer = (char*)n;
    
    /* Swap 'em around */
    temp = charPointer[0];
    charPointer[0] = charPointer[3];    // first and last
    charPointer[3] = temp;
    temp = charPointer[1];
    charPointer[1] = charPointer[2];    // middle two
    charPointer[2] = temp;
    
    return 1;
}

/* Main load function loads all game data */
void loadGame( void )
{
    char formatString[7];
    FILE* prefsScoresFile;
    char* filePath;
    char hash;
    int i;
    
    /*
        Show loading screen
    */
    loadingImg = loadGraphic("loading.bmp");
    SDL_FillRect(mainScreen, NULL, SDL_MapRGB(mainScreen->format, 0, 0, 0)); //clear screen
    drawGraphic(loadingImg, 0, 160);
    SDL_FreeSurface(loadingImg);
    SDL_Flip(mainScreen);
    
    /*
        Sounds
    */
    death = loadSound("MonkeyDeath2.wav");
    jetpack = loadSound("JetPack.wav");
    parachuteSnd = loadSound("Parachute.wav");
    timeUp = loadSound("timeUp.wav");
    congrats = loadSound("Congratulations.wav");
    congratsEnding = loadSound("CongratsEnding.wav");
    getStarRating = loadSound("GetJewel.wav");
    interface = loadSound("InterfaceSelect.wav");
    powerupBeanSnd = loadSound("PowerUpBean.wav");
    powerupHealthSnd = loadSound("PowerUpHealth.wav");
    powerupParachuteSnd = loadSound("PowerUpParachute.wav");
    powerupJetpackSnd = loadSound("PowerUpJet.wav");
    starRating = loadSound("StarRating.wav");
    CreateSoundSet(&ah, 1);
        AddSoundToSet("ah/MonkeyAh4.wav", &ah);
    CreateSoundSet(&boing, 3);
        AddSoundToSet("boing/Boing1.wav", &boing);
        AddSoundToSet("boing/Boing2.wav", &boing);
        AddSoundToSet("boing/Boing3.wav", &boing);
    CreateSoundSet(&ouch, 3);
        AddSoundToSet("bigHit/MonkeyBigHit1.wav", &ouch);
        AddSoundToSet("bigHit/MonkeyBigHit2.wav", &ouch);
        AddSoundToSet("bigHit/MonkeyBigHit3.wav", &ouch);
    CreateSoundSet(&ouchSmall, 3);
        AddSoundToSet("smallHit/MonkeySmallHit1.wav", &ouchSmall);
        AddSoundToSet("smallHit/MonkeySmallHit2.wav", &ouchSmall);
        AddSoundToSet("smallHit/MonkeySmallHit3.wav", &ouchSmall);
    CreateSoundSet(&land, 2);
        AddSoundToSet("land/land1.wav", &land);
        AddSoundToSet("land/land2.wav", &land);
    
    /*
        Music
    */
    titleMusic=Mix_LoadMUS("music/HappySong2.mid");
        if(!titleMusic)
            printf("Mix_LoadMUS(\"music/HappySong2.mid\"): %s\n", Mix_GetError());
    gameMusic = loadSound("../music/gameMusic.wav");
    /* reduce volume of this music because it's too obtrusive
        to be played full volume */
    Mix_VolumeChunk(gameMusic, 100);  
    
    
    /*
        Interface/Menu graphics
    */
    healthBarFull = loadGraphic("healthFull.bmp");
    healthBarEmpty = loadGraphic("healthEmpty.bmp");
    worldUnlocked = loadGraphic("worldunlocked.bmp");
    plunged = loadGraphic("primateplunged.bmp");
    clockImg = loadGraphic("clock.bmp");
    pausedImg = loadGraphic("paused.bmp");
    loadAnimation("numbers.bmp", 10, &numbersMapAnim);
    
    logoImg = loadGraphic("logo.bmp");
    newGameImg = loadGraphic("menuitems/newgame-menuitem.bmp");
    newGameOverImg = loadGraphic("menuitems/newgame-over-menuitem.bmp");
    optionsImg = loadGraphic("menuitems/options-menuitem.bmp");
    optionsOverImg = loadGraphic("menuitems/options-over-menuitem.bmp");
    instructionsImg = loadGraphic("menuitems/instructions-menuitem.bmp");
    instructionsOverImg = loadGraphic("menuitems/instructions-over-menuitem.bmp");
    quitImg = loadGraphic("menuitems/quit-menuitem.bmp");
    quitOverImg = loadGraphic("menuitems/quit-over-menuitem.bmp");
    
    instructions = loadGraphic("instructions.bmp");
    instructions2 = loadGraphic("instructions2.bmp");
    
    gameCompleteScreen = loadGraphic("gameCompleteScreen.bmp");
    starRating1Screen = loadGraphic("starRating1Screen.bmp");
    starRating2Screen = loadGraphic("starRating2Screen.bmp");
    starRating3Screen = loadGraphic("starRating3Screen.bmp");
    starRating4Screen = loadGraphic("starRating4Screen.bmp");
    starRating5Screen = loadGraphic("starRating5Screen.bmp");
    
    resetImg = loadGraphic("menuitems/reset-menuitem.bmp");
    resetOverImg = loadGraphic("menuitems/reset-over-menuitem.bmp");
    backImg = loadGraphic("menuitems/back-menuitem.bmp");
    backOverImg = loadGraphic("menuitems/back-over-menuitem.bmp");
    yesImg = loadGraphic("menuitems/yes-menuitem.bmp");
    yesOverImg = loadGraphic("menuitems/yes-over-menuitem.bmp");
    cancelImg = loadGraphic("menuitems/cancel-menuitem.bmp");
    cancelOverImg = loadGraphic("menuitems/cancel-over-menuitem.bmp");
    sureImg = loadGraphic("menuitems/sure-reset.bmp");
    soundImg = loadGraphic("menuitems/sound-menuitem.bmp");
    soundOverImg = loadGraphic("menuitems/sound-over-menuitem.bmp");
    musicImg = loadGraphic("menuitems/music-menuitem.bmp");
    musicOverImg = loadGraphic("menuitems/music-over-menuitem.bmp"); 
    
    onImg = loadGraphic("on.bmp");
    offImg = loadGraphic("off.bmp");
    
    prevArrowImg = loadGraphic("prev.bmp");
    nextArrowImg = loadGraphic("next.bmp");
    backtoMainImg = loadGraphic("backtomain.bmp");
    lockedImg = loadGraphic("locked.bmp");
    recordImg = loadGraphic("record.bmp");
    needImg = loadGraphic("need.bmp");
    needCompleteImg = loadGraphic("needComplete.bmp");
    starImg = loadGraphic("star.bmp");
    medalImg = loadGraphic("medal.bmp");
    
    voteMonkey = loadGraphic("WindowsThankYou.bmp");
    
    /*
        Character graphics
    */
    loadAnimation("swingRight.bmp", 12, &walkRightAnim);
    loadAnimation("swingLeft.bmp", 12, &walkLeftAnim);
    loadAnimation("fallRight.bmp", 5, &fallRightAnim);
    loadAnimation("fallLeft.bmp", 5, &fallLeftAnim);
    loadAnimation("fall.bmp", 2, &fallAnim);
    loadAnimation("jetpackMonkey.bmp", 2, &monkeyJetAnim);
    loadAnimation("monkeyIdle.bmp", 8, &standAnim);
    
    /*
        Powerup graphics
    */
    loadAnimation("heal.bmp", 4, &healAnim);
    loadAnimation("paraBag.bmp", 1, &paraBagAnim);
    loadAnimation("jetpackPickup.bmp", 1, &jetpackAnim);
    loadAnimation("jumpPickup.bmp", 6, &jumpAnim);
    loadAnimation("shield.bmp", 1, &shieldAnim);
    parachuteImg = loadGraphic("parachute.bmp");
    
    
    /*
        Load the worlds
    */
    loadWorlds();
    currentWorld=NULL;
    
    /*
        Prefs/Scores file
    */
    /* Build path */
    filePath = (char*) malloc(strlen("prefs.dat")+1);
    filePath = "prefs.dat";
    
    /* Open file */
    prefsScoresFile = fopen(filePath, "r");
    
    /* Get rid of malloced space used for filePath */
    free(filePath);
    
    /* if one doesn't exist, create it. */
    if(prefsScoresFile==NULL)
        savePrefsScores(); 
    else
    {
      
        /* Read fileformat string, check format */
        fread(formatString, 1, 6, prefsScoresFile);
        if(strncmp(formatString, "#chaps", 6)==0)
        {
            
            /* Read high scores for each world */
            starMedalLevel=5;
            for(i=0;i<numWorlds;i++)
            {
                int numStars=0;
                
                /* Check # starter */
                fread(&hash, 1, 1, prefsScoresFile);
                if(hash == '#')
                {
                    /* Read 7 char ID code */
                    fread(&worlds[i]->name, 1, 7, prefsScoresFile);
                    
                    /* Read int high score */
                    //readInt(prefsScoresFile, &worlds[i]->highScore);
                    fread(&worlds[i]->highScore, 4, 1, prefsScoresFile);
                }
                //else fprintf(stderr, "Invalid formatting in prefs/scores file\n");
                
                /* Calculate how many stars player has for this world */
                calculateWorldStars(i);
                numStars = worlds[i]->numStars;
                
                /* Calculate player's medal level */
                if(numStars<starMedalLevel) starMedalLevel=numStars;
                
            }
            
            /* Read whether sound is on or off */
            //readInt(prefsScoresFile, &soundOnOff);
            fread(&soundOnOff, 4, 1, prefsScoresFile);
            
            /* Read whether music is on or off */
            //readInt(prefsScoresFile, &musicOnOff);
            fread(&musicOnOff, 4, 1, prefsScoresFile);
            
            /* Close file */
            fclose(prefsScoresFile);
            
        }
        /* Format was incorrect - create new file */
        else
        {
            /* Close file */
            fclose(prefsScoresFile);
            savePrefsScores();
        } // format code correct?
    } // prefs file exists?
    
    /* Seed the random number generator */
    srand(SDL_GetTicks());
}

/* Calculate how many stars player has for this world */
/* If the number of stars has increased, then return the new star rating */
int calculateWorldStars( int worldNumber )
{
    int i;
    int oldStars;
    
    oldStars = worlds[worldNumber]->numStars;
    worlds[worldNumber]->numStars=0;
    
    for(i=0; i<5; i++)
    {
        if( worlds[worldNumber]->highScore >= worlds[worldNumber]->starScore[i] )
            worlds[worldNumber]->numStars=i+1;
        else break;
    }
    
    if(worlds[worldNumber]->numStars > oldStars)
        return worlds[worldNumber]->numStars;
    else return 0;
}

/* Saves prefs and scores into a file in ~/Library/Preferences/com.aelius.primateprefsscores */
void savePrefsScores( void )
{
    FILE* prefsScoresFile;
    char formatString[7] = "#chaps";
    char hash = '#';
    int i;
    char* filePath = (char*) malloc(strlen("prefs.dat")+1);
    
    /* Build path */
    sprintf(filePath, "%s", "prefs.dat");
    
    /* Overwrite any file which already exists there */
    prefsScoresFile = fopen(filePath, "w");
    if(prefsScoresFile==NULL)
    {
        perror("Cound not write to prefs/scores file"); 
        return;
    }
    
    /* Get rid of malloced space used for filePath */
    free(filePath);
        
    /* Write fileformat string */
    fwrite(&formatString[0], 1, 6, prefsScoresFile);
    
    /* Write high scores for each world */
    for(i=0;i<numWorlds;i++)
    {
        /* Write # starter */
        fwrite(&hash, 1, 1, prefsScoresFile);
        
        /* Write 7 char ID code */
        fwrite(&worlds[i]->name, 1, 7, prefsScoresFile);
        
        /* Write int high score */
        fwrite(&worlds[i]->highScore, 4, 1, prefsScoresFile);
    }
    
    /* Write ints sound/music on/off */
    fwrite(&soundOnOff, 4, 1, prefsScoresFile);
    fwrite(&musicOnOff, 4, 1, prefsScoresFile);
    
    /* Close file */
    fclose(prefsScoresFile);
}

/* Initialises a sound set including the array which holds the sound*s */
void CreateSoundSet(soundSet* set, int size)
{
    int i;
    
    set->size = size;
    set->soundArray = (sound**) malloc(sizeof(sound*)*size);
    
    /* Make sure array is initialised to NULL */
    for(i=0;i<size;i++) set->soundArray[i]=NULL;
}

/* Loads a sound effect into a free array space in a sound array */
void AddSoundToSet(char* filepath, soundSet* set)
{
    int i;
    
    /* Find free array slot for sound and load it */
    for(i=0;i<set->size;i++)
        if(set->soundArray[i]==NULL)
        {
            set->soundArray[i] = loadSound(filepath);
            break;
        }
}


/* Loads a sound effect and returns the Mix_Chunk pointer */
sound* loadSound(char* filepath)
{
    sound* sound;
    
    if (filepath==NULL) return NULL;
    
    char fullPath[256]="sounds/";
    
    /* prepend "sounds/" to path */
    strcat(fullPath, filepath);
    
    /* Load sound */
    sound = Mix_LoadWAV(fullPath);
    if(!sound) {
        fprintf(stderr, "Mix_LoadWAV: Failed to load sound file: %s\n", Mix_GetError());
    }

    return sound;
}

/* Loads an image with a background transparency of magenta and returns the SDL_Surface */
SDL_Surface* loadGraphic(char* filepath)
{
    char fullPath[256]="graphics/";
    
    /* Used by SDL */
    Uint32 transparentColour;
    
    /* raw bmp loaded onto a surface. */
    SDL_Surface* rawImage;
    
    /* Final surface */
    SDL_Surface* graphic;
    
    /* prepend "graphics/" to path */
    strcat(fullPath, filepath);
    
    /* Load image */
    rawImage = SDL_LoadBMP(fullPath);
    if(rawImage==NULL) {
        //fprintf(stderr, "Could not find image: %s\n", fullPath);
        printf("Could not find image: %s\n", fullPath);
        return NULL;
    }
    
    /* Set transparent colour (magenta - same for all animations) */
    transparentColour = SDL_MapRGB(rawImage->format, 255, 0, 255);
    SDL_SetColorKey(rawImage, SDL_SRCCOLORKEY, transparentColour);
        
    /* Convert image to the format of the screen */
    graphic = SDL_DisplayFormat(rawImage);
    
    /* Free temporary surface */
    SDL_FreeSurface(rawImage);
    
    return graphic;
}

/*  Loads animation (for character) given a filepath and the number of frames
    filepath must come from a 255 char string */
void loadAnimation(char* filepath, int numFrames, animation* anim)
{
    char fullPath[256]="graphics/";
    
    /* Used by SDL */
    Uint32 transparentColour;
    
    /* raw bmp loaded onto a surface. */
    SDL_Surface* allFramesImage;
    
    /* prepend "graphics/" to path */
    strcat(fullPath, filepath);
    
    /* Load image */
    allFramesImage = SDL_LoadBMP(fullPath);
    if(allFramesImage==NULL) {
        free(anim);
        fprintf(stderr, "Could not find anim image: %s\n", fullPath);
        return;
    }
    
    /* Set transparent colour (magenta - same for all animations) */
    transparentColour = SDL_MapRGB(allFramesImage->format, 255, 0, 255);
    SDL_SetColorKey(allFramesImage, SDL_SRCCOLORKEY, transparentColour);
        
    /* Convert the animation frames to the format of the screen */
    anim->frames = SDL_DisplayFormat(allFramesImage);
    anim->w = allFramesImage->w / numFrames;
    anim->h = allFramesImage->h;
    anim->numFrames = numFrames;
    
    /* Free temporary surface */
    SDL_FreeSurface(allFramesImage);
}

void enterGame()
{

    
    currentWorld=worlds[0];
    
    /*
        Keep showing menu until a new game is started (return 1)
        or quit message is returned (return 0)
    */
    while( enterMenu() && !quitApp )
        startGame();
    
    /* Show "Vote monkey" screen on quit */
    showGraphic(voteMonkey, 10000, 1);
    
}

int enterMenu()
{
    int i;
    int leavingMenu=0;				// how game knows when to quit
    int worldDisplayStartTime=SDL_GetTicks();	// for random background worlds
    int worldChangePeriod = 10000;
    world* newWorld=worlds[0];
    SDL_Event event;
    
    /* Although there isn't much game logic in the menu, this is needed for background scrolling speed */
    lastFrameTime = SDL_GetTicks();
    
    /* Flush event queue */
    while ( SDL_PollEvent(&event) );
    
    /*
        MENU LOOP
    */
    while( !leavingMenu && !quitApp )
    {
        /* Time to change random world? */
        if( worldDisplayStartTime+worldChangePeriod < SDL_GetTicks()
            && currentMenu!=WORLD_CHOICE
            && currentMenu!=INSTRUCTIONS )
        {
            if(numWorlds>1)
                while(newWorld==currentWorld) newWorld=worlds[randomInt(numWorlds)];
            else
                newWorld=currentWorld;
            currentWorld=newWorld;
            worldDisplayStartTime=SDL_GetTicks();
        }
        
        /* Scroll through the background at the right speed */
        framePeriod = (SDL_GetTicks()-lastFrameTime);
        lastFrameTime = SDL_GetTicks();
        if( currentMenu!=INSTRUCTIONS )
            levelDistance += currentWorld->scrollSpeed * framePeriod / 200.0;
        
        /* Draw all parallax layers */
        /* cropped when selecting the world */
        for(i=1; i<=1+slowProcessor*3; i++) // TEMP - emulating slow processor
        {
            if(currentMenu==WORLD_CHOICE)
                drawBackgrounds(1, 0, 7);
            else if(currentMenu!=INSTRUCTIONS)
                drawBackgrounds(0, 0, 7);
        }
        /* draw main menu interface */
        drawMenu();
        
        /* Make sure we are playing the sound loop for this level */
        /* Sound is off when should be on */
        if(ambientSndChannel==-1 && soundOnOff && currentMenu==WORLD_CHOICE)
            ambientSndChannel = Mix_PlayChannel(-1, currentWorld->ambientSnd, -1);
        /* Sound is on when should be off */
        else if((currentMenu!=WORLD_CHOICE || !soundOnOff) && ambientSndChannel!=-1)
        {
            Mix_HaltChannel(ambientSndChannel);
            ambientSndChannel=-1;
        }
    
        
        /* Whether to start a new game or quit or neither */
        switch(getMenuControls())
        {
            case 1: leavingMenu=1; break;
            case 2: leavingMenu=2; break;
            default: break;
        }
        
        /* Menu Music */
        if(!musicOnOff) Mix_FadeOutMusic(500); //stop music if it was toggled
        if(musicOnOff && !Mix_PlayingMusic()) Mix_PlayMusic(titleMusic, 0);
        
        SDL_Flip(mainScreen);
    }
    
    // fade out title music over 500 millisecs when leaving menu
    Mix_FadeOutMusic(500);

    /* Quitting game */
    if(leavingMenu==1) return 0;
    else return 1;
}

/* refresh the screen, set the new menu, flush events */
void changeMenu( menuName theMenu )
{
    SDL_Event event;
    
    /* Clear screen with black */
    //SDL_FillRect(mainScreen, NULL, SDL_MapRGB(mainScreen->format, 0, 0, 0));
    
    /* Set new menu */
    currentMenu = theMenu;
    
    /* Go to first world if going to world selection */
    if( currentMenu==WORLD_CHOICE )
        currentWorld = worlds[0];
        
    /* First menu item is 3 if in 'are you sure' dialog */
    if( currentMenu==SURE_DELETE_SCORES )
        mouseOverOption=3;
    
    /* Flush event queue */
    while ( SDL_PollEvent(&event) );
    
    keyDownLeftArrow=0;
    keyDownRightArrow=0;
    keyDownDownArrow=0;
    keyDownUpArrow=0;
    keyDownEscape=0;
    
    /* Play interface click */
    playSound(interface);
}

/* Show a 300x500 graphic until the player presses a button or the timer times out */
void showGraphic(SDL_Surface* theGraphic, int timeDelay, int enableQuitEvent)
{
    int startExitTime;
    int pressedKey=0;
    SDL_Event event;
    
    startExitTime = SDL_GetTicks();
    drawGraphic(theGraphic, 0, 0);
    SDL_Flip(mainScreen);
    
    /* Flush event queue */
    while ( SDL_PollEvent(&event) );
    
    while((timeDelay==-1 || SDL_GetTicks()-startExitTime < timeDelay) && pressedKey!=1)
    {
        /* Check for key presses */
        while ( SDL_PollEvent(&event) )
        {
            switch (event.type)
            {
                case SDL_KEYDOWN:
                case SDL_MOUSEBUTTONDOWN:
                    pressedKey=1;
                    break;
                case SDL_QUIT:
                    if(enableQuitEvent) pressedKey=1;
                    break;
                default: break;
            }
        }
    }
    
    /* Flush event queue */
    while ( SDL_PollEvent(&event) );
}
    
void drawMenu( void )
{
    /* Main options to draw depend on current menu */
    switch(currentMenu)
    {
        case MAIN:
            /* logo */
            if(logoImg!=NULL)		drawGraphic(logoImg, 150-logoImg->w/2, 50);
            
            /* Medal, if player has one */
            drawMedal();
            
            /* menu items */
            /* New Game */
            if(mouseOverOption==1 && newGameOverImg!=NULL)
                drawGraphic(newGameOverImg, 150-newGameOverImg->w/2, 230);
            else if(newGameImg!=NULL)
                drawGraphic(newGameImg, 150-newGameImg->w/2, 230);
            
            /* Options */
            if(mouseOverOption==2 && optionsOverImg!=NULL)
                drawGraphic(optionsOverImg, 150-optionsOverImg->w/2, 230+50);
            else if(optionsImg!=NULL) 
                drawGraphic(optionsImg, 150-optionsImg->w/2, 230+50);
            
            /* Instructions */
            if(mouseOverOption==3 && instructionsOverImg!=NULL)
                drawGraphic(instructionsOverImg, 150-instructionsOverImg->w/2, 230+100);
            else if(instructionsImg!=NULL)
                drawGraphic(instructionsImg, 150-instructionsImg->w/2, 230+100);
                
            /* Quit */
            if(mouseOverOption==4 && quitOverImg!=NULL)
                drawGraphic(quitOverImg, 150-quitOverImg->w/2, 230+150);
            else if(quitImg!=NULL)
                drawGraphic(quitImg, 150-quitImg->w/2, 230+150);
            
            break;
        
        case WORLD_CHOICE:
            /* personal frame for current world */
            drawGraphic(currentWorld->frameGraphic, 0, 0);
            
            /* prev/next arrows */
            if(currentWorld->order > 0) drawGraphic(prevArrowImg, 15, 245);
            if(currentWorld->order < numWorlds-1) drawGraphic(nextArrowImg, 262, 245);
            
            /* locked or playable? */
            if( worldLocked() ) drawGraphic(lockedImg, 87, 230);
            
            /* Record */
            if(currentWorld->highScore>0)
            {
                drawGraphic(recordImg, 85, 430);
                drawGameNumber(currentWorld->highScore, 164, 421, 0 );
            }
            /* Score player needs to progress to next world */
            else if(currentWorld->completeScore>0 && !worldLocked())
            {
                if(currentWorld->order >= numWorlds-1)
                    drawGraphic(needCompleteImg, 9, 421);
                else
                    drawGraphic(needImg, 53, 416);
                drawGameNumber(currentWorld->completeScore, 164, 421, 0 );
            }
            
            /* Number of stars player has */
            drawStars(452, currentWorld->numStars);
                
            /* back to previous menu */
            drawGraphic(backtoMainImg, 46, 476);
            
            break;
        case OPTIONS:
            /* logo */
            if(logoImg!=NULL)		drawGraphic(logoImg, 150-logoImg->w/2, 50);
            
            /* Medal */
            drawMedal();
            
            /* menu items */
            /* Reset high scores */
            if(mouseOverOption==1 && resetOverImg!=NULL)
                drawGraphic(resetOverImg, 150-resetOverImg->w/2, 230);
            else if(resetImg!=NULL)
                drawGraphic(resetImg, 150-resetImg->w/2, 230);
            
            /* Sound */
            if(soundOnOff)
                drawGraphic(onImg, 80, 230+50);
            else
                drawGraphic(offImg, 80, 230+50);
            if(mouseOverOption==2 && soundOverImg!=NULL)
                drawGraphic(soundOverImg, 150-soundOverImg->w/2, 230+50);
            else if(soundImg!=NULL)
                drawGraphic(soundImg, 150-soundImg->w/2, 230+50);
                
            /* Music */
            if(musicOnOff)
                drawGraphic(onImg, 80, 230+100);
            else
                drawGraphic(offImg, 80, 230+100);
            if(mouseOverOption==3 && musicOverImg!=NULL)
                drawGraphic(musicOverImg, 150-musicOverImg->w/2, 230+100);
            else if(musicImg!=NULL)
                drawGraphic(musicImg, 150-musicImg->w/2, 230+100);

                
            /* Back to main menu */
            if(mouseOverOption==4 && backOverImg!=NULL)
                drawGraphic(backOverImg, 150-backOverImg->w/2, 230+150);
            else if(backImg!=NULL) 
                drawGraphic(backImg, 150-backImg->w/2, 230+150);
            
            break;
            
        /* Are you sure you wish to reset the high scores? */
        case SURE_DELETE_SCORES:
            /* logo */
            if(logoImg!=NULL)	drawGraphic(logoImg, 150-logoImg->w/2, 50);
            
            /* message */
            if(sureImg!=NULL)	drawGraphic(sureImg, 150-sureImg->w/2, 210);
            
            /* Medal */
            drawMedal();
            
            /* menu items */
            /* yes */
            if(mouseOverOption==3 && yesOverImg!=NULL)
                drawGraphic(yesOverImg, 150-yesOverImg->w/2, 230+100);
            else if(yesImg!=NULL)
                drawGraphic(yesImg, 150-yesImg->w/2, 230+100);
            
            /* cancel */
            if(mouseOverOption==4 && cancelOverImg!=NULL)
                drawGraphic(cancelOverImg, 150-cancelOverImg->w/2, 230+150);
            else if(cancelImg!=NULL) 
                drawGraphic(cancelImg, 150-cancelImg->w/2, 230+150);
            break;
        
        case INSTRUCTIONS:
            drawGraphic(instructions, 0, 0);
            break;
        
        default:
            break;
    }
}


/* Draw medal display for menu - silver star + smaller stars for star rating */
void drawMedal( void )
{
    /* Game complete? */
    if( worlds[numWorlds-1]->highScore >= worlds[numWorlds-1]->completeScore )
    {
        /* Silver star */
        drawGraphic(medalImg, 119, 431);
        
        /* Rating stars */
        drawStars( 470, starMedalLevel );
    }
}

/* Draw star rating for current level in center of screen at a particular height */
void drawStars( int y, int numStars )
{
    int i;
    int startX;
    int gap = 3;
    
    /* Don't bother processing if there isn't at least one star, the image exists, and y isn't offscreen */
    if(numStars>0 && numStars<20 && starImg && y>-starImg->h && y<500)
    {
        startX = 152-(numStars*(starImg->w+gap)-gap)/2;
        
        /* Draw each star */
        for(i=0; i<numStars; i++) drawGraphic(starImg, startX+i*(starImg->w+gap), y);
    }
}

void startGame( void )
{
    int i;
    int dead;
    SDL_Event event;
    int justCompleted;
    int newStarRating;
    
    /* Flush event queue */
    while ( SDL_PollEvent(&event) );
    keyDownLeftArrow=0;
    keyDownRightArrow=0;
    keyDownDownArrow=0;
    keyDownUpArrow=0;
    keyDownEscape=0;
    keyDownActionKey=0;
    
    /*
        STARTING NEW GAME
    */
    /* Reset level/game variable parameters */
    dead = 0;
    levelDistance=0;
    lastFrameTime = SDL_GetTicks();
    worldCompleteTime = 0;
    gamePaused=0;
    nextStarTarget=0;
    starCompleteTime = 0;
    currentPowerup = NO_POWERUP;
    powerupElapsedTime=0;
    powerupStartTime=0;
    powerupPickupY=-1;
    powerupPickup=NO_POWERUP;
    jetpackChannel=-1;
    warned=0;
    
    /* Fill screen with platforms, and make the center one the start platform */
    randomiseAllPlatforms();
    
    /* Put player on start platform */
    resetPlayer();
    
    /* Start music */
    if(musicOnOff && musicLoopChannel==-1)
        musicLoopChannel = Mix_FadeInChannel(-1, gameMusic, -1, 500);
    else if(!musicOnOff && musicLoopChannel!=-1)
    {
        Mix_HaltChannel(musicLoopChannel);
        musicLoopChannel=-1;
    }
    
    
    
    /*
        ENTERING ACTUAL GAME LOOP
    */
    while ( !dead ) {
        
        /* For finding time difference for correct game logic timing */
        framePeriod = (SDL_GetTicks()-lastFrameTime);
        lastFrameTime = SDL_GetTicks();
        
        /* scroll through level based on time */
        /* get faster based on the level distance. At level completion, speed increases to 1.5x the initial speed */
        if(!gamePaused)
        {
            levelDY = (currentWorld->scrollSpeed +	// increment levelDistance by scrollSpeed
                    0.5 * currentWorld->scrollSpeed *	// extra bit to make level go faster and faster
                    ((float)(levelDistance/100)/(float) currentWorld->completeScore) )
                 * framePeriod / 300.0;			// correct timing, whatever the speed of the system
            levelDistance += levelDY;
        }

        
        /* completed world? */
        if(worldCompleteTime==0
            && levelDistance/100 > currentWorld->completeScore
            && currentWorld->highScore < currentWorld->completeScore
            && !gamePaused)
        {
            worldCompleteTime = lastFrameTime;
            playSound(congrats);
        }
        
        /* Draw parallax layers BEHIND player */
        for(i=1; i<=1+slowProcessor*5; i++)
            if(gamePaused<2) drawBackgrounds(0, 0, 5);
        
        /* Draw player */
        if(gamePaused<2) drawPlayer();
        
        /* Update the controls/player position */
        if(!gamePaused) updatePlayer();
        if(playerHealth==0) dead=1;
        
        /* Updates the timer etc on powerups / collision with player */
	if(!gamePaused) updatePowerups();

        /* Draws powerups in level */
	if(gamePaused<2) drawPowerups();

        /* Update platform positions */
        if(!gamePaused) updatePlatforms();
        
        /* Draw platforms */
        if(gamePaused<2) drawPlatforms();
        
        /* Draw ceiling obstruction - spikes etc */
        if(gamePaused<2) drawCeilingObstruction();
        
        /* Draw parallax layers IN FRONT OF player */
        for(i=1; i<=1+slowProcessor*3; i++)
            if(gamePaused<2) drawBackgrounds(0, 6, 7);
            
        /* Draw interface - health bar etc */
        if(gamePaused<2) drawGameInterface();
        
        if(gamePaused<2) SDL_Flip(mainScreen);
        
        /* Increment gamePaused so when paused everything doesn't keep being redrawn - just needs to draw frame once */
        if(gamePaused==1) gamePaused=2;
        
        /* Controls */
        if(getGameControls()) dead=1;
        
    }
    
    /* Stop jetpack looping sound, if it is still looping */
    if(jetpackChannel!=-1)
    {
        Mix_HaltChannel(jetpackChannel);
        jetpackChannel=-1;
    }
    
    /* Stop music */
    if(musicLoopChannel!=-1)
    {
        Mix_HaltChannel(musicLoopChannel);
        musicLoopChannel=-1;
    }
    
    /* Death sound */
    playSound(death);
    
    /* make sure game isn't paused for this period */
    gamePaused=0;
    
    /* Continue to scroll for a little bit after the player dies */
    diedTime = SDL_GetTicks();
    playerHealth=0;
    while(SDL_GetTicks()-diedTime < 2500)
    {
        framePeriod = (SDL_GetTicks()-lastFrameTime);
        lastFrameTime = SDL_GetTicks();
        levelDistance += (1.0-(float)(SDL_GetTicks()-diedTime)/5000.0) * currentWorld->scrollSpeed * framePeriod / 300.0; // slow down slightly
        drawBackgrounds(0, 0, 5);
        updateDeadPlayer();
        drawPlayer();
        updatePlatforms();
        drawPlatforms();
        drawCeilingObstruction();
        drawBackgrounds(0, 6, 7);
        drawGameInterface();
        SDL_Flip(mainScreen);
    }
    
    /* Make sure all sounds have stopped, in particular the looping ambient sound */
    Mix_HaltChannel(-1);	// -1 means halt all channels, including ambient sound loop
    ambientSndChannel=-1;
    
    /* Record high score */
    justCompleted=0;
    if( levelDistance/100 > currentWorld->highScore && levelDistance/100 >= currentWorld->completeScore )
    {
        if(currentWorld->highScore < currentWorld->completeScore) justCompleted=1;
        currentWorld->highScore = (int)levelDistance/100;
        savePrefsScores();
    }
    
    /* If just completed all levels, or completed all levels on a certain star rating,
        show a congratulations screen */
    /* Game completed */
    if(currentWorld->order >= numWorlds-1 && justCompleted )
    {
        playSound(congratsEnding);
        showGraphic(gameCompleteScreen, -1, 0);
    }
            
    /* New Star Rating */
    newStarRating=5;
    for(i=0; i<numWorlds; i++)
    {
        calculateWorldStars(i);
        if(worlds[i]->numStars<newStarRating) newStarRating=worlds[i]->numStars;
    }
    
    if(newStarRating>starMedalLevel)
    {
        starMedalLevel=newStarRating;
        
        playSound(getStarRating);
        switch(newStarRating)
        {
            case 1:
                showGraphic(starRating1Screen, -1, 0);
                break;
            case 2:
                showGraphic(starRating2Screen, -1, 0);
                break;
            case 3:
                showGraphic(starRating3Screen, -1, 0);
                break;
            case 4:
                showGraphic(starRating4Screen, -1, 0);
                break;
            case 5:
                showGraphic(starRating5Screen, -1, 0);
                break;
            default: break;
        }
    }
}



/* Each of these 2 functions is used when clicking and with keys */
void selectNextWorld(void)
{
    currentWorld = worlds[currentWorld->order+1];
    levelDistance=0;
    playSound(interface);
    
    /* Stop ambient loop so that a new one is played */
    if(ambientSndChannel!=-1)
    {
        Mix_HaltChannel(ambientSndChannel);
        ambientSndChannel=-1;
    }
}

void selectPrevWorld(void)
{
    currentWorld = worlds[currentWorld->order-1];
    levelDistance=0;
    playSound(interface);
    
    /* Stop ambient loop so that a new one is played */
    if(ambientSndChannel!=-1)
    {
        Mix_HaltChannel(ambientSndChannel);
        ambientSndChannel=-1;
    }
}

/* Code which is used several times in the function beneath */
int selectOption()
{
    int returnAction=0;
    int i=0;
    
    /* Choose what to do with the selected option */
    switch(currentMenu)
    {
        /* MAIN MENU */
        case MAIN:
            switch(mouseOverOption)
            {
                /* New Game - go to world select screen */
                case 1:
                    changeMenu(WORLD_CHOICE);
                    break;
                /* Options - go to options menu */
                case 2:
                    changeMenu(OPTIONS);
                    break;
                /* Instructions - go to instructions screen */
                case 3:
                    //changeMenu(INSTRUCTIONS);
                        playSound(interface);
                    showGraphic(instructions,-1,0);
                        playSound(interface);
                    showGraphic(instructions2,-1,0);
                        playSound(interface);
                    break;
                /* Quit - quit the game */
                case 4:
                    playSound(interface);
                    returnAction = 1;
                    break;
                default: break;
            }
            break;
            
        /* WORLD SELECTION SCREEN */
        case WORLD_CHOICE:
            
            /* Prev */
            if( mouseInRect(15, 245, 23, 96) && currentWorld->order > 0 )
            {
                selectPrevWorld();
            }
                
            /* Next */
            else if( mouseInRect(262, 256, 23, 96) && currentWorld->order < numWorlds-1 )
            {
                selectNextWorld();
            }
            /* Back to previous menu */
            else if( mouseInRect(46, 476, 203, 18) )
                changeMenu( MAIN );
            
            /* Anywhere else - start new game */
            else if(!worldLocked())
            {
                returnAction = 2;
                playSound(interface);
            }
            
            break;

        case OPTIONS:
            switch(mouseOverOption)
            {
                /* Reset high scores */
                case 1:
                    changeMenu(SURE_DELETE_SCORES);
                    break;
                
                /* Toggle sound */
                case 2:
                    if(soundOnOff) soundOnOff=0; else soundOnOff=1;
                    playSound(interface);
                    savePrefsScores();
                    break;
                
                case 3:
                    playSound(interface);
                    if(musicOnOff) musicOnOff=0; else musicOnOff=1;
                    savePrefsScores();
                    break;
                    
                /* Go back to main menu */
                case 4:
                    changeMenu(MAIN);
                    break;
                default: break;
            }
            break;
            
        case SURE_DELETE_SCORES:
            switch(mouseOverOption)
            {
                /* Yes */
                /* Reset high scores */
                case 3:
                    /* Set each score and star rating to 0 */
                    for(i=0;i<numWorlds;i++)
                    {
                        worlds[i]->highScore=0;
                        worlds[i]->numStars=0;
                    }
                    
                    /* Set medal score */
                    starMedalLevel=0;
                    
                    /* Save */
                    savePrefsScores();
                    
                    /* Back to options menu */
                    changeMenu(OPTIONS);
                    break;
                 
                /* Cancel */
                /* Go back to options */
                case 4:
                    changeMenu(OPTIONS);
                    break;
                default: break;
            }
            break;
            
        case INSTRUCTIONS:
            changeMenu(MAIN);
            break;
                
        default: break;
    }
    
    return returnAction;
}

/* Function affects global keyDown as well as mouseDown variables */
int getMenuControls( void )
{
    int returnAction=0;
    SDL_Event event;
    
    while ( SDL_PollEvent(&event) ) {
        switch (event.type) {
            case SDL_MOUSEMOTION:
                mouseOverOption = (int)((float)(event.motion.y - 230) / 50.0)+1;
                if(mouseOverOption<1) mouseOverOption=-1;
                mouseX = event.motion.x;
                mouseY = event.motion.y;
                break;
            case SDL_MOUSEBUTTONDOWN:
                returnAction = selectOption();
                break;
                
            case SDL_KEYDOWN:
                switch(event.key.keysym.sym)
                {
                    /* Menu controls */
                    case SDLK_LEFT:
                        if( currentWorld->order > 0 )
                        {
                            selectPrevWorld();
                        }
                        break;
                    case SDLK_RIGHT:
                        if( currentWorld->order < numWorlds-1 )
                        {
                            selectNextWorld();

                        }
                        break;
                    case SDLK_DOWN:	mouseOverOption++;	break;
                    case SDLK_UP:	mouseOverOption--;	break;
                    case SDLK_q:
                    case SDLK_ESCAPE:
                        switch(currentMenu)
                        {
                            /* Quit if we're in the main menu */
                            case MAIN:
                                returnAction=1;
                                break;
                            /* Go back to options if in the "are you sure" dialog */
                            case SURE_DELETE_SCORES:
                                changeMenu(OPTIONS);
                                break;
                            /* ... otherwise go down a level in the menu system */
                            default:
                                changeMenu(MAIN);
                                break;
                        }
                        break;
                    /* Music toggle */
                    case SDLK_m:
                        if(musicOnOff) musicOnOff=0; else musicOnOff=1;
                        break;
                    /* Screenshot */
                    case SDLK_F1:	SDL_SaveBMP(mainScreen, "screenshot.bmp");	break;
                    /* Slow game down for testing */
                    case SDLK_F2:
                        if(slowProcessor==0) slowProcessor=1; else slowProcessor=0;
                        break;
                    /* Select option (mouse click or return key in menu) */
                    case SDLK_RETURN:
                        returnAction = selectOption();
                        break;
                    default: break;
                }
                break;
            case SDL_KEYUP:
                switch(event.key.keysym.sym)
                {
                    case SDLK_LEFT:	keyDownLeftArrow=0;	break;
                    case SDLK_RIGHT:	keyDownRightArrow=0;	break;
                    case SDLK_DOWN:	keyDownDownArrow=0;	break;
                    case SDLK_UP:	keyDownUpArrow=0;	break;
                    case SDLK_ESCAPE:	keyDownEscape=0;	break;
                    default: break;
                }
                break;
            case SDL_QUIT:
                quitApp=1;
                break;
            default:
                break;
        }
    }
    
    /* Keeping mouseOverOption within bounds */
    if(mouseOverOption<1) mouseOverOption = 1;
    /* Terminating at the bottom - different menus have different */
    switch(currentMenu)
    {
        case MAIN:
            if(mouseOverOption>4) mouseOverOption=4;
            break;
        case OPTIONS:
            if(mouseOverOption>4) mouseOverOption=4;
            break;
        case SURE_DELETE_SCORES:
            if(mouseOverOption<3) mouseOverOption=3;
            if(mouseOverOption>4) mouseOverOption=4;
            break;
        default:
            if(mouseOverOption>1) mouseOverOption=1;
            break;
    }
    
    return returnAction;
}

/* Function affects global keyDown variables */
/* Also returns 1 if instructions have been received to pause the game */
int getGameControls( void )
{
    int quit=0;
    SDL_Event event;
    
    /* Parse through all events */
    while ( SDL_PollEvent(&event) ) {
        switch (event.type) {
            case SDL_KEYDOWN:
                switch(event.key.keysym.sym)
                {
                    /* Start moving */
                    case SDLK_a:
                    case SDLK_KP4:
                    case SDLK_COMMA:
                    case SDLK_LEFT:	keyDownLeftArrow=1;	break;
                    case SDLK_d:
                    case SDLK_KP6:
                    case SDLK_PERIOD:
                    case SDLK_RIGHT:	keyDownRightArrow=1;	break;
                    case SDLK_s:
                    case SDLK_KP5:
                    case SDLK_KP2:
                    case SDLK_DOWN:	keyDownDownArrow=1;	break;
                    /* Action key for powerups */
                    case SDLK_w:
                    case SDLK_UP:
                    case SDLK_KP8:
                    case SDLK_LCTRL:
                    case SDLK_RCTRL:
                    case SDLK_SPACE:
                    case SDLK_LALT:
                    case SDLK_RALT:
                    case SDLK_KP0:	keyDownActionKey=1;	break;
                    /* Screenshot */
                    case SDLK_F1:	SDL_SaveBMP(mainScreen, "../../../screenshot.bmp");	break;
                    /* Slow game down for testing */
                    case SDLK_F2:
                        if(slowProcessor==0) slowProcessor=1; else slowProcessor=0;
                        break;
                    /* Quit */
                    case SDLK_q:
                        quit=1;
                        gamePaused=0;
                        break;
                    /* Pause */
                    case SDLK_ESCAPE:
                        if(gamePaused)
                        {
                            gamePaused=0;
                            if(currentPowerup!=NO_POWERUP) powerupStartTime=SDL_GetTicks();
                            
                            /* Start ambient sound loop again */
                            if(soundOnOff)
                                    ambientSndChannel = Mix_PlayChannel(-1, currentWorld->ambientSnd, -1);
                                    
                            /* Start music */
                            if(musicOnOff && musicLoopChannel==-1)
                            {
                                musicLoopChannel = Mix_PlayChannel(-1, gameMusic, -1);
                            }
                                    
                        }
                        else
                        {
                            gamePaused=1;
                            if(currentPowerup!=NO_POWERUP) powerupElapsedTime += SDL_GetTicks()-powerupStartTime;
                            /* Stop jetpack loop */
                            if(jetpackChannel>-1)
                            {
                                Mix_HaltChannel(jetpackChannel);
                                jetpackChannel=-1;
                            }
                            
                            /* Stop ambient sound loop */
                            if(ambientSndChannel!=-1)
                            {
                                Mix_HaltChannel(ambientSndChannel);
                                ambientSndChannel=-1;
                            }
                            
                            /* Stop music */
                            if(musicLoopChannel!=-1)
                            {
                                Mix_HaltChannel(musicLoopChannel);
                                musicLoopChannel=-1;
                            }

                        }
                        
                        break;
                    default: break;
                }
                break;
            case SDL_KEYUP:
                switch(event.key.keysym.sym)
                {
                    /* Stop moving */
                    case SDLK_a:
                    case SDLK_KP4:
                    case SDLK_COMMA:
                    case SDLK_LEFT:	keyDownLeftArrow=0;	break;
                    case SDLK_d:
                    case SDLK_KP6:
                    case SDLK_PERIOD:
                    case SDLK_RIGHT:	keyDownRightArrow=0;	break;
                    case SDLK_s:
                    case SDLK_KP5:
                    case SDLK_KP2:
                    case SDLK_DOWN:	keyDownDownArrow=0;	break;
                    /* Action key for powerups */
                    case SDLK_w:
                    case SDLK_UP:
                    case SDLK_KP8:
                    case SDLK_LCTRL:
                    case SDLK_RCTRL:
                    case SDLK_SPACE:
                    case SDLK_LALT:
                    case SDLK_RALT:
                    case SDLK_KP0:	keyDownActionKey=0;	break;
                    default: break;
                }
                break;
            case SDL_QUIT:
                quitApp = 1;
                break;
            default:
                break;
        }
    }
    return quit+quitApp;
}

/* Checks to see if currentWorld is locked, for world selection screen */
int worldLocked( void )
{
    /* If first world, then unlocked */
    if( currentWorld->order<1 ) return 0;
    else
    {
        /* Check high score of level below */
        if( worlds[currentWorld->order-1]->highScore >= worlds[currentWorld->order-1]->completeScore )
            return 0;
        else
            return 1;
    }
}


/* Positions player in the center of the start platform */
void resetPlayer( void )
{
    playerX = platforms[START_PLATFORM].x;
    playerY = platforms[START_PLATFORM].y;
    playerDY = 0;
    playerOn = 1;
    playerAbove = START_PLATFORM;
    playerAnimation = &standAnim;
    playerAnimFrame = randomInt(7);
    playerHealth=10;
}

/* Player just falls off the bottom of the screen */
void updateDeadPlayer( void )
{
    /* falling physics */
    playerDY += currentWorld->gravity*framePeriod/12.0;
    playerY += playerDY*framePeriod/12.0;
    
    /* animation */
    playerAnimation = &fallAnim;
}



/* Platform numbers loop round - useful function makes sure they are within bounds */
int wrapPlatformNum(int toWrap)
{
    while(toWrap<0) toWrap+=9;
    return toWrap%9;
}
    
    
/* Updates positioning/physics of player */
void updatePlayer( void )
{
    int playerHalfWidth;	// Used for calculating collisions with level boundaries and platforms
    float playerPrevY=0;		// Used for collisions with platforms
    float playerSpeedMultiplier=1;	// For tweaking gameplay
    animation* prevAnim;
    int thisAbove=0;	// for collision detection - running through the platforms the player has passed
    int i=0;
    int collided=0;
    int limit=0;
    
    /* Calculate half the player's width for collision calculations later on */
    //if(playerAnimation!=NULL) playerHalfWidth=playerAnimation->w/2; else playerHalfWidth=20;
    playerHalfWidth=13;
    
    /* Moving left and right */
    if(keyDownLeftArrow) playerX -= (playerSpeedMultiplier*framePeriod/8.0);
    if(keyDownRightArrow) playerX += (playerSpeedMultiplier*framePeriod/8.0);
    
    /* X out of bounds? */
    if(playerX < playerHalfWidth) playerX=playerHalfWidth;
    if(playerX > mainScreen->w-playerHalfWidth) playerX = mainScreen->w-playerHalfWidth;
    
    /* Colliding with roof */
    if(playerY-levelDistance < 0)
    {
        /* On platform - knock off */
        if(playerOn)
        {
            playerOn=0;
            
            /* Player must now be above the platform below this one */
            playerAbove = wrapPlatformNum(playerAbove+1);
        }
        
        /* Subtract health for hitting ceiling */
        if(currentPowerup==SHIELD) playerHealth-=2; else playerHealth-=5;
        if(playerHealth<0) playerHealth=0;
        
        /* Play ouch sound */
        playSoundSet(&ouch);
            
        playerDY = levelDY+1;	// move down, away from roof
        playerY = levelDistance+5;
    }
    
    /*
        Hanging from platform
    */
    if(playerOn)	// if player is on a platform
    {
    /* Update falling off the edge of a platform, conveyors, bouncing block */
        /* If player has landed on a ledge, the parachute must have closed */
        /* Similarly for the jetpack */
        /* Used for sound */
        parachuteOpen=0;
        if(jetpackChannel>-1)
        {
            Mix_HaltChannel(jetpackChannel);
            jetpackChannel=-1;
        }
        
        switch(platforms[playerAbove].type->typeCode)
        {
            int delayTime;
            /* Conveyors */
            case CONVEYORLEFT:
                playerX -= (playerSpeedMultiplier*(float)framePeriod/16.0);
                break;
            case CONVEYORRIGHT:
                playerX += (playerSpeedMultiplier*(float)framePeriod/16.0);
                break;
            
            /* Bouncing block */
            case BOUNCE:
                playerX += ( platforms[playerAbove].active*framePeriod/20.0 );
                break;
                
            /* Corrosion */
            case CORRODEFAST:
            case CORRODESLOW:
                if(platforms[playerAbove].type->typeCode==CORRODEFAST) delayTime=150; else delayTime=300;
                if(currentPowerup==SHIELD) delayTime*=2;
                
                if(lastFrameTime-lastCorrodeTime > delayTime)
                {
                    playerHealth-=1;
                    playSoundSet(&ouchSmall);
                    lastCorrodeTime=lastFrameTime;
                }
                
                break;
                
                
            default: break;
        }
        if(playerHealth<0) playerHealth=0;
        
        /* Jump powerup */
        if(currentPowerup==JUMP && keyDownActionKey==1)
        {
            playerDY = -7;
            playerY = platforms[playerAbove].y;
            platforms[playerAbove].active = 1;
            playerOn=0;
        }
                
        /* X position is outside limits of platform we are meant to be standing on? */
        /* OR player is pressing down key */
        /* OR platform drops player (is correct type for behaviour) */
        if(playerX < platforms[playerAbove].x-(platforms[playerAbove].type->width/2)-playerHalfWidth
        || playerX > platforms[playerAbove].x+(platforms[playerAbove].type->width/2)+playerHalfWidth
        || keyDownDownArrow
        || platforms[playerAbove].type->typeCode==REVOLVE
        || platforms[playerAbove].type->typeCode==DESTROY
        || platforms[playerAbove].type->typeCode==FALLAWAY )
        {
            playerOn=0;
            
            /* Now player has landed and started dropping, start animation */
            if(	   platforms[playerAbove].type->typeCode==REVOLVE
                || platforms[playerAbove].type->typeCode==DESTROY
                || platforms[playerAbove].type->typeCode==FALLAWAY )
            {
                platforms[playerAbove].active=1;
                /* drop player slightly more gently */
                playerDY=-1;
                playSoundSet(&ah);
            }
                
            /* Player must now be above the platform below this one */
            playerAbove = wrapPlatformNum(playerAbove+1);
        }
        
        /* Animations */
        if(keyDownLeftArrow) playerAnimation = &walkLeftAnim;
        else if(keyDownRightArrow) playerAnimation = &walkRightAnim;
        else if(playerAnimation!=&standAnim)
        {
            playerAnimation = &standAnim;
            playerAnimFrame = randomInt(7);
        }
    }
    
    /*
        Already falling
    */
    else
    {
        
        /* Find out where the player starts at (for debugging) */
        playerPrevY = playerY;
        
        /* falling physics */
        playerDY += currentWorld->gravity*framePeriod/20.0;
        
        /* jetpack */
        if(currentPowerup==JETPACK && keyDownActionKey==1)
        {
            playerDY -= currentWorld->gravity*1.3*framePeriod/20.0;
            
            /* jetpack's looping sound */
            if(jetpackChannel==-1 && soundOnOff)
                jetpackChannel = Mix_PlayChannel(-1, jetpack, -1);
        } else if(((currentPowerup==JETPACK && keyDownActionKey!=1) || currentPowerup!=JETPACK) && jetpackChannel>-1)
        {
            Mix_HaltChannel(jetpackChannel);
            jetpackChannel=-1;
        }
        
        /* terminal velocity */
        if(playerDY > 15) playerDY = 15;
        if(playerDY < -5) playerDY = -5;
        if(currentPowerup==PARACHUTE
            && keyDownActionKey==1)
        {
                if(playerDY > currentWorld->gravity*10)
                    playerDY = currentWorld->gravity*3;
                
                /* sound */
                if(!parachuteOpen)
                    playSound(parachuteSnd);
                parachuteOpen=1;
        } else parachuteOpen=0;
        
        // distance to travel this frame = distance to travel last frame + some increase
        playerY += playerDY*framePeriod/20.0;
        
        
        /* Collision with platform below */
        if(playerY > platforms[playerAbove].y)
        {
            
            /* Make sure it doesn't loop round to top */
            /* just went past bottom platform, so no collision detection needed
               except that one platform */
            if(platforms[playerAbove].isBottom)
                limit=0;
            /* Otherwise we could be passing more than one platform */
            else
            {
                limit=(int)floor((playerY-platforms[playerAbove].y)/60.0);
                while(platforms[wrapPlatformNum(playerAbove+limit+1)].y < platforms[playerAbove].y)
                {
                    limit--;
                    if(limit==0) break;
                }
            }
            
            /* Player has passed more than 1 platform in a single frame (game stuttered) */
            for(i=0;i<=limit;i++)
            {
                thisAbove=wrapPlatformNum(playerAbove+i);
                
                /* Collided? */
                if(playerX>platforms[thisAbove].x-(platforms[thisAbove].type->width/2)-playerHalfWidth
                    && playerX<platforms[thisAbove].x+(platforms[thisAbove].type->width/2)+playerHalfWidth
                    && playerPrevY < platforms[thisAbove].y)
                {
                    /* Damaging platforms */
                    switch(platforms[thisAbove].type->typeCode)
                    {
                        case CORRODEFAST:
                            if(currentPowerup!=SHIELD) playerHealth-=1;
                            playSoundSet(&ouch);
                        case CORRODESLOW:
                            lastCorrodeTime=lastFrameTime;
                            break;
                        
                        case DAMAGE:
                            if(currentPowerup==SHIELD) playerHealth-=1; else playerHealth-=4;
                            playSoundSet(&ouch);
                            break;
                        default: break;
                    }
                    if(playerHealth<0) playerHealth=0;
                    
                    /* Otherwise add on one point of health if necessary */
                    if(playerHealth<10 && !keyDownDownArrow && platforms[thisAbove].type->typeCode < DAMAGE) playerHealth++;
                    
                    /* Bounces back upwards if spring or is jumping using jump powerup */
                    if(platforms[thisAbove].type->typeCode==SPRING)
                    {
                        /* Drop past it if holding down arrow */
                        if(!keyDownDownArrow)
                        {
                            playerDY = -7;
                            playerY = platforms[thisAbove].y;
                            platforms[thisAbove].active = 1;
                            playSoundSet(&boing);
                            collided = 1;
                        }
                        playerOn=0;	// bouncing in air
                    }
                    /* Otherwise reduce speed to zero */
                    else
                    {
                        playerDY = 0;
                        playerOn = 1;
                        playerY = platforms[thisAbove].y;
                        if(platforms[thisAbove].type->snd==NULL)
                            playSoundSet(&land);
                        else
                            playSound(platforms[thisAbove].type->snd);
                        collided = 1;
                    } // if platform is a spring
                    
                    
                    /* player has hit a platform - for loop broken */
                    break;
                } // if within x bounds of platform
            } // for each platform player has passed in this frame
            
            /* Above the newly collided platform, or the one below if not collided */
            if(collided)
                playerAbove = thisAbove;
            else if(platforms[wrapPlatformNum(thisAbove+1)].y > platforms[thisAbove].y)
            {
                playerAbove = wrapPlatformNum(thisAbove+1);
            }
            
            if(playerY < playerPrevY)
                fprintf(stderr, "above!! ");
            
        } // if player has passed platform playerAbove
        
        /* Animations */
        prevAnim = playerAnimation;
        if(currentPowerup==PARACHUTE && keyDownActionKey==1) playerAnimation = &standAnim;
        else if(currentPowerup==JETPACK && keyDownActionKey==1) playerAnimation = &monkeyJetAnim;
        else if(keyDownLeftArrow) playerAnimation = &fallLeftAnim;
        else if(keyDownRightArrow) playerAnimation = &fallRightAnim;
        else playerAnimation = &fallAnim;
        if(prevAnim != playerAnimation) playerAnimFrame=0;
    }
    
    /* Fallen off bottom */
    if(playerY-levelDistance > mainScreen->h)
        playerHealth=0;
}


/* Updates the timer etc on powerups / collision with player */
void updatePowerups( void )
{
    int timeLimit=0;
    
    /* Pickups */
    if(powerupPickup != NO_POWERUP)
    {
        /* Player collision - simple radius check */
        if(sqrt(pow(150-playerX,2) + pow(powerupPickupY-playerY,2)) < 50)
        {
            /* Player now holds it */
            currentPowerup = powerupPickup;
            prevPickupY=powerupPickupY;
            powerupElapsedTime=0;
            powerupStartTime=SDL_GetTicks();
            warned=0;
            
            /* Play collect sound */
            switch(currentPowerup)
            {
                case HEAL:
                    playSound(powerupHealthSnd);
                    break;
                case PARACHUTE:
                    playSound(powerupParachuteSnd);
                    break;
                case JETPACK:
                    playSound(powerupJetpackSnd);
                    break;
                case JUMP:
                    playSound(powerupBeanSnd);
                    break;
                default: break;
            }
            
            /* Remove from world */
            powerupPickup = NO_POWERUP;
        }
        
        /* Powerup disappears offscreen */
        if(powerupPickupY<levelDistance-60) powerupPickup = NO_POWERUP;
    }
    
    /* Player's current powerup */
    if(currentPowerup != NO_POWERUP)
    {
        switch(currentPowerup)
        {
            /* Bring player back to full health when he lands on the heart */
            case HEAL:
                timeLimit = 1000; // invincible for 1 sec
                
                /* Renew health */
                if(playerHealth > 0 && playerHealth < 10)
                    playerHealth=10;
                
                break;
            
            /* Terminal velocity is reduced when action key is pressed */
            case PARACHUTE:
                timeLimit = 20000; // 20 sec limit 
                break;  
            /* Boost is given when action button is pressed */
            case JETPACK:
                timeLimit = 6000; // 5 sec limit
                break;
            /* Monkey can jump as if on a spring platform when action key is pressed */
            case JUMP:
                timeLimit = 20000; // 20 second limit
                break;  
            /* Shield - reduces damage off damaging platforms/ceiling spikes */
            case SHIELD:
                timeLimit = 15000; // 15 sec limit
                break;
            default: break;
        }
        
        /* timer NEARLY up? */
        if(powerupElapsedTime+SDL_GetTicks()-powerupStartTime > timeLimit-1500 && !warned && currentPowerup!=HEAL)
        {
            playSound(timeUp);
            warned=1;
        }
        
        /* timer up? */
        if(powerupElapsedTime+SDL_GetTicks()-powerupStartTime > timeLimit)
        {
            currentPowerup = NO_POWERUP;
            warned=0;
        }
                            
    } // player has a powerup?
                    
}

/* Draws powerups in level */
void drawPowerups( void )
{
    int frameTime = 60;
    animation* anim=NULL;
    int animWidth =0;
    
    /* If there is a pickup on screen */
    if(powerupPickup != NO_POWERUP)
    {
        /* Depends on type of powerup */
        switch(powerupPickup)
        {
            case HEAL:
                anim = &healAnim;
                animWidth = healAnim.w;
                frameTime = 100;
                break;
            
            case PARACHUTE:
                anim = &paraBagAnim;
                animWidth = paraBagAnim.w;
                break;
                
            case JETPACK:
                anim = &jetpackAnim;
                animWidth = jetpackAnim.w;
                break;
            
            case JUMP:
                anim = &jumpAnim;
                animWidth = jumpAnim.w;
                break;
                
            case SHIELD:
                anim = &shieldAnim;
                animWidth = shieldAnim.w;
                break;
                
            default:
                break;
        }
        
        /* Update animation */
        if(SDL_GetTicks()-powerupLastFrameTime > frameTime && !gamePaused)
        {
            powerupAnimFrame++;
            if(powerupAnimFrame>anim->numFrames-1) powerupAnimFrame=0;
            powerupLastFrameTime=SDL_GetTicks();
        }
        
        /* Draw */
        drawAnimation(anim, (int)(150-animWidth/2), (int)(powerupPickupY-levelDistance), (int)powerupAnimFrame);
    }
}


/* Updates the positions of the platforms, and repositions and changes them when they go offscreen */
/* A platform which goes off the top of the screen wraps back to the bottom and mutates */
void updatePlatforms( void )
{
    int i;
    int powerupIndex;
    
    /* Update each platform in the array */
     for(i=0;i<9;i++)
     {
        
        /* Fall away using active variable as a primate y */
        if(platforms[i].type->typeCode==FALLAWAY && platforms[i].active>0 && platforms[i].active<1100)
            platforms[i].active += 5+platforms[i].active/5;
        
        /* X Bouncing block */
        if(platforms[i].type->typeCode==BOUNCE)
        {
            /* make sure it is going somewhere */
            if(platforms[i].active==0)
            {
                if(platforms[i].x>150)
                    platforms[i].active=-1;
                else
                    platforms[i].active=1;
            }
            
            /* increment position */
            platforms[i].x += (int)( platforms[i].active*framePeriod/20.0 );
            
            /* bounce off edge of screen */
            if(platforms[i].x>300)
            {
               platforms[i].x=299;
               platforms[i].active=-1;
            }
            if(platforms[i].x<0)
            {
               platforms[i].x=1;
               platforms[i].active=1;
            }
        }
        
        /* Offscreen? */
        if(platforms[i].y < (levelDistance-20))
        {
            int prevPlat;
            
            prevPlat=i-1;
            if(prevPlat<0) prevPlat+=9;
            
            /* Move to bottom change to a new random type/position */
            platforms[i].y += 9*60;
            randomisePlatform(i);
            platforms[i].isBottom = 1;
            platforms[prevPlat].isBottom=0;
            
            /* POWERUP CREATION */
            if(powerupPickup==NO_POWERUP)
            {
                for(powerupIndex=0; powerupIndex<8; powerupIndex++)
                {
                    /* Powerup at array index? */
                    if(currentWorld->powerups[powerupIndex]!=NO_POWERUP)
                    {
                        /* Random whether powerup appears or not */
                        if(randomTrueFalse(currentWorld->powerupFrequencies[powerupIndex]))
                        {
                            /* Place a powerup */
                            powerupPickupY = platforms[i].y + 15;
                            powerupPickup = currentWorld->powerups[powerupIndex];
                        }
                    }
                    else break;
                } // for each world powerup type
            } // if there isn't already a powerup pickup
        } // platform off top of screen?
    } // for each platform
}

/* Use SDL_Mixer to play a sound, playing just once on a free channel */
void playSound(sound* sound)
{
    if(soundOnOff)
    {
        /*	play sample on first free unreserved channel
            play it exactly once through */
        if(Mix_PlayChannel(-1, sound, 0)==-1)
            fprintf(stderr, "Mix_PlayChannel: %s\n",Mix_GetError());
    }
}

/* Play a random sound from a sound set */
void playSoundSet(soundSet* set)
{
    if(soundOnOff)
        playSound(set->soundArray[randomInt(set->size)]);
}

/* Draws a particular SDL_Surface graphic at a position on the screen */
void drawGraphic(SDL_Surface* graphic, int x, int y)
{
    SDL_Rect blitSrc;
    SDL_Rect blitDest;
    
    /* Draw if the graphic exists */
    if(graphic!=NULL)
    {
        /* Set up regions to blit */
        blitSrc.x=0;
        blitSrc.y=0;
        blitSrc.w=graphic->w;
        blitSrc.h=graphic->h;
        blitDest.x=x;
        blitDest.y=y;	// draw from bottom left corner
        blitDest.w=0;	//unused
        blitDest.h=0;	//unused
        
        /* Blit from graphic's surface onto screen surface */
        SDL_BlitSurface(graphic, &blitSrc, mainScreen, &blitDest);
        
    }// else fprintf(stderr, "Error: Graphic missing\n");

}

/* Draws an animation frame at a position on the screen */
void drawAnimation(animation* anim, int x, int y, int frame)
{
    SDL_Rect blitSrc;
    SDL_Rect blitDest;
    
    /* Draw if the graphic exists */
    if(anim && anim->frames)
    {
        /* Set up regions to blit */
        blitSrc.x=frame*anim->w;
        blitSrc.y=0;
        blitSrc.w=anim->w;
        blitSrc.h=anim->h;
        blitDest.x=x;
        blitDest.y=y;	// draw from bottom left corner
        blitDest.w=0;	//unused
        blitDest.h=0;	//unused
        
        /* Blit from graphic's surface onto screen surface */
        SDL_BlitSurface(anim->frames, &blitSrc, mainScreen, &blitDest);
        
    }// else fprintf(stderr, "Error: Graphic missing\n");

}


/* Draw the player in his current position with the correct animation */
void drawPlayer(void)
{
    int frameTime;
    
    if(playerAnimation!=NULL)
    {
        /* Hard coded (shut up I know it is messy) frame times */
        if(playerAnimation==&fallRightAnim || playerAnimation==&fallLeftAnim )
            frameTime=80;
        else if(playerAnimation==&fallAnim)
            frameTime=80;
        else if(playerAnimation==&standAnim)
            frameTime=120;
        else
            frameTime=100;
        
        /* Update animation */
        if(SDL_GetTicks()-playerLastFrameTime > frameTime && !gamePaused)
        {
            playerAnimFrame++;
            if(playerAnimFrame>playerAnimation->numFrames-1)
            {
                /* play to end frame then stop */
                if(playerAnimation==&fallRightAnim || playerAnimation==&fallLeftAnim )
                    playerAnimFrame=playerAnimation->numFrames-1;
                /* Loop round to start */
                else playerAnimFrame=0;
            }
            playerLastFrameTime=SDL_GetTicks();
        }
        
        /* Draw it */
        drawAnimation(playerAnimation, (int)playerX-(playerAnimation->w/2), (int)(playerY-levelDistance), (int)playerAnimFrame);
        
        /* Parachute? */
        if(currentPowerup==PARACHUTE && keyDownActionKey==1 && playerOn==0 && parachuteImg)
            drawGraphic(parachuteImg, (int)playerX-(parachuteImg->w/2), (int)(playerY-levelDistance-parachuteImg->h));
    }
}

/* Draw all the platforms onto the screen */
void drawPlatforms(void)
{
    int fallAwayY=0;
    int i=0;
    int update=0;
    
    /* Draw each one from the platforms[9] array, if it exists */
    for(i=0;i<9;i++)
    {
        /* platforms which are falling down the screen get drawn in a weird location */
        if( platforms[i].type->typeCode==FALLAWAY )
            fallAwayY=platforms[i].active/2;
        else fallAwayY=0;
        
        if(platforms[i].type)
        {
            /* Some only animate once (when stepped on) */
            if(platforms[i].active==0
                &&(    platforms[i].type->typeCode==SPRING
                    || platforms[i].type->typeCode==FALLAWAY
                    || platforms[i].type->typeCode==REVOLVE
                    || platforms[i].type->typeCode==DESTROY) )
                update=0;
            else 
                update=1;
    
            /* Update animation */
            if(SDL_GetTicks()-platforms[i].lastFrameTime > platforms[i].type->framePeriod && update && !gamePaused)
            {
                platforms[i].frame++;
                /* leaving last frame */
                if(platforms[i].frame > platforms[i].type->anim.numFrames-1)
                {
                    switch(platforms[i].type->typeCode)
                    {
                        /* Revert to last frame and do not deactivate */
                        case DESTROY:
                        case FALLAWAY:
                            platforms[i].frame--;
                            break;
                        /* Normal - finish animation, go back to start, deactivate if necessary */
                        default:
                            platforms[i].frame=0;
                            if(platforms[i].type->typeCode!=BOUNCE) platforms[i].active=0;
                            break;
                    }
                }
                platforms[i].lastFrameTime = SDL_GetTicks();
            }
            
            /* Draw animation, correct frame */
            drawAnimation(&platforms[i].type->anim,
                        (int)(platforms[i].x - (platforms[i].type->anim.w)/2),
                        (int)(platforms[i].y - levelDistance - platforms[i].type->yOffset + fallAwayY),
                        platforms[i].frame);
        }
    }
}

/* Draw the parallax layered backgrounds */
void drawBackgrounds(int cropped, int startLayer, int endLayer)
{
    int offset;	// Where to draw the graphic background on the screen (y)
    int layer;	// Iterate over each layer
    SDL_Rect clipRect;	// for world selection drawing
    
    /* If on the world selection screen, only draw into this box for speed */
    if(cropped)
    {
        clipRect.x = 10;
        clipRect.y = 100;
        clipRect.w = 280;
        clipRect.h = 370;
        
        SDL_SetClipRect(mainScreen, &clipRect);
    }
    
    for(layer=startLayer;layer<=endLayer;layer++)
    {
        
        if(currentWorld->backgrounds[layer] != NULL)
        {
            /* Different layers move at different speeds */
            switch (currentWorld->scrollSpeeds[layer])
            {
                /* No movement at all, unless background is greater than 500, in which case it scrolls all the while the level isn't complete */
                case 0:
                    if(currentWorld->backgrounds[layer]->h <=500) offset=0;
                    else if(levelDistance/100 < currentWorld->completeScore) 
                                // total pixels available for background movement  *        current distance / level completion distance
                        offset = - (int) ((currentWorld->backgrounds[layer]->h-500) * ((float)(levelDistance/100) / (float) currentWorld->completeScore));
                                    // maximum extent of background scrolling (eg a background 700 pixels high could scroll a total of 200 pixels on a 500 pixel high game area)
                    else offset = -(currentWorld->backgrounds[layer]->h-500);
                    break;
                    
                /* Varying levels of small movement */
                case 1:
                case 2:
                case 3:
                case 4:
                    offset = - ((int)(levelDistance / (float)(10-2*layer)) % currentWorld->backgrounds[layer]->h);
                    break;
                    
                /* Normal level speed */
                case 5:
                    offset = -((int)levelDistance % currentWorld->backgrounds[layer]->h);
                    break;
                    
                /* Faster than level speed */
                case 6:
                    offset = -((int)(levelDistance*2) % currentWorld->backgrounds[layer]->h);
                    break;
                case 7:
                    offset = -((int)(levelDistance*4) % currentWorld->backgrounds[layer]->h);
                    break;
                
                /* Opposite direction for waterfalls etc */
                case 8: // slow(ish)
                    offset = ((int)(levelDistance*2) % currentWorld->backgrounds[layer]->h) - 500;
                    break;
                    
                case 9: // fast
                    offset = ((int)(levelDistance*4) % currentWorld->backgrounds[layer]->h) - 500;
                    break;
                    
                default:
                    offset=0;
                    break;
            }
            
            /* Draw at correct location */
            while(offset < mainScreen->h)
            {
                drawGraphic( currentWorld->backgrounds[layer], 0, offset );
                
                offset+=currentWorld->backgrounds[layer]->h;
                
            } // iterate over each repeat of background pic
            
        } // if graphic exists
            
    } // for each layer
    
    /* Reset the main drawing rect if we were clipping for world selection screen */
    if(cropped) SDL_SetClipRect(mainScreen, NULL);
}



/* Draw ceiling obstruction - spikes etc */
void drawCeilingObstruction(void)
{
    int x=0;
    
    /* Update animation */
    if(SDL_GetTicks()-currentWorld->ceilingAnimLastFrameTime > currentWorld->ceilingAnimFramePeriod && !gamePaused)
    {
        currentWorld->ceilingAnimFrame++;
        if(currentWorld->ceilingAnimFrame>currentWorld->ceilingAnim.numFrames-1) currentWorld->ceilingAnimFrame=0;
        currentWorld->ceilingAnimLastFrameTime=SDL_GetTicks();
    }
        
    
    /* repeatedly tile horizontally */
    while(x<300)
    {
        drawAnimation(&currentWorld->ceilingAnim, x, 0, currentWorld->ceilingAnimFrame);            	x+=currentWorld->ceilingAnim.w;
    }
}


/* Draw a number out of digits at a specific location */
/* alignment 0 for left, 1 for center, 2 for right */
void drawGameNumber( int displayNumber, int x, int y, int alignment )
{
    char numberString[9];
    int startDigit=0;
    int numberLength=0;
    int numberSpacing=0;
    int i=0;
    
    /* Convert integer to a string of digits */
    sprintf(numberString, "%8.8d", displayNumber);
    
    /* Find the first digit and the length of the number */
    for(i=1;i<=8;i++)
    {
        if(numberString[i-1]=='0') startDigit = i;
        else break;
    }
    numberLength=8-startDigit;
    
    /* Convert from ASCII to ints */
    for(i=0;i<8;i++)
        numberString[i] = (int)numberString[i]-48;
    
    /* Draw to screen */
    numberSpacing=22;
    for(i=startDigit;i<8;i++)
        drawAnimation(	&numbersMapAnim,
                        x + numberSpacing*(i-startDigit) - alignment*(numberSpacing/2)*numberLength,
                        y,
                        (int)numberString[i] );

}

/* Draw game interface - health bar etc */
void drawGameInterface(void)
{
    int healthBarOffsetY=55;
    animation* anim=NULL;
    float progress=0.0f;
    int drawX=0, drawY=0;
    
    /*
        Paused Image
    */
    if(gamePaused && pausedImg)
        drawGraphic(pausedImg, 150-pausedImg->w/2, 250-pausedImg->h/2);
    
    /*
        New Star Rating Achieved
    */
    if( nextStarTarget<5 && levelDistance/100 > currentWorld->starScore[nextStarTarget] )
    {
        starCompleteTime = lastFrameTime;
        
        /* Play ting ting ting! */
        if(soundOnOff)
        {
            /*	play sample on first free unreserved channel
                play it several times depending on how many stars the player has now */
            if(Mix_PlayChannel(-1, starRating, nextStarTarget)==-1)
                fprintf(stderr, "Mix_PlayChannel: %s\n",Mix_GetError());
        }
        
        nextStarTarget++;
    }
    if(starCompleteTime > 0)
    {
        drawStars((int)( -pow((((float)lastFrameTime-starCompleteTime-1000)/100.0), 2)+50), nextStarTarget );
        if(lastFrameTime - starCompleteTime > 3000) starCompleteTime = -1;
    }
    
    /*
        Ticking clock for powerups
    */
    if(warned) drawGraphic(clockImg, 212, 422);
    
    /*
        Current Powerup 
    */
    if(currentPowerup != NO_POWERUP)
    {
        switch(currentPowerup)
        {
            case HEAL:
                anim = &healAnim;
                drawX=5;
                drawY=15;
                break;
            
            case PARACHUTE:
                anim = &paraBagAnim;
                drawX=5;
                drawY=15;
                break;
                
            case JETPACK:
                anim = &jetpackAnim;
                drawX=5;
                drawY=15;
                break;
                
            case JUMP:
                anim = &jumpAnim;
                drawX=9;
                drawY=15;
                break;
                
            case SHIELD:
                anim = &shieldAnim;
                drawX=7;
                drawY=15;
                break;
                
            default: break;
        }
        /* If only just picked up, then jump to top left of screen before sitting there permanently */
        if(SDL_GetTicks()-powerupStartTime < 300)
        {
            progress = 1.0 - ((float)(SDL_GetTicks()-powerupStartTime)) / 300.0;
            drawAnimation(anim, (int)(progress*150 - 20), (int)(progress*(prevPickupY-levelDistance - 20)), 0);
        }
        /* Already picked up */
        else drawAnimation(anim, drawX, drawY, 0);
    }
    
    /*
        Health Bar
    */
    /* Just draw the complete graphic if either completely empty or completely full */
    if(playerHealth==10)
        drawGraphic(healthBarFull, 12, healthBarOffsetY);
    else if(playerHealth==0)
        drawGraphic(healthBarEmpty, 12, healthBarOffsetY);
    else
    {
        /* We will need to work out how much of each bar we need to draw */
        SDL_Rect blitFullSrc;
        SDL_Rect blitFullDest;
        SDL_Rect blitEmptySrc;
        SDL_Rect blitEmptyDest;
    
        /* Draw if the graphics exist */
        if(healthBarFull!=NULL && healthBarEmpty!=NULL)
        {
            /* Set up regions to blit */
            blitFullSrc.x=0;
            blitFullSrc.y=202-(20*playerHealth+1);
            blitFullSrc.w=17;
            blitFullSrc.h=20*playerHealth+1;
            blitFullDest.x=12;
            blitFullDest.y=202+healthBarOffsetY-(20*playerHealth+1);	// draw from bottom left corner
            blitFullDest.w=0;	//unused
            blitFullDest.h=0;	//unused
            
            blitEmptySrc.x=0;
            blitEmptySrc.y=0;
            blitEmptySrc.w=17;
            blitEmptySrc.h=202-(20*playerHealth+1);
            blitEmptyDest.x=12;
            blitEmptyDest.y=healthBarOffsetY;	// draw from bottom left corner
            blitEmptyDest.w=0;	//unused
            blitEmptyDest.h=0;	//unused
            
            /* Blit from graphics' surface onto screen surface */
            SDL_BlitSurface(healthBarFull, &blitFullSrc, mainScreen, &blitFullDest);
            SDL_BlitSurface(healthBarEmpty, &blitEmptySrc, mainScreen, &blitEmptyDest);
            
        }// else fprintf(stderr, "Error: Graphic missing\n");
    }
    
    /*
        World unlocked
    */
    if(worldCompleteTime > 0)
    {
        if(currentWorld->order >= numWorlds-1)
            drawGraphic(plunged, 70, (int)(-pow((( (float)lastFrameTime-worldCompleteTime-1000)/100.0), 2)+50) );
        else
            drawGraphic(worldUnlocked, 70, (int)(-pow((( (float)lastFrameTime-worldCompleteTime-1000)/100.0), 2)+50));
        if(lastFrameTime - worldCompleteTime > 3000) worldCompleteTime = -1;
    }
        

    /*
        Distance numbers
    */
    if(playerHealth>0)
        drawGameNumber( (int)(levelDistance/100), mainScreen->w-20, 10, 2 );
    else if(SDL_GetTicks()-diedTime < 300)
    {
        drawX = 280 - (130*(SDL_GetTicks()-diedTime))/300;
        drawY = (240*(SDL_GetTicks()-diedTime))/300 + 10;
        
        drawGameNumber( (int)(levelDistance/100), drawX, drawY, 2 );
    }
    else
        drawGameNumber( (int)(levelDistance/100), 150, 250, 2 );
}

/* Fill screen with platforms, and make the center one the start platform */
void randomiseAllPlatforms(void)
{
    worldPlatformRef* startPlatform=NULL;
    int i=0;

    for(i=0;i<9;i++)
    {
        randomisePlatform(i);
        platforms[i].y = (int)(i*60+levelDistance);
        platforms[i].isBottom=0;
    }
    platforms[9].isBottom=1;
    
    /* Find platformRef with the start type */
    for(i=0;i<16;i++)
        if(currentWorld->platformRefs[i] != NULL && currentWorld->platformRefs[i]->typeCode == currentWorld->startPlatform)
        {
            startPlatform = currentWorld->platformRefs[i];
            break;
        }
    
    /* platform START_PLATFORM is just below middle, vertically */
    platforms[START_PLATFORM].type = startPlatform;
    
    /* Place in the center of the screen */
    platforms[START_PLATFORM].x = 120;
}

void randomisePlatform( int platformNumber )
{
    platforms[platformNumber].type = getRandomPlatform();
    platforms[platformNumber].x = (randomInt(9)+1)  * 30;
        //hack to fix bug where testers said platforms went off edge
        if(platforms[platformNumber].x > 270) platforms[platformNumber].x=270;
        if(platforms[platformNumber].x < 30) platforms[platformNumber].x=30;
    platforms[platformNumber].frame = 0;
    platforms[platformNumber].lastFrameTime = SDL_GetTicks();
    platforms[platformNumber].active = 0;
}


/* Given the currentWorld, return a random platformRef, using frequency values */
worldPlatformRef* getRandomPlatform(void)
{
    int targetValue;
    int total=0;
    int i;
    
    targetValue = randomInt(currentWorld->frequencyTotal);
    for(i=0;i<16;i++)
    {
        if( currentWorld->platformRefs[i] != NULL )
        {
            total+=currentWorld->platformRefs[i]->frequency;
            if(targetValue<total) break;
        }
    }
    
    return currentWorld->platformRefs[i];
}


/* Function returns a random integer between 0 and the specified max value */
int randomInt(int maxValue)
{
    double r;
    
    r = ( (double)(rand()) / (double)(RAND_MAX) );
    
    return ((int)floor(r * maxValue));
}

/* Based on a float between 0 and 1, where 0.9 is likely and 0.1 is unlikely, return 1 or 0 (true or false) */
int randomTrueFalse(float frequency)
{
    double r;
    
    r = ( (double)(rand()) / (double)(RAND_MAX) );
    
    if(r<frequency) return 1; else return 0;
}


/* Uses mouseX and mouseY to determine whether mouse is in a given rect. returns 1 for true, 0 for false */
int mouseInRect(int x, int y, int w, int h)
{
    /* If in bounds */
    if( mouseX > x && mouseX < x+w && mouseY > y && mouseY < y+h )
        return 1;
        
    /* Else if not in bounds */
    else return 0;
}

