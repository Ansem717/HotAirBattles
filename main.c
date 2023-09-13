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

CP_Color BLACK;

int paused = 0;
int pauseMenuShowing = 0;

float playerX, playerY, velocityX, velocityY;
float ww, wh;
float drag = 0.1f;
float velocityIncrement = 0.2f;
float windSpeed;

void game_init(void) {
	CP_System_Fullscreen();
	bubbleLetters = CP_Font_Load("Assets/fonts/AlloyInk-nRLyO.ttf");
	BLACK = CP_Color_Create(0, 0, 0, 255);
	ww = CP_System_GetWindowWidth();
	wh = CP_System_GetWindowHeight();
	playerX = ww / 4;
	playerY = wh / 2;
}

void game_update(void) {

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
		} else {

			CP_Settings_Fill(CP_Color_Create(255, 255, 255, 50));
			CP_Graphics_DrawRect(0, 0, ww, wh);
			CP_Settings_Fill(CP_Color_Create(50, 50, 50, 50));
			CP_Graphics_DrawRect(ww / 4, wh / 4, ww / 2, wh / 2);

			CP_Settings_Fill(BLACK);
			CP_Font_Set(bubbleLetters);
			CP_Settings_TextAlignment(CP_TEXT_ALIGN_H_CENTER, CP_TEXT_ALIGN_V_MIDDLE);

			CP_Settings_TextSize(70.0f);
			CP_Font_DrawText("PAUSED", ww / 2, wh * 3 / 8);

			CP_Settings_TextSize(50.0f);
			CP_Font_DrawText("Press ESC to resume", ww / 2, wh * 4 / 8);
			CP_Font_DrawText("Press SPACE to quit", ww / 2, wh * 5 / 8);

			pauseMenuShowing = 1;
		}
	} else {
		//Play The Game
		CP_Graphics_ClearBackground(CP_Color_Create(32, 192, 255, 255));

		//Crude Hot Air Balloon
		CP_Settings_Fill(BLACK);
		CP_Graphics_DrawEllipse(playerX, playerY, 150, 200);
		CP_Graphics_DrawLine(playerX - 75, playerY, playerX - 25, playerY + 170);
		CP_Graphics_DrawLine(playerX + 75, playerY, playerX + 25, playerY + 170);
		CP_Graphics_DrawRect(playerX - 25, playerY + 170, 50, 50);

		//Perpetual motion based on velocity
		//Velocity increases and decreases based on user input
		//When no user input applied, velocity will gradually go towards 0
		//TODO: Velocity will gradually go towards *wind speed*
		playerX += velocityX;
		playerY += velocityY;
		velocityX = (velocityX <= 0) ? velocityX + drag : velocityX - drag;
		velocityY = (velocityY <= 0) ? velocityY + drag : velocityY - drag;

		if (CP_Input_KeyDown(KEY_W) || CP_Input_KeyDown(KEY_UP) ) {
			velocityY -= velocityIncrement;
		}
		if (CP_Input_KeyDown(KEY_A) || CP_Input_KeyDown(KEY_LEFT)) {
			velocityX -= velocityIncrement;
		}
		if (CP_Input_KeyDown(KEY_S) || CP_Input_KeyDown(KEY_DOWN)) {
			velocityY += velocityIncrement;
		}
		if (CP_Input_KeyDown(KEY_D)|| CP_Input_KeyDown(KEY_RIGHT)) {
			velocityX += velocityIncrement;
		}


		/************\
		| PAUSE MENU |
		\************/
		if (CP_Input_KeyReleased(KEY_ESCAPE)) {
			paused = 1;
		}
	}
}



void game_exit(void) {
	//CP_Font_Free(bubbleLetters);
}

int main(void) {
	CP_Engine_SetNextGameState(game_init, game_update, game_exit);
	CP_Engine_Run();
	return 0;
}
