//---------------------------------------------------------
// file:	main.c
// author:	Andy Malik
// email:	me@andymalik.com
//
// brief:	Main entry point for the sample project
//			of the CProcessing library
//
// documentation link:
// https://github.com/DigiPen-Faculty/CProcessing/wiki
//
// Copyright © 2020 DigiPen, All rights reserved.
//---------------------------------------------------------

#include "cprocessing.h"

/*
Ok so, the plan is to make a hot-air-balloon game, that's half exploration half action
you try to climb as high as you can - the higher you are, the harder it is to control, and the more impactful an obstacle is
The game ends when you crash, with various statistics determining a score

Stats:
Air Time
Height
Collectables
Hits (-negative)
Near Misses (+positive)

Controls:
WASD or Arrow Keys to Move

Features:
Wind Speed - determines how fast the balloon travels and in which direction. Becomes more erratic at higher elevations
Obstacles - Birds, Planes / Helicopters, Kites, Alien ships - all of them function similarly. They can hit the balloon.
Getting Hit - while I want there to be a way for players to skillfully save themselves, I think that's out of scope. For now, it's 3 strikes.
*/

int paused = 0;
int pauseMenuShowing = 0;
float x = 0;
float y = 0;

void game_init(void)
{
	CP_System_Fullscreen();
}

void unpause() {
	paused = 0;
	pauseMenuShowing = 0;
}

void game_update(void)
{
	if (paused) {
		if (pauseMenuShowing) {
			//close pause menu
			if (CP_Input_KeyReleased(KEY_ESCAPE)) {
				unpause();
			}
		}
		else {
			CP_Settings_Fill(CP_Color_Create(100, 100, 100, 100));
			float ww = (float)CP_System_GetWindowWidth();
			float wh = (float)CP_System_GetWindowHeight();
			CP_Graphics_DrawRect(0, 0, ww, wh);
			CP_Graphics_DrawRect(ww / 3, wh / 3, ww / 3, wh / 3);
			pauseMenuShowing = 1;
		}
	}
	else {
		//Play The Game
		CP_Graphics_ClearBackground(CP_Color_Create(0, 155, 0, 0));

		CP_Settings_Fill(CP_Color_Create(0, 0, 0, 255));
		CP_Graphics_DrawCircle(x, y, 50);

		x += 10;
		y += 10;
		if (x > CP_System_GetWindowWidth()) {
			x = 0;
		}
		if (y > CP_System_GetWindowHeight()) {
			y = 0;
		}

		/************\
		| PAUSE MENU |
		\************/
		if (CP_Input_KeyReleased(KEY_ESCAPE)) {
			paused = 1;
		}
	}
}



void game_exit(void)
{

}

int main(void)
{
	CP_Engine_SetNextGameState(game_init, game_update, game_exit);
	CP_Engine_Run();
	return 0;
}
