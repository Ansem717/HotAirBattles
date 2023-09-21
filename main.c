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
#include <stdbool.h>
#define PI 3.14159265358979323846264

CP_Image cloudTexture, redhitFlash, coinIMG;

CP_Color BLACK, BLUE;
float ww, wh; //window width and window height

int paused, pauseMenuShowing, isIFraming;
char buffer[50] = { 0 };

float globalX, globalY;

CP_Vector directionVector, centerVector;
float rotationAngle, rotationIncrement, rotationCap;

float speed, speedCap, speedMin, speedIncrement, drag;

float iFrameDuration, iFrameStart, flashAlpha;
int remainingLives, score;

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

//These values "cut" the CloudTextures.png image into subimages to randomize clouds
TextureRect CLOUD_TEXTURE_POSITIONS[] = {
	{111,  73,   5,  18, 116,  91},	//0
	{122,  58, 142,  23, 264,  81},	//1
	{ 90,  48, 288,  26, 378,  74},	//2
	{104,  46, 410,  30, 514,  76},	//3
	{135,  86, 543,  10, 678,  96},	//4
	{ 91,  69, 724,  13, 815,  82},	//5
	{112,  59,   8, 120, 120, 179},	//6
	{176,  70, 138, 112, 314, 182},	//7
	{126,  69, 342, 115, 468, 184},	//8
	{199, 100, 494, 112, 693, 183},	//9
	{137,  70, 718, 110, 845, 180}, //10
	{187,  93,  10, 209, 287, 302},	//11
	{184,  84, 339, 203, 523, 287}	//12
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

CP_Vector activeCoin, mappedCoinVector;
float coinYPos, coinVelocity, coinCap, coinAlpha, coinFadeSpeed;

float deathAlpha;
int dminutes, dseconds;
float timeOfRestart;

void death_init(void);
void death_update(void);
void death_exit(void);
void game_init(void);
void game_update(void);
void game_exit(void);

void initGlobalVariables(void) {
	paused = false;
	pauseMenuShowing = false;
	isIFraming = false;

	activeClouds = malloc(CLOUD_ARR_SIZE * sizeof * activeClouds);

	globalX = 0;
	globalY = bounds.height / 2;
	directionVector = CP_Vector_Set(0, 1);

	rotationAngle = 0;
	rotationIncrement = 0; //the increment changes based on speed - the faster you are, the harder it is to turn.
	rotationCap = 0.06f;

	speed = 0;
	speedCap = 20;
	speedMin = 5; //The plane is always moving!
	speedIncrement = 0.25f;
	drag = 0.1f;

	iFrameDuration = 1;
	iFrameStart = 0;
	flashAlpha = 0;

	remainingLives = 1;
	score = 0;

	coinVelocity = 1;
	coinYPos = 0;
	coinCap = 10;
	coinAlpha = 255;
	coinFadeSpeed = 1;
}

void initBounds(void) {
	bounds.north = -wh;
	bounds.east = 2 * ww;
	bounds.south = 2 * wh;
	bounds.west = -ww;
	bounds.width = bounds.east - bounds.west;
	bounds.height = bounds.south - bounds.north;
}

/* * * * * * *
* DRAW PLAYER *
 * * * * * * */
void drawPlayer(CP_Color c) {
	CP_Settings_Fill(c);
	CP_Settings_NoStroke();

	float bodyW = 30;
	float bodyH = 70;
	float bodyOffsetMult = 10;
	CP_Vector bodyOffsetVector = CP_Vector_Set(bodyOffsetMult * directionVector.x, bodyOffsetMult * directionVector.y);
	centerVector = CP_Vector_Set(ww / 2 - bodyOffsetVector.x, wh / 2 - bodyOffsetVector.y);
	float wingW = 70;
	float wingYOffset = 9;
	float wingH = 50;

	float centerX = ww / 2;
	float centerY = wh / 2;

	float bodyAngle = acos(directionVector.y) * 180 / PI;
	bodyAngle = (directionVector.x < 0) ? bodyAngle : -bodyAngle;

	
	if (!isIFraming || CP_System_GetFrameCount() % 10 < 5) {
		//Only draw the body if we're not iFraming OR if we are iFraming, the only draw the body every other frame (flash).

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
	}
	//DEBUGGING SHAPES
	/*CP_Settings_Stroke(CP_Color_Create(255, 255, 0, 255));
	CP_Settings_StrokeWeight(3);
	CP_Settings_NoFill();
	CP_Graphics_DrawLineAdvanced(centerX, centerY, directionVector.x * -100 + centerX, directionVector.y * -100 + centerY, rotationAngle);
	CP_Settings_Stroke(CP_Color_Create(255, 0, 0, 255));
	CP_Graphics_DrawCircle(centerX - bodyOffsetVector.x, centerY - bodyOffsetVector.y, 70);
	CP_Settings_StrokeWeight(2);
	CP_Settings_Stroke(BLACK);
	CP_Settings_Fill(BLACK);*/
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
		activeClouds[i].img_id = CP_Random_RangeInt(0, 11); //Should be 0 to 12 but the last cloud in the texture pack isn't great for collision.
	}
}

/* * * * * * * * * * * *
* RANDOMLY CREATE COIN *
* * * * * * * * * * * */
void createCoin(void) {
	activeCoin.x = CP_Random_RangeFloat(bounds.west + ww / 2, bounds.east - ww / 2 - 200);
	activeCoin.y = CP_Random_RangeFloat(bounds.north + wh / 2, bounds.south - wh / 2 - 100);
	mappedCoinVector = CP_Vector_Set(activeCoin.x + globalX, activeCoin.y + globalY);
}

void game_init(void) {
	CP_System_Fullscreen();
	cloudTexture = CP_Image_Load("Assets/cloudtextures.png");
	redhitFlash = CP_Image_Load("Assets/redhit.png");
	coinIMG = CP_Image_Load("Assets/coin.png");
	BLACK = CP_Color_Create(0, 0, 0, 255);
	BLUE = CP_Color_Create(32, 192, 255, 255);
	
	ww = CP_System_GetWindowWidth();
	wh = CP_System_GetWindowHeight();

	initBounds();
	initGlobalVariables();

	createClouds();
	createCoin();

	CP_Settings_Fill(BLACK);
	CP_Settings_TextAlignment(CP_TEXT_ALIGN_H_CENTER, CP_TEXT_ALIGN_V_MIDDLE);
	CP_Settings_ImageMode(CP_POSITION_CORNER);
}

void game_update(void) {
	if (paused) {
		if (pauseMenuShowing) {
			//close pause menu
			if (CP_Input_KeyReleased(KEY_ESCAPE)) {
				paused = false;
				pauseMenuShowing = false;
			} else if (CP_Input_KeyReleased(KEY_R)) {
				CP_Engine_SetNextGameStateForced(game_init, game_update, game_exit);
			} else if (CP_Input_KeyReleased(KEY_Q) || CP_Input_KeyReleased(KEY_SPACE)) {
				CP_Engine_Terminate();
			}
		} else {

			CP_Settings_Fill(CP_Color_Create(50, 50, 50, 175));
			CP_Graphics_DrawRect(ww / 4, wh / 4, ww / 2, wh / 2);

			CP_Settings_Fill(CP_Color_Create(255, 255, 255, 255));
			CP_Settings_TextAlignment(CP_TEXT_ALIGN_H_CENTER, CP_TEXT_ALIGN_V_MIDDLE);

			CP_Settings_TextSize(70.0f);
			CP_Font_DrawText("PAUSED", ww / 2, wh * 5 / 16);

			CP_Settings_TextSize(50.0f);
			CP_Font_DrawText("Press ESC to Go Back", ww / 2, wh * 7 / 16);
			CP_Font_DrawText("Press Q to Quit", ww / 2, wh * 17 / 32);
			CP_Font_DrawText("Press R to Restart", ww / 2, wh * 20 / 32);

			pauseMenuShowing = true;
		}
	} else {
		// DRAW BACKGROUND (Sky)
		CP_Graphics_ClearBackground(BLUE);
		CP_Settings_Fill(BLACK);

		/*************\
		| DRAW CLOUDS |
		\*************/
		for (int i = 0; i < CLOUD_ARR_SIZE; i++) {
			Cloud currentCloud = activeClouds[i];
			TextureRect currentTexture = CLOUD_TEXTURE_POSITIONS[currentCloud.img_id];
			CP_Image_DrawSubImage(cloudTexture, currentCloud.x + globalX, currentCloud.y + globalY, currentCloud.size * currentTexture.w, currentCloud.size * currentTexture.h, currentTexture.x0, currentTexture.y0, currentTexture.x1, currentTexture.y1, 255);
			
			float widthScalar = 0.8f;
			float heightScalar = 0.7f;
			//DEBUG COLLISION CIRCLE
			//CP_Settings_NoFill();
			//CP_Settings_Stroke(CP_Color_Create(255, 0, 0, 255));
			//CP_Graphics_DrawEllipse(
			//	activeClouds[i].x + globalX + currentTexture.w/2, 
			//	activeClouds[i].y + globalY + currentTexture.h/2 + 5, 
			//	currentTexture.w * widthScalar, //a percentage of actual width 
			//	currentTexture.h * heightScalar
			//);
			//CP_Settings_Fill(BLACK);
			//CP_Settings_Stroke(BLACK);

			CP_Vector currentCloudVector = CP_Vector_Set(currentCloud.x + globalX + currentTexture.w / 2, currentCloud.y + globalY + currentTexture.h / 2);
			//CP_Graphics_DrawLine(currentCloudVector.x, currentCloudVector.y, centerVector.x, centerVector.y);
			
			/* 
			to get the radius of the cloud ellipse collision:
			r = ab / root(a * a * sin^2(theta) + b * b * cos^2(theta))
			where:
				a = currentTexture.w * 0.75 / 2
				b = cloud height / 2
				theta = horiztonal angle towards ship

			To get the horizontal angle towards the ship:
				acos(plane.y-cloud.y / distance) * 180 / PI
			*/

			double a = currentTexture.w * widthScalar / 2;
			double b = currentTexture.h * heightScalar / 2;
			double x = centerVector.x - currentCloudVector.x; 
			double y = centerVector.y - currentCloudVector.y;
			double distance = sqrt(x * x + y * y);
			double t = acos(x / distance);
			double ellipseRadiusTowardsPlayer = a*b / sqrt(a*a*sin(t)*sin(t) + b*b*cos(t)*cos(t));

			//COLISION
			if (!isIFraming && ellipseRadiusTowardsPlayer + 35 > distance) {
				remainingLives--;
				if (remainingLives <= 0) {
					//PLAYER DIED
					//instead of running iFrames, let's swap to the death gamestate
					CP_Engine_SetNextGameState(death_init, death_update, death_exit);
					return;
				}
				isIFraming = true;
				flashAlpha = 255;
				iFrameStart = CP_System_GetSeconds();
			}
		}

		/***********\
		| DRAW COIN |
		\***********/
		float size = 80;
		mappedCoinVector.x = activeCoin.x + globalX + size/2;
		mappedCoinVector.y = activeCoin.y + globalY + coinYPos + size / 2;
		CP_Image_Draw(coinIMG, mappedCoinVector.x, mappedCoinVector.y, size, size, coinAlpha);
		coinYPos += coinVelocity;
		if (coinYPos > coinCap || coinYPos < -coinCap) coinVelocity *= -1;
		//DEBUG LINE TOWARDS COIN
		CP_Settings_StrokeWeight(2.0);
		CP_Settings_Stroke(BLACK);
		CP_Graphics_DrawLine(mappedCoinVector.x + size / 2, mappedCoinVector.y + size / 2, ww / 2, wh / 2);

		double x = centerVector.x - mappedCoinVector.x;
		double y = centerVector.y - mappedCoinVector.y;
		double distance = sqrt(x * x + y * y);
		//distance = CP_Vector_Distance(centerVector, activeCoin);

		CP_Settings_TextSize(30.0f);
		sprintf_s(buffer, _countof(buffer), "%5.2f", distance);
		CP_Font_DrawText(buffer, centerVector.x + size / 2, centerVector.y + 120);

		sprintf_s(buffer, _countof(buffer), "Coin X: %5.2f", mappedCoinVector.x);
		CP_Font_DrawText(buffer, centerVector.x + size / 2, centerVector.y + 160);

		sprintf_s(buffer, _countof(buffer), "Coin Y: %5.2f", mappedCoinVector.y);
		CP_Font_DrawText(buffer, centerVector.x + size / 2, centerVector.y + 200);

		sprintf_s(buffer, _countof(buffer), "Plane X: %5.2f", centerVector.x);
		CP_Font_DrawText(buffer, centerVector.x + size / 2, centerVector.y + 240);

		sprintf_s(buffer, _countof(buffer), "Plane Y: %5.2f", centerVector.y);
		CP_Font_DrawText(buffer, centerVector.x + size / 2, centerVector.y + 280);

		if (distance < 75) {
			//Collect coin
			score += speed;
			coinAlpha = 0;
		}


		/**********************
		* DEBUG BOUNDARY SQUARE
		**********************/
		//CP_Settings_NoFill();
		//CP_Graphics_DrawRect(bounds.west + globalX, bounds.north + globalY, bounds.width, bounds.height);
		//CP_Settings_Fill(BLACK);

		/*************\
		| DRAW PLAYER |
		\*************/
		drawPlayer(BLACK);

		/*******************************************************\
		| CALCULATE VELOCITY, POSITION, ROTATION, AND DIRECTION |
		\*******************************************************/

		//x2 = cosAx1 − sinAy1
		//y2 = sinAx1 + cosAy1
		double newVX = cos(rotationAngle) * directionVector.x - sin(rotationAngle) * directionVector.y;
		double newVY = sin(rotationAngle) * directionVector.x + cos(rotationAngle) * directionVector.y;

		directionVector.x = newVX;
		directionVector.y = newVY;

		speed = (speed < 0) ? speed + drag : speed - drag;
		speed = (speed > speedCap) ? speedCap : (speed < speedMin) ? speedMin : speed;
		speed = (CP_Input_KeyDown(KEY_S)) ? 0.001 : speed;
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

		sprintf_s(buffer, _countof(buffer), "Global X position: %.0f", -globalX);
		CP_Font_DrawText(buffer, 200, 50);

		sprintf_s(buffer, _countof(buffer), "Global Y position: %.0f", globalY);
		CP_Font_DrawText(buffer, 200, 100);

		sprintf_s(buffer, _countof(buffer), "Speed: %.0f", speed);
		CP_Font_DrawText(buffer, 200, 150);

		sprintf_s(buffer, _countof(buffer), "Direction: %.0f", acos(directionVector.y) * 180 / PI);
		CP_Font_DrawText(buffer, 200, 200);

		sprintf_s(buffer, _countof(buffer), "Game Time: %.1f", CP_System_GetSeconds() - timeOfRestart);
		CP_Font_DrawText(buffer, 200, 250);

		sprintf_s(buffer, _countof(buffer), "Score: %d", score);
		CP_Font_DrawText(buffer, 200, 300);

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

		/**********\
		| FEEDBACK |
		\**********/
		//When the player gets hit by a cloud:
		// Flash the screen, time the iframes, increase turbulence, decrease speed, mark a "HIT"
		if (isIFraming) {
			//We just got hit! 
			CP_Image_Draw(redhitFlash, 0, 0, ww, wh, flashAlpha);
			flashAlpha -= 10;
			rotationAngle += CP_Random_RangeFloat(-1, 1) / 2;

			if (CP_System_GetSeconds() >= iFrameStart + iFrameDuration) {
				isIFraming = false;
				flashAlpha = 0;
			}
		}

		/************\
		| PAUSE MENU |
		\************/
		if (CP_Input_KeyReleased(KEY_ESCAPE)) {
			paused = true;
		}
	}
}

void game_exit(void) {
	
}


void death_init(void) {
	deathAlpha = 0;
	int timeOfDeath = CP_System_GetSeconds() - timeOfRestart;
	dminutes = timeOfDeath / 60;
	dseconds = timeOfDeath % 60;
	CP_Settings_TextAlignment(CP_TEXT_ALIGN_H_CENTER, CP_TEXT_ALIGN_V_MIDDLE);
}

void death_update(void) {
	BLACK.a = deathAlpha;
	CP_Settings_Fill(BLACK);
	CP_Graphics_DrawRect(0, 0, ww, wh);

	drawPlayer(CP_Color_Create(255, 255, 255, deathAlpha * 5));

	CP_Settings_Fill(BLUE);
	CP_Settings_TextSize(100.0f);

	CP_Font_DrawText("Game Over!", ww / 2, 150);

	CP_Settings_TextSize(70.0f);

	sprintf_s(buffer, _countof(buffer), "Score: %d", score);
	CP_Font_DrawText(buffer, ww / 2, 250);

	sprintf_s(buffer, _countof(buffer), "Gametime: %dm%ds", dminutes, dseconds);
	CP_Font_DrawText(buffer, ww / 2, 320);

	CP_Font_DrawText("Press R to restart.", ww / 2, wh - 250);

	CP_Font_DrawText("Press Q to quit.", ww / 2, wh - 150);

	deathAlpha += 1;

	if (CP_Input_KeyReleased(KEY_R)) {
		CP_Engine_SetNextGameState(game_init, game_update, game_exit);
	} else if (CP_Input_KeyReleased(KEY_Q)) {
		CP_Engine_Terminate();
	}
}

void death_exit(void) {
	timeOfRestart = CP_System_GetSeconds();
}



int main(void) {
	CP_Engine_SetNextGameState(game_init, game_update, game_exit);
	CP_Engine_Run();
	return 0;
}
