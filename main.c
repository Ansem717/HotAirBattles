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
CP_Image hotAirBalloonIMG;

CP_Color BLACK;

int paused = 0;
int pauseMenuShowing = 0;

float globalX, globalY, velocityX, velocityY;
float velocityXCap = 8;
float velocityYCap = 5;

float ww, wh;
float drag = 0.05f;
float velocityIncrement = 0.1f;
float windSpeedX, windSpeedY;

typedef struct {
	float size;
	float x;
	float y;
	int img_id; //0 to 12 variations
} Cloud;

int CLOUD_ARR_SIZE = 15;
Cloud* activeClouds;

typedef struct {
	float w;
	float h;
	float x0;
	float y0;
	float x1;
	float y1;
} TextureRect;

TextureRect CLOUD_TEXTURE_POSITIONS[] = {
	{140, 100, 1, 1, 140, 100},		//0
	{140, 100, 140, 0, 280, 100},	//1
	{100, 100, 280, 0, 380, 100},	//2
	{140, 100, 380, 0, 520, 100},	//3
	{260, 100, 520, 0, 680, 100},	//4
	{100, 100, 720, 0, 820, 100},	//5
	{140, 100, 1, 100, 140, 200},	//6
	{200, 100, 140, 100, 340, 200},	//7
	{140, 100, 340, 100, 480, 200},	//8
	{210, 100, 490, 100, 700, 200},	//9
	{140, 100, 710, 100, 850, 200}, //10
	{300, 100, 1, 200, 305, 305},	//11
	{210, 120, 330, 190, 530, 310}	//12
};

void game_init(void) {
	activeClouds = malloc(CLOUD_ARR_SIZE * sizeof * activeClouds);

	CP_System_Fullscreen();
	bubbleLetters = CP_Font_Load("Assets/fonts/AlloyInk-nRLyO.ttf");
	cloudTexture = CP_Image_Load("Assets/cloudtextures.png");
	hotAirBalloonIMG = CP_Image_Load("Assets/hot-air-balloon.png");
	BLACK = CP_Color_Create(0, 0, 0, 255);
	ww = CP_System_GetWindowWidth();
	wh = CP_System_GetWindowHeight();

	for (int i = 0; i < CLOUD_ARR_SIZE; i++) {
		activeClouds[i].size = 1;
		activeClouds[i].x = CP_Random_RangeFloat(-ww/2, ww+ ww/2);
		activeClouds[i].y = CP_Random_RangeFloat(-wh, wh / 2);
		activeClouds[i].img_id = CP_Random_RangeInt(0, 12);
	}

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

			CP_Settings_Fill(CP_Color_Create(255, 255, 255, 255));
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
		// DRAW BACKGROUND (Sky)
		CP_Graphics_ClearBackground(CP_Color_Create(32, 192, 255, 255));

		// DRAW GRASS
		CP_Settings_Fill(CP_Color_Create(72, 255, 72, 255));
		CP_Graphics_DrawRect(0, wh * 0.80f + globalY, ww, wh / 4);
		CP_Settings_Fill(BLACK);

		/*************\
		| DRAW CLOUDS |
		\*************/
		for (int i = 0; i < CLOUD_ARR_SIZE; i++) {
			Cloud currentCloud = activeClouds[i];
			TextureRect currentTexture = CLOUD_TEXTURE_POSITIONS[currentCloud.img_id];
			CP_Image_DrawSubImage(cloudTexture, currentCloud.x + globalX, currentCloud.y + globalY, currentCloud.size * currentTexture.w, currentCloud.size * currentTexture.h, currentTexture.x0, currentTexture.y0, currentTexture.x1, currentTexture.y1, 255);
		}

		//DRAW DEBUGGING LINES
		CP_Settings_Stroke(BLACK);
		float x1 = -ww / 2;
		float x2 = ww + ww / 2;
		float y1 = -wh;
		float y2 = wh / 2;
		CP_Graphics_DrawLine(x1 + globalX, y1 + globalY, x1 + globalX, y2 + globalY);
		CP_Graphics_DrawLine(x2 + globalX, y1 + globalY, x2 + globalX, y2 + globalY);
		CP_Graphics_DrawLine(x1 + globalX, y1 + globalY, x2 + globalX, y1 + globalY);
		CP_Graphics_DrawLine(x1 + globalX, y2 + globalY, x2 + globalX, y2 + globalY);


		/*************\
		| DRAW PLAYER |
		\*************/
		CP_Image_Draw(hotAirBalloonIMG, ww / 2, wh / 2 - 20, 100, 200, 255);

		velocityX = (velocityX < 0) ? velocityX + drag : velocityX - drag;
		velocityX = (velocityX > velocityXCap) ? velocityXCap : (velocityX < -velocityXCap) ? -velocityXCap : velocityX;
		velocityY = (velocityY < 0) ? velocityY + drag : velocityY - drag;
		velocityY = (velocityY > velocityYCap) ? velocityYCap : (velocityY < -velocityYCap) ? -velocityYCap : velocityY;

		//Global X is positive for right, negative for left
		//Global Y is positive for DOWN, negative for up
		globalX += -velocityX;
		globalY = (globalY - velocityY <= 0) ? 0 : globalY - velocityY;


		CP_Settings_TextSize(50.0f);

		int len = snprintf(NULL, 0, "%.0f", globalX);
		char* result = malloc(len + 1);
		snprintf(result, len + 1, "%.0f", globalX);
		CP_Font_DrawText(result, ww/2 - 300, 100);
		free(result);

		len = snprintf(NULL, 0, "%.2f", velocityX);
		result = malloc(len + 1);
		snprintf(result, len + 1, "%.2f", velocityX);
		CP_Font_DrawText(result, ww / 2, 100);
		free(result);

		len = snprintf(NULL, 0, "%.0f", globalY);
		result = malloc(len + 1);
		snprintf(result, len + 1, "%.0f", globalY);
		CP_Font_DrawText(result, ww / 2 - 150, 100);
		free(result);

		len = snprintf(NULL, 0, "%.2f", velocityY);
		result = malloc(len + 1);
		snprintf(result, len + 1, "%.2f", velocityY);
		CP_Font_DrawText(result, ww / 2 + 150, 100);
		free(result);


		/*********\
		| CONTROL |
		\*********/

		if (CP_Input_KeyDown(KEY_W) || CP_Input_KeyDown(KEY_UP)) {
			velocityY -= velocityIncrement;
		} 
		
		if (CP_Input_KeyDown(KEY_S) || CP_Input_KeyDown(KEY_DOWN)) {
			velocityY += velocityIncrement;
		}

		if (CP_Input_KeyDown(KEY_A) || CP_Input_KeyDown(KEY_LEFT)) {
			velocityX -= velocityIncrement;
		}
		
		if (CP_Input_KeyDown(KEY_D) || CP_Input_KeyDown(KEY_RIGHT)) {
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
