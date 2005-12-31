/*
 *  main.c
 *  Primate Plunge
 *
 *  Created by Joseph Humfrey on Fri Jul 18 2003.
 *  Copyright (c) 2003 aelius productions. All rights reserved.
 *
 *  Licenced under uDevGame LICENSE Version 4b1
 *  See included COPYING file for details
 *
 *  Sets up main SDL/video mode stuff and calls load/enter game functions
 *
 */
   
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include <SDL.h>
#include <SDL_mixer.h>

#include "game.h"
#include "config.h"

/* Global surface which specifies the screen to draw on to */
SDL_Surface *mainScreen=NULL;


int main(int argc, char *argv[])
{
	Uint32 initflags = SDL_INIT_VIDEO || SDL_INIT_AUDIO;
	Uint8  video_bpp = 16;
	Uint32 videoflags = SDL_HWSURFACE;
	SDL_Surface* taskBarIcon;
	
	/* Initialise the SDL library */
	if ( SDL_Init(initflags) < 0 ) {
		fprintf(stderr, "Couldn't initialize SDL: %s\n",
			SDL_GetError());
		exit(1);
	}
        
        /* Initialise the SDL_Mixer library */
        // open 22KHz, signed 16bit, system byte order,
        //      stereo audio, using 1024 byte chunks
        if( Mix_OpenAudio(MIX_DEFAULT_FREQUENCY, MIX_DEFAULT_FORMAT, 2, 1024) == -1 ) {
            fprintf(stderr, "Mix_OpenAudio: %s\n", Mix_GetError());
            exit(2);
        }

	/* Set 300x500 video mode */
	mainScreen=SDL_SetVideoMode(300,500, video_bpp, videoflags);
        if (mainScreen == NULL) {
		fprintf(stderr, "Couldn't set 300x500x%d video mode: %s\n",
                        video_bpp, SDL_GetError());
		SDL_Quit();
		exit(2);
	}
        
    /* Set window caption to "Primate Plunge" */
    SDL_WM_SetCaption("Primate Plunge", "Primate Plunge");
    taskBarIcon = loadGraphic("taskBarIcon.bmp");
    SDL_WM_SetIcon(taskBarIcon, NULL);
    
        /* Load all game resources before going into the main menu */
        loadGame();
        enterGame();
        /* Game is finished */
        
	/* Clean up the SDL and SDL_Mixer libraries */
        Mix_CloseAudio();
	SDL_Quit();
	return(0);
}

