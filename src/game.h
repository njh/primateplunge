/*
 *  game.h
 *  Primate Plunge
 *
 *  Created by Joseph Humfrey on Fri Jul 18 2003.
 *  Copyright (c) 2003 aelius productions. All rights reserved.
 *
 *  Licenced under uDevGame LICENSE Version 4b1
 *  See included COPYING file for details
 *
 */


#include <SDL.h>
#include <SDL_mixer.h>



/* The platform directly below the monkey when the game starts */
#define START_PLATFORM	(5)

/* Maximum length of a string/filepath */
#define	MAX_STRING_LEN	(1024)

/* Maximum number of worlds that can be loaded */
#define MAX_NUM_WORLDS	(32)


/* All the different type of platform behaviours which can occur in different worlds */
typedef enum _platformType {
    NORMAL=1,
    SPRING=20,
    FALLAWAY=30,		// platform falls away off screen (animates to last frame as it goes)
    REVOLVE=32,			// platform animates once and goes back to normal
    DESTROY=35,			// platform animates once and stays on last frame
    CONVEYORRIGHT=60,
    CONVEYORLEFT=61,
    BOUNCE=70,
    DAMAGE=100,
    CORRODESLOW=110,
    CORRODEFAST=111
} platformType;

/* All the different types of powerup */
typedef enum _powerupType {
    NO_POWERUP = 0,
    HEAL = 10,
    SHIELD = 20,
    PARACHUTE = 30,
    JETPACK = 40,
    JUMP = 50
} powerupType;

/* The menus the player could be in */
typedef enum _menuName {
    MAIN,
    WORLD_CHOICE,
    OPTIONS,
    SURE_DELETE_SCORES,
    INSTRUCTIONS
} menuName;

/* Animations were initially just used for character animations, loaded from animation strips */
/* Now used for lots of different things */
typedef struct _animation {
    int w, h, numFrames;
    SDL_Surface* frames;
} animation;

/* Worlds have one of these for each type of platform they contain */
/* Worlds can contain up to 16 of them, although this is limited to the number of
    types of platforms you can have in Chasm */
typedef struct _worldPlatformRef {
    platformType typeCode;
    animation anim;
    int framePeriod;	// how many millisecs to display each frame for
    int yOffset;	// offset of physical collision surface rather than top of graphic
    int width;		// width of collision surface
    Mix_Chunk* snd;
    int frequency;
} worldPlatformRef;

/* World data */
typedef struct _world {
    int order;				// which array place it is in
    char name[8];			// name/id reference
    SDL_Surface* frameGraphic;		// frame drawn in world selection screen
    animation ceilingAnim;		// e.g. falling ceiling spikes
    Mix_Chunk* ambientSnd;
    int ceilingAnimFramePeriod;
    int ceilingAnimFrame;
    int ceilingAnimLastFrameTime;
    SDL_Surface* backgrounds[8];	// background layer graphics
    int scrollSpeeds[8];
    worldPlatformRef* platformRefs[16];	// types of platforms, each with own graphics and frequency ratio
    platformType startPlatform;		// The type of platform the player appears on to start with
    powerupType powerups[8];		// A list of the powerups in this world
    float powerupFrequencies[8];	// A list of frequencies corresponding to the powerups above
    int scrollSpeed;			// The speed that the screen moves down at - directly affects difficulty
    float gravity;			// How fast the player will fall.
    int frequencyTotal;			// sum of frequencies of all platformRefs, so percentage of each can be calculated
    int completeScore;			// score required to get to unlock next world
    int highScore;			// record high score on this world
    int starScore[5];			// scores required to get stars 1-5
    int numStars;			// how many stars player has for this world
} world;

/* Platforms */
/* The actual objects in the game */
typedef struct _platform {
    int x, y;			// Position in world. All world relative, not screen relative
    int isBottom;		// 1 = bottom plaform on screen, 0 = not
    worldPlatformRef* type;
    int frame;
    int lastFrameTime;
    int active;			// whether platform has just been stepped on
} platform;

/* Sound sets */
/* For sounds which have several variations */
typedef struct _soundSet {
    int size;
    Mix_Chunk** soundArray;
} soundSet;


/* Glabals in main.c */
SDL_Surface *mainScreen;

/* Globals in game.c */
extern world* currentWorld;
extern world* worlds[32];
extern int numWorlds;
extern world* currentWorld;



/* Main load function loads all game data */
void loadGame( void );

/* Check to see if a file exists */
int fileExists( char* filepath );

/* Locates game resource file by testing paths */
char* locateFile( char* filename, char* subdir );

/* Loads a sound effect and returns the Mix_Chunk pointer */
Mix_Chunk* loadSound(char*);

/* Loads a music file and returns the Mix_Music pointer */
Mix_Music* loadMusic(char* filepath);

/* Initialises a sound set including the array which holds the Mix_Chunk*s */
void CreateSoundSet(soundSet*, int);

/* Loads a sound effect into a free array space in a sound array */
void AddSoundToSet(char*, soundSet*);

/* Loads an image with a background transparency of magenta and returns the SDL_Surface */
SDL_Surface* loadGraphic(char*);

/* Loads animation (for character) given a filepath and the number of frames */
void loadAnimation(char*, int, animation*);

/* Open the preferences file */
FILE* openPrefsScores( const char* mode );

/* Saves prefs and scores into a file */
void savePrefsScores( void );

/* Reads in preference and scores */
void readPrefsScores( void );

/* Calculate how many stars player has for this world */
/* If the number of stars has increased, then return the new star rating */
int calculateWorldStars( int );

/* Simple function controls main menu/game system */
void enterGame( void );

/* Function enters the main menu */
int enterMenu( void );

/* refresh the screen, set the new menu, flush events */
void changeMenu( menuName );

/* Show a 300x500 graphic until the player presses a button or the timer times out */
void showGraphic(SDL_Surface*, int, int);

/* Update and draw functions for menu */
void drawMenu( void );

/* Draw medal display for menu - silver star + smaller stars for star rating */
void drawMedal( void );

/* Function enters the main game loop */
void startGame( void );

/* Functions do slightly different stuff */
/* gameControls sets keyDown globals, while menuControls calls functions based on mouse clicks */
int getGameControls( void );
int getMenuControls( void );

/* Checks to see if currentWorld is locked, for world selection screen */
int worldLocked( void );

/* Positions player in the center of the start platform */
void resetPlayer( void );

/* Updates the positions of the platforms, and repositions and changes them when they go offscreen */
/* A platform which goes off the top of the screen wraps back to the bottom and mutates */
void updatePlatforms( void );

/* Simplified version of updatePlayer so the player just falls off the bottom of the screen */
void updateDeadPlayer( void );

/* Updates positioning/physics of player */
void updatePlayer( void );

/* Updates the timer etc on powerups / collision with player */
void updatePowerups( void );

/* Draws powerups in level */
void drawPowerups( void );

/* Use SDL_Mixer to play a sound, playing just once on a free channel */
void playSound(Mix_Chunk*);

/* Play a random sound from a sound set */
void playSoundSet(soundSet*);

/* Draws a particular SDL_Surface graphic at a position on the screen */
void drawGraphic(SDL_Surface*, int, int);

/* Draws an animation frame at a position on the screen */
void drawAnimation(animation*, int, int, int);

/* Draw all the platforms onto the screen */
void drawPlatforms(void);

/* Draw the player in his current position with the correct animation */
void drawPlayer(void);

/* Draw the parallax layered backgrounds */
/* input is whether they are cropped (for world selection) */
/* this gets called twice - once for backgrounds in front of player, once for backgrounds behind */
/* the two second parameters specify the range of backgrounds to draw */
void drawBackgrounds(int, int, int);

/* Draw ceiling obstruction - spikes etc */
void drawCeilingObstruction(void);
        
/* Draw a number out of digits at a specific location */
void drawGameNumber( int, int, int, int );

/* Draw game interface - health bar etc */
void drawGameInterface(void);

/* Draw star rating in center of screen at a particular height */
void drawStars( int, int );

/* Sets distance through level based on time */
void scrollWorld(void);

/* Fill screen with platforms, and make the center one the start platform */
void randomiseAllPlatforms(void);

/* Randomises the x position and type of platform for a specific platform in the array */
void randomisePlatform(int platformNumber);

/* Given the currentWorld, return a random platformRef, using frequency values */
worldPlatformRef* getRandomPlatform(void);

/* Function returns a random integer between 0 and the specified max value */
int randomInt(int);

/* Based on a float between 0 and 1, where 0.9 is likely and 0.1 is unlikely, return 1 or 0 (true or false) */
int randomTrueFalse(float);

/* Uses mouseX and mouseY to determine whether mouse is in a given rect. returns 1 for true, 0 for false */
int mouseInRect(int, int, int, int);


/* In worlds.c */
void loadWorlds(void);

