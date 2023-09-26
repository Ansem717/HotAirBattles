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

bool pauseMenuShowing, isIFraming, coinTriggered;
char buffer[50] = { 0 };
char guide[50] = { 0 };
char playText[15] = { 0 };

float globalX, globalY;

CP_Vector directionVector, centerVector;
float rotationAngle, rotationIncrement, rotationCap;

float speed, speedCap, speedMin, speedIncrement, drag, speedBonus;

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

CP_Vector activeCoin;
CP_Vector mappedCoinVector;
float coinYPos, coinVelocity, coinCap, coinAlpha, coinFadeSpeed;

float deathAlpha;
int dminutes, dseconds;
float timeOfRestart;


////////////////////////////
/// IMPORTED FROM FIRST ASSIGNMENT - LOGO SPLASH
////////////////////////////
CP_Color RED;
CP_Color DARK_RED;
CP_Color WHITE;
CP_Color GRAY;

CP_Image logo;
CP_Image logo_screenshot;

/***
* KEYFRAME LABELS
*
* Each of these are boolean constants that guide the update loop on which "animation" to run.
*
* I think the right way to do it would be to create a series of functions for new gamestates
* And run CP_Engine_NewGameState() to change the update function.
* I could use a PreUpdate function for update-persistance through each gamestate.
* HOWEVER, I already wrote all this code, so I'm just going to stick with this plan.
***/
int KEYFRAME_READY = 0;
int KEYFRAME_LOGO_COMBINE_DONE = 0;
int KEYFRAME_LOGO_SEISMIC_FALL_DONE = 0;
int KEYFRAME_TITLE_WIPE_DONE = 0;
int KEYFRAME_HOLD_DONE = 0;

// First Animation - the two parts of the logo crash into each other. 
float YIncrement = 0; //the current change in height for the first half drop
float YSpeed = 100; //the rate of change for the above increment

// Magic Number Helper
int smallLogoW = 251; //Just so I don't have to keep typing 251 everywhere.

// Second Animation - once collision hits, the logo rises and shakes (scale and rotation)
float intensity = 10; //how strong the shake is
float intensityMultiplier = -0.87f; // intensity *= intensityMultipler ;; the intensity will flip +/- and will dampen down to 0.
float altitude = 1; //height of logo in z-index (scale)
float velocity = 0.2f; //how fast the logo will reach the max.
float gravity = 0.03f; //Reducing the velocity every frame for a "damping" effect

// Third Animation - Quickly wipe the logo into frame.
float titleSpeed = 60;

// Fourth Animation - do nothing for 2 seconds
float startingSeconds = 0; //When "Hold" begins, this value will be set to some number. 
//Hold will end after this value increments by 2 seconds.

// Fifth Animation - Fade Out
float alpha = 255;
float alphaSpeed = 30;

// Once everything is cleared, wait a brief moment before the logo disappears. 
float finishingSeconds = 0;
/////////////////////////
/// END IMPORT FROM FIRST ASSIGNMENT
/////////////////////////

void logo_init(void);
void logo_update(void);
void logo_exit(void);
void game_init(void);
void game_update(void);
void game_exit(void);
void death_init(void);
void death_update(void);
void death_exit(void);
void pause_update(void);
void pause_exit(void);

void logo_init(void) {
	RED = CP_Color_Create(220, 17, 39, 255);
	DARK_RED = CP_Color_Create(133, 0, 34, 255);
	WHITE = CP_Color_Create(255, 255, 255, 255);
	GRAY = CP_Color_Create(209, 211, 213, 255);
	BLACK = CP_Color_Create(0, 0, 0, 255);
	BLUE = CP_Color_Create(32, 192, 255, 255);

	logo = CP_Image_Load("Assets/DigiPen_BLACK.png");

	sprintf_s(playText, _countof(playText), "PLAY!");

	CP_System_Fullscreen();
}

void logo_update(void) {
	CP_Graphics_ClearBackground(RED);
	int width = CP_System_GetWindowWidth();
	int height = CP_System_GetWindowHeight();
	int logoW = CP_Image_GetWidth(logo);
	int logoH = CP_Image_GetHeight(logo);

	if (KEYFRAME_READY) {
		//For this instance, I'm putting the condition first to manage the screenshot drawn. 
		// This way it wont screenshot the independent parts as well as the subimage.
		if (YIncrement >= height / 2 + logoH / 2) {
			//the "dP" is in the center, proceed to next keyframe
			KEYFRAME_READY = 0;
			KEYFRAME_LOGO_COMBINE_DONE = 1;

			//I need to do some background setup before going into the next animation
			//Notably, the DrawSubImage function has been and will be fantastic, but
			// I need to rotate the image, thus I need to use DrawAdvanced. 
			//To do this, I will DrawSubImage then ScreenShot and use that screenshot for the next animation.
			CP_Image_DrawSubImage(logo, width / 2, height / 2, smallLogoW, logoH, 0, 0, smallLogoW, logoH, 255);
			logo_screenshot = CP_Image_Screenshot(width / 2 - smallLogoW / 2, height / 2 - logoH / 2, smallLogoW, logoH);
		}

		//Cut the "dP" logo in half, and have them slide in from the top and bottom
		CP_Image_DrawSubImage(logo, width / 2 - 56, (-logoH / 2) + YIncrement, 138, logoH, 0, 0, 138, logoH, 255);
		CP_Image_DrawSubImage(logo, width / 2 + 56, height + logoH / 2 - YIncrement, 138, logoH, 113, 0, 251, logoH, 255);
		YIncrement += YSpeed;
	} else if (KEYFRAME_LOGO_COMBINE_DONE) {
		//So now that they collided, the logo will:
		// RISE in the z-index, violently shake, and then fall back to the ground.
		CP_Image_DrawAdvanced(logo_screenshot, width / 2, height / 2, smallLogoW * altitude, logoH * altitude, 255, intensity);

		altitude += velocity;
		if (altitude <= 1) altitude = 1;
		velocity -= gravity;
		intensity *= intensityMultiplier;

		if (altitude <= 1) {
			KEYFRAME_LOGO_COMBINE_DONE = 0;
			KEYFRAME_LOGO_SEISMIC_FALL_DONE = 1;
		}
	} else if (KEYFRAME_LOGO_SEISMIC_FALL_DONE) {
		//start with the previous image
		CP_Image_DrawSubImage(logo, width / 2, height / 2, smallLogoW + titleSpeed, logoH, 0, 0, smallLogoW + titleSpeed, logoH, 255);

		//Title Wipe is cool because the above subimage can simply extend to the full image over time.
		titleSpeed += titleSpeed;

		if (smallLogoW + titleSpeed >= logoW) {
			//the full logo is now displayed! Next keyframe!
			KEYFRAME_LOGO_SEISMIC_FALL_DONE = 0;
			KEYFRAME_TITLE_WIPE_DONE = 1;
		}
	} else if (KEYFRAME_TITLE_WIPE_DONE) {
		//hold full image
		CP_Image_Draw(logo, width / 2, height / 2, logoW, logoH, 255);
		startingSeconds = (!startingSeconds) ? CP_System_GetSeconds() : startingSeconds;

		if (CP_System_GetSeconds() >= startingSeconds + 2.2) {
			//We've waited 2.2 seconds, go to the next keyframe!
			KEYFRAME_TITLE_WIPE_DONE = 0;
			KEYFRAME_HOLD_DONE = 1;
		}
	} else if (KEYFRAME_HOLD_DONE) {
		//Same image but with a dynamic alpha value.
		CP_Graphics_ClearBackground(BLUE);
		CP_Settings_Fill(CP_Color_Create(220, 17, 39, alpha));
		CP_Graphics_DrawRect(0, 0, width, height);
		CP_Image_Draw(logo, width / 2, height / 2, logoW, logoH, alpha);

		alpha -= alphaSpeed;

		if (alpha <= 0) {
			KEYFRAME_HOLD_DONE = 0;
			CP_Graphics_ClearBackground(BLUE);

			finishingSeconds = (!finishingSeconds) ? CP_System_GetSeconds() : finishingSeconds;
			CP_Engine_SetNextGameState(game_init, pause_update, pause_exit);
		}
	} else {
		//When the program loads, nothing is ready, so we wait for 0.75 seconds and then start the animations.
		if (CP_System_GetSeconds() > 0.75) KEYFRAME_READY = 1;
	}

}

void logo_exit(void) {
	CP_Image_Free(&logo);
}

void initGlobalVariables(void) {
	pauseMenuShowing = false;
	isIFraming = false;

	activeClouds = malloc(CLOUD_ARR_SIZE * sizeof * activeClouds);

	globalX = 0;
	globalY = -bounds.height / 2 + 100;
	directionVector = CP_Vector_Set(0, 1);

	rotationAngle = 0;
	rotationIncrement = 0.03f; //the increment changes based on speed - the faster you are, the harder it is to turn.
	rotationCap = 0.06f;

	speed = 10;
	//speedCap = 20;
	speedMin = 5; //The plane is always moving!
	speedIncrement = 0.25f;
	drag = 0.1f;
	speedBonus = 3;

	iFrameDuration = 1;
	iFrameStart = 0;
	flashAlpha = 0;

	remainingLives = 3;
	score = 0;

	coinVelocity = 1;
	coinYPos = 0;
	coinCap = 10;
	coinAlpha = 255;
	coinFadeSpeed = 1;
	coinTriggered = false;

	sprintf_s(guide, _countof(guide), "Collect the coin for points!");
	
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
	bodyAngle = (directionVector.x <= 0) ? bodyAngle : -bodyAngle;

	
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
	coinAlpha = 255;
	coinTriggered = false;
}

/* * * * * * 
* DRAW COIN *
 * * * * * */
void drawCoin(float initialX, float initialY, float size) {
	mappedCoinVector.x = initialX + globalX + size / 2;
	mappedCoinVector.y = initialY + globalY + coinYPos + size / 2;
	CP_Image_Draw(coinIMG, mappedCoinVector.x, mappedCoinVector.y, size, size, coinAlpha);
	coinYPos += coinVelocity;
	if (coinYPos > coinCap || coinYPos < -coinCap) coinVelocity *= -1;

	double x = centerVector.x - mappedCoinVector.x - size / 2;
	double y = centerVector.y - mappedCoinVector.y - size / 2;
	
	double distance = sqrt(x * x + y * y);

	//Lock the X and Y value of the triangle's tip based to the edge of the screen if the coin is off screen... with some padding.
	float hPadding = 100;
	float vPadding = 70;
	float triangleX = (mappedCoinVector.x + size / 2 > ww - hPadding) ? ww - hPadding : (mappedCoinVector.x + size / 2 < hPadding) ? hPadding : mappedCoinVector.x + size / 2;
	float triangleY = (mappedCoinVector.y + size / 2 > wh - vPadding) ? wh - vPadding : (mappedCoinVector.y + size / 2 < vPadding) ? vPadding : mappedCoinVector.y + size / 2;
	float triangleW = 60;
	float triangleH = 50;

	CP_Vector tv = CP_Vector_Normalize(CP_Vector_Subtract(centerVector, mappedCoinVector));

	float triangleR = acos(tv.y) * 180 / PI;
	triangleR = (tv.x <= 0) ? triangleR : -triangleR;

	CP_Settings_Fill(CP_Color_Create(220, 220, 100, 255));
	CP_Settings_Stroke(BLACK);
	CP_Settings_StrokeWeight(2.0);

	if (!coinTriggered)
		if (mappedCoinVector.x + size / 2 > ww || mappedCoinVector.x + size / 2 < 0 || mappedCoinVector.y + size / 2 > wh || mappedCoinVector.y + size / 2 < 0)
			CP_Graphics_DrawTriangleAdvanced(triangleX, triangleY, triangleX - triangleW / 2, triangleY + triangleH, triangleX + triangleW / 2, triangleY + triangleH, triangleR);

	if (distance < 75 && !coinTriggered) {
		//Collect coin
		score += speed;
		speed += speedBonus;
		coinAlpha = 0;
		sprintf_s(guide, _countof(guide), "Explore for a new coin!");
		coinTriggered = true;
	}

	CP_Settings_Fill(BLACK);
}

void game_init(void) {
	CP_System_Fullscreen();
	cloudTexture = CP_Image_Load("Assets/cloudtextures.png");
	redhitFlash = CP_Image_Load("Assets/redhit.png");
	coinIMG = CP_Image_Load("Assets/coin.png");
	
	ww = CP_System_GetWindowWidth();
	wh = CP_System_GetWindowHeight();

	initBounds();
	initGlobalVariables();

	createClouds();
	createCoin();

	CP_Settings_Fill(BLACK);
	CP_Settings_TextAlignment(CP_TEXT_ALIGN_H_CENTER, CP_TEXT_ALIGN_V_MIDDLE);
	CP_Settings_ImageMode(CP_POSITION_CORNER);
	CP_System_ShowCursor(false);
}

void game_update(void) {
	
	// DRAW BACKGROUND (Sky)
	CP_Graphics_ClearBackground(BLUE);
	CP_Settings_Fill(BLACK);

	/*************\
	| DRAW PLAYER |
	\*************/
	drawPlayer(BLACK);

	/*************\
	| DRAW CLOUDS |
	\*************/
	for (int i = 0; i < CLOUD_ARR_SIZE; i++) {
		Cloud currentCloud = activeClouds[i];
		TextureRect currentTexture = CLOUD_TEXTURE_POSITIONS[currentCloud.img_id];
		CP_Image_DrawSubImage(cloudTexture, currentCloud.x + globalX, currentCloud.y + globalY, currentCloud.size * currentTexture.w, currentCloud.size * currentTexture.h, currentTexture.x0, currentTexture.y0, currentTexture.x1, currentTexture.y1, 255);
			
		float widthScalar = 0.8f;
		float heightScalar = 0.7f;
			
		CP_Vector currentCloudVector = CP_Vector_Set(currentCloud.x + globalX + currentTexture.w / 2, currentCloud.y + globalY + currentTexture.h / 2);
			
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
			speed *= 2;
			isIFraming = true;
			flashAlpha = 255;
			iFrameStart = CP_System_GetSeconds();
		}
	}

	/***********\
	| DRAW COIN |
	\***********/
	drawCoin(activeCoin.x, activeCoin.y, 80);

	/*******************************************************\
	| CALCULATE VELOCITY, POSITION, ROTATION, AND DIRECTION |
	\*******************************************************/

	//x2 = cosAx1 − sinAy1
	//y2 = sinAx1 + cosAy1
	double newVX = cos(rotationAngle) * directionVector.x - sin(rotationAngle) * directionVector.y;
	double newVY = sin(rotationAngle) * directionVector.x + cos(rotationAngle) * directionVector.y;

	directionVector.x = newVX;
	directionVector.y = newVY;

	directionVector = CP_Vector_Normalize(directionVector);

	rotationAngle = (rotationAngle > rotationCap) ? rotationCap : (rotationAngle < -rotationCap) ? -rotationCap : rotationAngle;

	/*
	The statements above are known as Ternary Operators:

	VARIABLE = (CONDITION) ? [true value] : [false value];

	We can set the value of a variable based on a condition all in one line.

	Looking at the line: 
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

	globalX += directionVector.x * speed;
	globalY += directionVector.y * speed;

	if (globalX > bounds.width/2 || globalX < -bounds.width/2) {
		//if we're out of bounds, teleport to the opposite boundary.
		globalX *= -1;
		createClouds(); //no clouds are visible, great time to randomize them!
		createCoin();
		sprintf_s(guide, _countof(guide), "");
	}

	if (globalY > bounds.height / 2 || globalY < -bounds.height / 2) {
		globalY *= -1;
		createClouds();
		createCoin();
		sprintf_s(guide, _countof(guide), "");
	}

	/***********\
	| DRAW TEXT |
	\***********/
	CP_Settings_TextSize(40.0f);

	sprintf_s(buffer, _countof(buffer), "Speed: %.0f", speed);
	CP_Font_DrawText(buffer, 200, 250);

	sprintf_s(buffer, _countof(buffer), "Direction: %.0f", acos(directionVector.y) * 180 / PI);
	CP_Font_DrawText(buffer, 200, 200);

	sprintf_s(buffer, _countof(buffer), "Game Time: %.1f", CP_System_GetSeconds() - timeOfRestart);
	CP_Font_DrawText(buffer, 200, 50);

	sprintf_s(buffer, _countof(buffer), "Score: %d", score);
	CP_Font_DrawText(buffer, 200, 150);

	sprintf_s(buffer, _countof(buffer), "Lives: %d", remainingLives);
	CP_Font_DrawText(buffer, 200, 100);

	CP_Settings_TextSize(60.0f);
	CP_Font_DrawText(guide, ww / 2, 100);

	/*********\
	| CONTROL |
	\*********/
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
			speed /= 4;
			flashAlpha = 0;
		}
	}

	/************\
	| PAUSE MENU |
	\************/
	if (CP_Input_KeyReleased(KEY_ESCAPE)) {
		CP_Engine_SetNextGameState(NULL, pause_update, pause_exit);
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

void pause_update(void) {
	if (pauseMenuShowing) {
		//close pause menu
		if (CP_Input_KeyReleased(KEY_ESCAPE)) {
			CP_Engine_SetNextGameState(NULL, game_update, game_exit);
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

		CP_Settings_TextSize(100.0f);
		CP_Font_DrawText("MENU", ww / 2, wh * 5 / 16);

		CP_Settings_TextSize(50.0f);
		CP_Font_DrawText(playText, ww / 2, wh * 7 / 16);
		CP_Font_DrawText("QUIT", ww / 2, wh * 17 / 32);
		CP_Font_DrawText("RESET", ww / 2, wh * 20 / 32);

		pauseMenuShowing = true;
	}
}

void pause_exit(void) {
	pauseMenuShowing = false;
	sprintf_s(playText, _countof(playText), "CONTINUE");
}


int main(void) {
	CP_Engine_SetNextGameState(logo_init, logo_update, logo_exit);
	CP_Engine_Run();
	return 0;
}
