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
#include <math.h>
#define PI 3.14159265358979323846264

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

CP_Image cloudTexture;
CP_Image hotAirBalloonIMG;

CP_Color BLACK;

int paused = 0;
int pauseMenuShowing = 0;

float globalX, globalY;

CP_Vector directionVector;
float rotationAngle;
float rotationIncrement; //the increment changes based on speed - the faster you are, the harder it is to turn.
float rotationCap = 0.06f;

float speed;
float speedCap = 20;
float speedMin = 5; //The plane is always moving!
float speedIncrement = 0.25f;
float drag = 0.1f;
float windSpeedX, windSpeedY;

float ww, wh;

typedef struct {
	float size;
	float x;
	float y;
	int img_id; //0 to 12 variations
} Cloud;

int CLOUD_ARR_SIZE = 20;
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

typedef struct {
	float north;
	float south;
	float east;
	float west;
	float width;
	float height;
} Bounds;

Bounds bounds;

/* * * * * * *
* DRAW PLAYER *
 * * * * * * */
void drawPlayer(void) {
	CP_Settings_Fill(BLACK);
	CP_Settings_NoStroke();

	float bodyW = 30;
	float bodyH = 70;
	float bodyOffsetMult = 10;
	CP_Vector bodyOffsetVector = CP_Vector_Set(bodyOffsetMult * directionVector.x, bodyOffsetMult * directionVector.y);
	float wingW = 70;
	float wingYOffset = 9;
	float wingH = 50;

	float centerX = ww / 2;
	float centerY = wh / 2;

	float bodyAngle = acos(directionVector.y) * 180 / PI;
	bodyAngle = (directionVector.x < 0) ? bodyAngle : -bodyAngle;
	CP_Graphics_DrawEllipseAdvanced(centerX - bodyOffsetVector.x, centerY - bodyOffsetVector.y, bodyW, bodyH, bodyAngle);
	CP_Graphics_DrawTriangleAdvanced(
		centerX - wingW / 2,				//x1
		centerY + wingH / 2 - wingYOffset,	//y1
		centerX,							//x2
		centerY - wingH / 2 - wingYOffset,	//y2
		centerX + wingW / 2,				//x3
		centerY + wingH / 2 - wingYOffset,	//y3
		bodyAngle							//Rotation
	);

	CP_Settings_Stroke(CP_Color_Create(255,0,0,255));
	CP_Settings_StrokeWeight(3);
	//CP_Graphics_DrawLineAdvanced(centerX, centerY, directionVector.x * -100 + centerX, directionVector.y * -100 + centerY, rotationAngle);
	CP_Settings_StrokeWeight(2);
	CP_Settings_Stroke(BLACK);
}

/* * * * * * * * * * * * *
* RANDOMLY CREATE CLOUDS *
* * * * * * * * * * * * */
void createClouds(void) {
	//This for loop simply assigns random positions and images to the clouds array.
	//It is recalled every time the player warps through the screen.
	for (int i = 0; i < CLOUD_ARR_SIZE; i++) {
		activeClouds[i].size = 1;
		activeClouds[i].x = CP_Random_RangeFloat(bounds.west + ww / 2, bounds.east - ww / 2 - 200);
		activeClouds[i].y = CP_Random_RangeFloat(bounds.north + wh / 2, bounds.south - wh / 2 - 100);
		activeClouds[i].img_id = CP_Random_RangeInt(0, 12);
	}
}

void game_init(void) {
	activeClouds = malloc(CLOUD_ARR_SIZE * sizeof * activeClouds);
	directionVector = CP_Vector_Set(0, 1);

	CP_System_Fullscreen();
	cloudTexture = CP_Image_Load("Assets/cloudtextures.png");
	BLACK = CP_Color_Create(0, 0, 0, 255);
	
	ww = CP_System_GetWindowWidth();
	wh = CP_System_GetWindowHeight();

	bounds.north = -wh;
	bounds.east = 2*ww;
	bounds.south = 2*wh;
	bounds.west = -ww;
	bounds.width = bounds.east - bounds.west;
	bounds.height = bounds.south - bounds.north;

	createClouds();

	CP_Settings_Fill(BLACK);
	CP_Settings_TextAlignment(CP_TEXT_ALIGN_H_CENTER, CP_TEXT_ALIGN_V_MIDDLE);
	CP_Settings_ImageMode(CP_POSITION_CORNER);
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
		CP_Settings_Fill(BLACK);

		/*************\
		| DRAW CLOUDS |
		\*************/
		for (int i = 0; i < CLOUD_ARR_SIZE; i++) {
			Cloud currentCloud = activeClouds[i];
			TextureRect currentTexture = CLOUD_TEXTURE_POSITIONS[currentCloud.img_id];
			CP_Image_DrawSubImage(cloudTexture, currentCloud.x + globalX, currentCloud.y + globalY, currentCloud.size * currentTexture.w, currentCloud.size * currentTexture.h, currentTexture.x0, currentTexture.y0, currentTexture.x1, currentTexture.y1, 255);
		}

		//DRAW DEBUGGING SQUARE
		CP_Settings_NoFill();
		CP_Graphics_DrawRect(bounds.west + globalX, bounds.north + globalY, bounds.width, bounds.height);
		CP_Settings_Fill(BLACK);

		/*************\
		| DRAW PLAYER |
		\*************/
		drawPlayer();

		/*********************************\
		| CALCULATE VELOCITY AND POSITION |
		\*********************************/

		//x2 = cosAx1 − sinAy1
		//y2 = sinAx1 + cosAy1

		double newVX = cos(rotationAngle) * directionVector.x - sin(rotationAngle) * directionVector.y;
		double newVY = sin(rotationAngle) * directionVector.x + cos(rotationAngle) * directionVector.y;

		directionVector.x = newVX;
		directionVector.y = newVY;

		speed = (speed < 0) ? speed + drag : speed - drag;
		speed = (speed > speedCap) ? speedCap : (speed < speedMin) ? speedMin : speed;
		rotationAngle = (rotationAngle > rotationCap) ? rotationCap : (rotationAngle < -rotationCap) ? -rotationCap : rotationAngle;

		/*
		The statements above are known as Ternary Operators:

		VARIABLE = (CONDITION) ? [true value] : [false value];

		We can set the value of a variable based on a condition all in one line.

		Looking at this line: 
		speed = (speed < 0) ? speed + drag : speed - drag;

		This is equal to:
		if (speed < 0) {
			speed = speed + drag;
		} else {
			speed = speed - drag;
		}

		Notice how the speedCap and rotationCap lines are if-else-if-else combinations.

		The benefit is simplifying line usage for simple conditions like this. 
		However, 
			it can be confusing to many people, 
			it could look unpleasant if it grows too large, 
			it's only useful for simple conditions,
			and it can be hard to make changes.
		There are more negatives than positives, but I really enjoy the compactness.
		*/

		rotationIncrement = 3 / (20 * speed);
		//Minimum speed is 5; 3/(20*5) = 3/100 = 0.03 :: desired maximum rotation speed when slow
		//Maximum speed is 20; 3/(20*20) = 3/400 = 0.0075 :: mimimum rotation speed when fast
		//It's inherent in our brains that items at faster speeds will turn slower.

		globalX += directionVector.x * speed;
		globalY += directionVector.y * speed;

		if (globalX > bounds.width/2 || globalX < -bounds.width/2) {
			//if we're out of bounds, teleport to the opposite boundary.
			globalX *= -1;
			createClouds(); //no clouds are visible, great time to randomize them!
		}

		if (globalY > bounds.height / 2 || globalY < -bounds.height / 2) {
			globalY *= -1;
			createClouds();
		}

		/***********\
		| DRAW TEXT |
		\***********/
		CP_Settings_TextSize(40.0f);

		char buffer[50] = { 0 };
		sprintf_s(buffer, _countof(buffer), "Global X position: %.0f", -globalX);
		CP_Font_DrawText(buffer, 200, 50);

		sprintf_s(buffer, _countof(buffer), "Global Y position: %.0f", globalY);
		CP_Font_DrawText(buffer, 200, 100);

		sprintf_s(buffer, _countof(buffer), "Speed: %.0f", speed);
		CP_Font_DrawText(buffer, 200, 150);

		sprintf_s(buffer, _countof(buffer), "Direction: %.0f", acos(directionVector.y) * 180 / PI);
		CP_Font_DrawText(buffer, 200, 200);

		sprintf_s(buffer, _countof(buffer), "Rotation Inc: %.4f", rotationIncrement);
		CP_Font_DrawText(buffer, 200, 250);

		/*********\
		| CONTROL |
		\*********/

		if (CP_Input_KeyDown(KEY_W) || CP_Input_KeyDown(KEY_UP)) {
			speed += speedIncrement;
		} 
		
		if (CP_Input_KeyDown(KEY_S) || CP_Input_KeyDown(KEY_DOWN)) {
			speed -= speedIncrement;
		}

		if (CP_Input_KeyDown(KEY_A) || CP_Input_KeyDown(KEY_LEFT)) {
			rotationAngle -= rotationIncrement;
		} else if (CP_Input_KeyDown(KEY_D) || CP_Input_KeyDown(KEY_RIGHT)) {
			rotationAngle += rotationIncrement;
		} else {
			rotationAngle = 0;
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
	
}

int main(void) {
	CP_Engine_SetNextGameState(game_init, game_update, game_exit);
	CP_Engine_Run();
	return 0;
}
