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
#include <stdio.h>
#include <stdlib.h>

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
CP_Image cloudTexture;

CP_Color BLACK;

int paused = 0;
int pauseMenuShowing = 0;

float playerX, playerY, velocityX, velocityY;
float ww, wh;
float drag = 0.1f;
float velocityIncrement = 0.2f;
float windSpeedX, windSpeedY;

struct Cloud {
	int size;
	float x;
	float y;
	int img_id; //0 to 12 variations
};

struct TextureRect {
	float w;
	float h;
	float x0;
	float y0;
	float x1; 
	float y1;
};

struct TextureRect CLOUD_TEXTURE_POSITIONS[] = {
	{140, 100, 1, 1, 140, 100},
	{140, 100, 140, 0, 280, 100},
	{100, 100, 280, 0, 380, 100},
	{140, 100, 380, 0, 520, 100},
	{260, 100, 520, 0, 680, 100},
	{100, 100, 720, 0, 820, 100}
};

int i = 5;

void game_init(void) {
	CP_System_Fullscreen();
	bubbleLetters = CP_Font_Load("Assets/fonts/AlloyInk-nRLyO.ttf");
	cloudTexture = CP_Image_Load("Assets/cloudtextures.png");
	BLACK = CP_Color_Create(0, 0, 0, 255);
	ww = CP_System_GetWindowWidth();
	wh = CP_System_GetWindowHeight();
	playerX = ww / 4;
	playerY = wh / 2;

	CP_Settings_Fill(BLACK);
	CP_Font_Set(bubbleLetters);
	CP_Settings_TextAlignment(CP_TEXT_ALIGN_H_CENTER, CP_TEXT_ALIGN_V_MIDDLE);
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

			CP_Settings_Fill(CP_Color_Create(50, 50, 50, 175));
			CP_Graphics_DrawRect(ww / 4, wh / 4, ww / 2, wh / 2);

			CP_Settings_Fill(CP_Color_Create(255,255,255,255));
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
		CP_Settings_Fill(CP_Color_Create(72, 255, 72, 255));
		CP_Graphics_DrawRect(0, wh * 0.80f, ww, wh / 4);
		CP_Settings_Fill(BLACK);

		playerX = ww / 2;
		playerY = wh / 2;

		CP_Graphics_DrawEllipse(playerX, playerY - 50, 80, 90);
		CP_Graphics_DrawLine(playerX - 40, playerY-50, playerX - 15, playerY + 40);
		CP_Graphics_DrawLine(playerX + 40, playerY-50, playerX + 15, playerY + 40);
		CP_Graphics_DrawRect(playerX - 15, playerY + 40, 30, 30);

		//Perpetual motion based on velocity
		//Velocity increases and decreases based on user input
		//When no user input applied, velocity will gradually go towards 0
		//MOVE THE PLAYER BY MOVING EVERYTHING ELSE! 
		//THE PLAYER NEVER ACTUALLY MOVES! 

		//playerX = (playerX >= ww) ? ww - drag : (playerX <= 0) ? 0 + drag : playerX + velocityX;
		//playerY = (playerY >= wh-230) ? wh-230-drag : (playerY <= 0) ? 0 + drag : playerY + velocityY;
		velocityX = (velocityX < 0) ? velocityX + drag : velocityX - drag;
		velocityY = (velocityY < 0) ? velocityY + drag : velocityY - drag;

		//COMMENTING OUT THE WINDSPEED
		//WILL REIMPLEMENT LATER
		/*windSpeedX += CP_Random_RangeFloat(-0.5, 0.5);
		windSpeedY += CP_Random_RangeFloat(-0.05, 0.05);*/

		CP_Settings_TextSize(50.0f);

		/*int len = snprintf(NULL, 0, "%.2f", windSpeedX);
		char* result = malloc(len + 1);
		snprintf(result, len + 1, "%.2f", windSpeedX);
		CP_Font_DrawText(result, ww/2, wh*2/6);
		free(result);*/

		int len = snprintf(NULL, 0, "%.2f", velocityX);
		char* result = malloc(len + 1);
		snprintf(result, len + 1, "%.2f", velocityX);
		CP_Font_DrawText(result, ww / 2 - 100, 100);
		free(result);

		/*len = snprintf(NULL, 0, "%.2f", windSpeedY);
		result = malloc(len + 1);
		snprintf(result, len + 1, "%.2f", windSpeedY);
		CP_Font_DrawText(result, ww / 2, wh * 4 / 6);
		free(result);*/

		len = snprintf(NULL, 0, "%.2f", velocityY);
		result = malloc(len + 1);
		snprintf(result, len + 1, "%.2f", velocityY);
		CP_Font_DrawText(result, ww / 2 + 100, 100);
		free(result);

		struct TextureRect current = CLOUD_TEXTURE_POSITIONS[i];
		CP_Image_DrawSubImage(cloudTexture, ww*0.7f, wh/2, current.w, current.h, current.x0, current.y0, current.x1, current.y1, 255);

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
