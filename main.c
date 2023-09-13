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
			if (CP_Input_KeyReleased(KEY_SPACE)) {
				CP_Engine_Terminate();
			}
		}
		else {
			float ww = (float)CP_System_GetWindowWidth();
			float wh = (float)CP_System_GetWindowHeight();
			
			CP_Settings_Fill(CP_Color_Create(255, 255, 255, 50));
			CP_Graphics_DrawRect(0, 0, ww, wh);
			CP_Settings_Fill(CP_Color_Create(50, 50, 50, 50));
			CP_Graphics_DrawRect(ww / 4, wh / 4, ww / 2, wh / 2);

			float bw = (ww * 4 / 39); //button width.
			float bh = (bw / 2); //button height
			float spacing = (bw / 2); //gap between both buttons
			float marginBottom = bh * 2; //gap between button and bottom border
			//In hopes to remove "MAGIC" numbers, everything is calculated based on the fullscreen window. 
			//This means I'm doing a lot of awkward math, but hypothetically everything should lineup on all monitors
			//I'm also reusing certain values in different variables (MarginBottom is equal to Button Width) because
			//   the variable acts as a named alias. The fact that it's equal to button width is coincidence.

			CP_Graphics_DrawRect(ww / 2 - bw - spacing, (wh / 4) * 3 - marginBottom, bw, bh);
			CP_Graphics_DrawRect(ww / 2 + spacing, (wh / 4) * 3 - marginBottom, bw, bh);

			CP_Settings_Fill(CP_Color_Create(0, 0, 0, 255));
			CP_Settings_Stroke(CP_Color_Create(0, 0, 0, 255));
			CP_Settings_StrokeWeight(2.0);

			CP_Font_DrawText("Quit", ww / 3 + 100, (wh / 3) * 2 - 150);
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
