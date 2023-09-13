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

CP_Font bubbleLetters;

int paused = 0;
int pauseMenuShowing = 0;
float x = 0;
float y = 0;

void game_init(void)
{
	CP_System_Fullscreen();
	bubbleLetters = CP_Font_Load("Assets/fonts/AlloyInk-nRLyO.ttf");
}

void game_update(void)
{

	float ww = CP_System_GetWindowWidth();
	float wh = CP_System_GetWindowHeight();

	if (paused) {
		if (pauseMenuShowing) {
			//close pause menu
			if (CP_Input_KeyReleased(KEY_ESCAPE)) {
				paused = 0;
				pauseMenuShowing = 0;
			}
			if (CP_Input_KeyReleased(KEY_SPACE)) {
				CP_Engine_Terminate();
			}
		}
		else {
			
			CP_Settings_Fill(CP_Color_Create(255, 255, 255, 50));
			CP_Graphics_DrawRect(0, 0, ww, wh);
			CP_Settings_Fill(CP_Color_Create(50, 50, 50, 50));
			CP_Graphics_DrawRect(ww / 4, wh / 4, ww / 2, wh / 2);

			CP_Settings_Fill(CP_Color_Create(0, 0, 0, 255));
			CP_Settings_Stroke(CP_Color_Create(0, 0, 0, 255));
			CP_Settings_TextSize(70.0f);

			CP_Font_Set(bubbleLetters);
			CP_Settings_TextAlignment(CP_TEXT_ALIGN_H_CENTER, CP_TEXT_ALIGN_V_MIDDLE);
			CP_Font_DrawText("PAUSED", ww / 2, wh * 3 / 8);
			CP_Settings_TextSize(50.0f);
			CP_Font_DrawText("Press ESC to resume", ww / 2, wh * 4 / 8);
			CP_Font_DrawText("Press SPACE to quit", ww / 2, wh * 5 / 8);
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
	//CP_Font_Free(bubbleLetters);
}

int main(void)
{
	CP_Engine_SetNextGameState(game_init, game_update, game_exit);
	CP_Engine_Run();
	return 0;
}
