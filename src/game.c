//Include SDL to use... SDL
#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>
#include <SDL_mixer.h>

//Include header files I made
#include "boolean.h"
#include "graphics.h"
#include "gfunc.h"
#include "entity.h"

/* from gfunc.c */
extern SDL_Surface *screen;
extern SDL_Surface *background;

extern SDL_Surface *upMessage;
extern SDL_Surface *downMessage;
extern SDL_Surface *leftMessage;
extern SDL_Surface *rightMessage;

extern SDL_Surface *message;

extern TTF_Font *font;
extern SDL_Color textColor;
extern SDL_Rect clips[4];

extern SDL_Surface *seconds;

extern Mix_Music *music;
extern Mix_Chunk *scratch;
extern Mix_Chunk *high;
extern Mix_Chunk *med;
extern Mix_Chunk *low;

extern SDL_Rect camera;
extern SDL_Rect health;
extern SDL_Rect anger;

//objects
extern SDL_Surface *dot;

extern SDL_Event event;

entity_t *wall;

/* I will be making an unsigned integer */
Uint8 *keystates;
Uint32 currentTime = 0;

//create main or else (error LNK2001: unresolved external symbol _SDL_main)
int main(int argc, char *argv[])
{	
	entity_t *dot_ent;

	entity_t *enemy1;
	entity_t *enemy2;
	entity_t *enemy3;

//	entity_t *boss;

	//don't forget barhud
	
	//boolean created for making sure the application stays active until this boolean equals true
	bool done = false;

	/* Starting time for the timer */
	Uint32 start = 0;

	/* Flag for if the timer is running or not */
	bool running = true;
	
	/* Start everything (SDL, open a window, etc.) and make sure it did it successfully */
	if ( init() == false)
	{
		return 1;
	}
	
	/* This func. loads all the sprites and checks that I did it correctly */
	if ( load_Files() == false)
	{
		return 1;
	}
	
	//Start the timer right before the gameloop starts, will start at 0 and increment
	start = SDL_GetTicks();

	/* HUD */
	health.x = 10;
	health.y = 10;
	health.w = 100;
	health.h = 35;

	anger.x = 115;
	anger.y = 10;
	anger.w = 100;
	anger.h = 35;
	/* --- */

	/* PLAYER */
	dot_ent = Init_Ent();
	dot_ent->resetPosition = reset_Position;
	dot_ent->handle_Input = handle_Input;
	dot_ent->move = move;
	dot_ent->show = show;

	reset_Position(dot_ent);
	/* ------- */

	/* Enemy 1 */
	enemy1 = Init_Ent();
	
	enemy1->x = 600;
	enemy1->y = 350;

	enemy1->bBox.x = enemy1->x;
	enemy1->bBox.y = enemy1->y;

	enemy1->sprite = load_Image("sprite/red.png");

	enemy1->bBox.w = 64;
	enemy1->bBox.h = 64;
	
	enemy1->show = show_Enemy;
	/* ------- */

	/* Enemy 2 */
	enemy2 = Init_Ent();

	enemy2->sprite = load_Image("sprite/bluepng.png");

	enemy2->x = 770;
	enemy2->y = 350;

	enemy2->bBox.x = enemy2->x;
	enemy2->bBox.y = enemy2->y;

	enemy2->bBox.w = 64;
	enemy2->bBox.h = 64;
	/* ------- */

	/* Enemy 3 */
	enemy3 = Init_Ent();

	enemy3->sprite = load_Image("sprite/green.png");

	enemy3->x = 900;
	enemy3->y = 350;

	enemy3->bBox.x = enemy3->x;
	enemy3->bBox.y = enemy3->y;

	enemy3->bBox.w = 64;
	enemy3->bBox.h = 64;
	/* ------- */

	/* Wall */
	wall = Init_Ent();

	wall->sprite = load_Image("sprite/wall.jpg");

	wall->x = 400;
	wall->y = 40;

	wall->bBox.w = 40;
	wall->bBox.h = 400;

	wall->bBox.x = wall->x;
	wall->bBox.y = wall->y;
	/* ------- */

	/* GAME ------------------------------------------------------------------------- */
	do
	{
		//While there is still things to do
		while (SDL_PollEvent (&event))
		{
			handle_Input(dot_ent);

			//Do them
			//In the event that a key has been pressed...
			if ( event.type == SDL_KEYDOWN )
			{
				//system for checking what key has been pressed
				switch ( event.key.keysym.sym )
				{
					case SDLK_UP: //in the case the up button has been pressed
						if (Mix_PlayChannel (-1, scratch, 0) == -1) //arg. 1- -1 means looking for the nxt sound channel available to play sound
						{											//arg. 2- the sound that will be played
							return 1;								//arg. 3- how many times sound will loop
						}
						break;
					case SDLK_9:
						camera.x += 20;
						printf("camera ( %d, %d )\n", camera.x, camera.y);
						break;
					case SDLK_RIGHT:
						if (Mix_PlayChannel (-1, low, 0) == -1)
						{
							return 1;
						}
						break;
					case SDLK_0: //Completely stops music
						Mix_HaltMusic(); break;
					case SDLK_s:
						if (running == true) //if the timer is running
						{
							//STAHP
							running = false;
							start = 0; //restart the timer
						}
						else
						{
							//or maybe the timer never started
							running = true;
							start = SDL_GetTicks();
						}
					}
				}

			//If the user presses Quit (the x button on the window)
			if(event.type == SDL_QUIT)
			{
				//Game is done
				done = true;
			}

			move(dot_ent);
		}

		enemy1->bBox.x -= .1;

		set_Camera(dot_ent);

		/* Text keeps overlapping each other, recreate bg to fix problem */
		show_Surface (0, 0, background, screen, &camera);

		if (running == true)
			{
				char msg[20];
				sprintf( msg, "%s", FormatTimeString(start));

				seconds = TTF_RenderText_Solid (font, msg, textColor);
				show_Surface ((SCREEN_WIDTH - seconds->w ) / 2, 50, seconds, screen, NULL);

				SDL_FreeSurface( seconds );
			}

		show(dot_ent);

		show_Enemy(enemy1);
		show_Enemy(enemy2);
		show_Enemy(enemy3);

		show_Enemy(wall);

		SDL_FillRect ( screen, &health, SDL_MapRGB ( screen->format, 0, 0xFF, 0 ) );
		SDL_FillRect ( screen, &anger, SDL_MapRGB ( screen->format, 0x77, 0x77, 0x77 ) );

		/* this gives us an array of all the possible keystates and whether a key is pressed or not */
		keystates = SDL_GetKeyState( NULL );

		/* Constantly getting the raw time from SDL */
		currentTime = SDL_GetTicks();

		/* Function so that the screen is constantly updated so you can see things happening as they happen */
		SDL_Flip(screen);
		
	}
	
	while(!done); //Part of the do-while loop "Do game things while done is false (!done)"
	
	//When done equals true, the game escape the do-while loop (gameloop) and should reach the next line.
	//Free up the memory the game is using
	clear();

	//These last two lines fully exit the game.
	exit(0);
	return 0;

	/* GAME -------------------------------------------------------------------------- */
}