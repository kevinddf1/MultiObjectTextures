//3D Gaming
//Fan Ding, 2021

//This program was based on Stephen J. Guy's demo which demonstrates:
// Loading multiple models (a cube and a knot)
// Using multiple textures (wood and brick)
// Instancing (the teapot is drawn in two locations)
// Continuous keyboard input - arrows (moves knot up/down/left/right continuous when being held)
// Keyboard modifiers - shift (up/down arrows move knot in/out of screen when shift is pressed)
// Single key events - pressing 'c' changes color of a random teapot
// Mixing textures and colors for models
// Phong lighting
// Binding multiple textures to one shader

const char* INSTRUCTIONS =
"***************\n"
"This demo shows multiple objects being draw at once along with user interaction.\n"
"\n"
"Up/down/left/right - Moves the knot.\n"
"c - Changes to teapot to a random color.\n"
"***************\n"
;

//Mac OS build: g++ multiObjectTest.cpp -x c glad/glad.c -g -F/Library/Frameworks -framework SDL2 -framework OpenGL -o MultiObjTest
//Linux build:  g++ multiObjectTest.cpp -x c glad/glad.c -g -lSDL2 -lSDL2main -lGL -ldl -I/usr/include/SDL2/ -o MultiObjTest

#include "glad/glad.h"  //Include order can matter here
#if defined(__APPLE__) || defined(__linux__)
#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>
#else
#include <SDL.h>
#include <SDL_opengl.h>
#endif
#include <cstdio>

#define GLM_FORCE_RADIANS
#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>

#include <cstdio>
#include <iostream>
#include <fstream>
#include <string>
#include <map>
#include <list>
#include <cmath>
#include <map>




using namespace std;

int screenWidth = 800;
int screenHeight = 600;
float timePast = 0;


//Character Atributes
float StartX = 0;
float StartY = 0;
float StartZ = 0;

bool topView = false;

float angleSpeed = 0.01;
float linSpeed = 0.01;
float playerRadius = .05;
float pickupRadius = .01;
float w = 0;


bool win = false;


//What map symbols corrispond to doors
map<char, bool> isDoor;

map<char, bool> DoorOpen;


//What map symbols corrispond to keys
map<char, bool> isKey;

map<char, bool> playOwnItem;




//Initalize player inventory to be emtpy
std::list<string> hasItem = {  };  //Player inventory


//map variables
int mapW = 0;
int mapH = 0;
int totalObject = 0;
char* mapArray;

//some variables for the model
//int totalNumModel = 0;
int totalNumVerts = 0;
int startVertTeapot, startVertKnot, startVertCube, startVertSphere = 0;
int numVertsTeapot, numVertsKnot, numVertsCube, numVertsSphere = 0;


//Store the object coordinates
//You should have a representation for the state of each object
float objx = 0, objy = 0, objz = 0;
float goDirx = 0, goDiry = 1*angleSpeed, goDirz = 0;
float colR = 1, colG = 1, colB = 1;

float camPosx = 3, camPosy = 0.4, camPosz = -0.4;
float camDirx = 0, camDiry = 1, camDirz = -0.8;
float upx = 1, upy = 0, upz = 0;


bool DEBUG_ON = true;
GLuint InitShader(const char* vShaderFileName, const char* fShaderFileName);
bool fullscreen = false;
void Win2PPM(int width, int height);

//srand(time(NULL));
float rand01() {
	return rand() / (float)RAND_MAX;
}

void drawMap(int shaderProgram);
void drawObject(int shaderProgram, GLint uniColorID, GLint uniTexID, char c, float Tx, float Ty, float Tz);
bool isWalkable(float newY, float newZ);
void checkForEvents(float currentY, float currentZ);
void updateDir();
void gameover();


//--Check if a given position would be okay for the player to move to(ie, not colliding
//	--  with any walls or doors).We don't just check if the player's center is colliding,
//	--but also check a radius around the agent.
//	--TODO: We end up treating the agent as a square more than a circle...
//	--        ...this casues us problems with corners. = /
bool isWalkable(float newY, float newZ) {
	//printf("Y and Z value: %f, %f\n", newY, newZ);
	for (int dz = -1; dz <= 1; dz += 2) {
		for (int dy = -1; dy <= 1; dy += 2) {
			float i = (newZ + playerRadius * dz)*5;
			float j = (newY + playerRadius * dy)*5;
			int index = round(-i)*mapW + round(j);
			//printf("the index is %d \n", index);
			char element = mapArray[index];
			if (element == 'W' ||  round(-i) <0 || round(-i) >= mapH || round(j)<0||round(j)>=mapW) {
				return false;
			}
			if (isDoor[element] && !playOwnItem[tolower(element)]) {
				return false;
			}
			if (isDoor[element] && playOwnItem[tolower(element)]) {
				DoorOpen[element] = true;
				//playOwnItem[tolower(element)] = false;
				return true;
			}
		}
	}
	//printf("result is yes, u can walk\n");
	return true;
}


//--This is called each time the player moves.Here, we need to see if they pick up any
//--keys or unlock any doors.
//function checkForEvents()
//--We know the player can only interact with objects in one of the 9 neighboring cells
//--  so we only check the 8 extreme points of the agent(plus the center).
//--Note: this assumes pickupRadius < cell size
void checkForEvents(float currentY, float currentZ) {

	for (int dz = -1; dz <= 1; dz += 2) {
		for (int dy = -1; dy <= 1; dy += 2) {
			float i = (currentZ + pickupRadius * dz) * 5;
			float j = (currentY + pickupRadius * dy) * 5;
			int index = round(-i) * mapW + round(j);
			//printf("the index is %d \n", index);
			if (round(-i) < 0 || round(-i) >= mapH || round(j) < 0 || round(j) >= mapW) {
				//out bound, ingore
			}
			else {
				char element = mapArray[index];
				if (element == 'G') {
					playOwnItem['G'] = true;
					printf("game over!");
					gameover();
				}
				if (isKey[element]) {
					playOwnItem[element] = true;
				}
			}
		}
	}
	
}


void updateDir() {
	//	//update the camDir by the goDir.
	goDiry = cos(w) * angleSpeed;
	goDirz = sin(w) * angleSpeed;

	camDirx = camPosx + goDirx;
	camDiry = camPosy + goDiry;
	camDirz = camPosz + goDirz;

	printf("camera postion:\t\t %f,%f,%f\n", camPosx, camPosy, camPosz);
	printf("camera goDir:\t\t %f,%f,%f\n", goDirx, goDiry, goDirz);
	printf("camera Dir:\t\t %f,%f,%f\n", camDirx, camDiry, camDirz);
}

void gameover() {
	win = true;
	topView = true;
	
}



int main(int argc, char* argv[]) {
	isDoor['A'] = true;
	isDoor['B'] = true;
	isDoor['C'] = true;
	isDoor['D'] = true;
	isDoor['E'] = true;

	DoorOpen['A'] = false;
	DoorOpen['B'] = false;
	DoorOpen['C'] = false;
	DoorOpen['D'] = false;
	DoorOpen['E'] = false;


	isKey['a'] = true;
	isKey['b'] = true;
	isKey['c'] = true;
	isKey['d'] = true;
	isKey['e'] = true;

	playOwnItem['a'] = false;
	playOwnItem['b'] = false;
	playOwnItem['c'] = false;
	playOwnItem['d'] = false;
	playOwnItem['e'] = false;
	playOwnItem['G'] = false;


	

	SDL_Init(SDL_INIT_VIDEO);  //Initialize Graphics (for OpenGL)

	//Ask SDL to get a recent version of OpenGL (3.2 or greater)
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);

	//Create a window (offsetx, offsety, width, height, flags)
	SDL_Window* window = SDL_CreateWindow("My OpenGL Program", 100, 100, screenWidth, screenHeight, SDL_WINDOW_OPENGL);

	//Create a context to draw in
	SDL_GLContext context = SDL_GL_CreateContext(window);

	//Load OpenGL extentions with GLAD
	if (gladLoadGLLoader(SDL_GL_GetProcAddress)) {
		printf("\nOpenGL loaded\n");
		printf("Vendor:   %s\n", glGetString(GL_VENDOR));
		printf("Renderer: %s\n", glGetString(GL_RENDERER));
		printf("Version:  %s\n\n", glGetString(GL_VERSION));
	}
	else {
		printf("ERROR: Failed to initialize OpenGL context.\n");
		return -1;
	}

	//Here we will load 4 different model files 


	
	ifstream modelFile;
	int numLines = 0;
	
	//Load Model 1
	modelFile.open("C:/Users/kevin/Desktop/OpenGLStarterCode/models/teapot.txt");
	numLines = 0;
	modelFile >> numLines;
	float* model1 = new float[numLines];
	for (int i = 0; i < numLines; i++) {
		modelFile >> model1[i];
	}
	printf("%d\n", numLines);
	numVertsTeapot = numLines / 8;
	modelFile.close();
	totalNumVerts += numVertsTeapot;

	//Load Model 2
	modelFile.open("C:/Users/kevin/Desktop/OpenGLStarterCode/models/knot.txt");
	numLines = 0;
	modelFile >> numLines;
	float* model2 = new float[numLines];
	for (int i = 0; i < numLines; i++) {
		modelFile >> model2[i];
	}
	printf("%d\n", numLines);
	numVertsKnot = numLines / 8;
	modelFile.close();
	totalNumVerts += numVertsKnot;


	//Load Model 3
	modelFile.open("C:/Users/kevin/Desktop/OpenGLStarterCode/models/cube.txt");
	numLines = 0;
	modelFile >> numLines;
	float* model3 = new float[numLines];
	for (int i = 0; i < numLines; i++) {
		modelFile >> model3[i];
	}
	printf("%d\n", numLines);
	numVertsCube = numLines / 8;
	modelFile.close();
	totalNumVerts += numVertsCube;

	//Load Model 4
	modelFile.open("C:/Users/kevin/Desktop/OpenGLStarterCode/models/sphere.txt");
	numLines = 0;
	modelFile >> numLines;
	float* model4 = new float[numLines];
	for (int i = 0; i < numLines; i++) {
		modelFile >> model4[i];
	}
	printf("%d\n", numLines);
	numVertsSphere = numLines / 8;
	modelFile.close();
	totalNumVerts += numVertsSphere;


	
	totalNumVerts = numVertsTeapot + numVertsKnot + numVertsCube + numVertsSphere;
	//SJG: I load each model in a different array, then concatenate everything in one big array
	// This structure works, but there is room for improvement here. Eg., you should store the start
	// and end of each model a data structure or array somewhere.
	//Concatenate model arrays
	float* modelData = new float[totalNumVerts * 8];
	copy(model1, model1 + numVertsTeapot * 8, modelData);
	copy(model2, model2 + numVertsKnot * 8, modelData + numVertsTeapot * 8);
	copy(model3, model3 + numVertsCube * 8, modelData + numVertsTeapot * 8 + numVertsKnot * 8);
	copy(model4, model4 + numVertsSphere * 8, modelData + numVertsTeapot * 8 + numVertsKnot * 8 + numVertsCube * 8);


	
	startVertTeapot = 0;  //The teapot is the first model in the VBO
	startVertKnot = numVertsTeapot; //The knot starts right after the teapot
	startVertCube = numVertsTeapot + numVertsKnot;
	startVertSphere = numVertsTeapot + numVertsKnot + numVertsCube;





	//// Allocate Texture 0 (Wood) ///////
	SDL_Surface* surface = SDL_LoadBMP("C:/Users/kevin/Desktop/MultiObjectTextures/door5.bmp");
	if (surface == NULL) { //If it failed, print the error
		printf("Error: \"%s\"\n", SDL_GetError()); return 1;
	}
	GLuint tex0;
	glGenTextures(1, &tex0);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, tex0);

	//What to do outside 0-1 range
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	//Load the texture into memory
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, surface->w, surface->h, 0, GL_BGR, GL_UNSIGNED_BYTE, surface->pixels);
	glGenerateMipmap(GL_TEXTURE_2D); //Mip maps the texture

	SDL_FreeSurface(surface);
	//// End Allocate Texture ///////


	//// Allocate Texture 1 (Brick) ///////
	SDL_Surface* surface1 = SDL_LoadBMP("C:/Users/kevin/Desktop/MultiObjectTextures/brick.bmp");
	if (surface1 == NULL) { //If it failed, print the error
		printf("Error: \"%s\"\n", SDL_GetError()); return 1;
	}
	GLuint tex1;
	glGenTextures(1, &tex1);

	//Load the texture into memory
	glActiveTexture(GL_TEXTURE1);

	glBindTexture(GL_TEXTURE_2D, tex1);
	//What to do outside 0-1 range
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	//How to filter
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, surface1->w, surface1->h, 0, GL_BGR, GL_UNSIGNED_BYTE, surface1->pixels);
	glGenerateMipmap(GL_TEXTURE_2D); //Mip maps the texture

	SDL_FreeSurface(surface1);
	//// End Allocate Texture ///////

		//// Allocate Texture 2 (stone) ///////
	SDL_Surface* surface2 = SDL_LoadBMP("C:/Users/kevin/Desktop/MultiObjectTextures/unnamed.bmp");
	if (surface2 == NULL) { //If it failed, print the error
		printf("Error: \"%s\"\n", SDL_GetError()); return 1;
	}
	GLuint tex2;
	glGenTextures(1, &tex2);

	//Load the texture into memory
	glActiveTexture(GL_TEXTURE2);

	glBindTexture(GL_TEXTURE_2D, tex2);
	//What to do outside 0-1 range
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	//How to filter
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, surface2->w, surface2->h, 0, GL_BGR, GL_UNSIGNED_BYTE, surface2->pixels);
	glGenerateMipmap(GL_TEXTURE_2D); //Mip maps the texture

	SDL_FreeSurface(surface2);
	//// End Allocate Texture ///////

			//// Allocate Texture 3 (win) ///////
	SDL_Surface* surface3 = SDL_LoadBMP("C:/Users/kevin/Desktop/MultiObjectTextures/win.bmp");
	if (surface3 == NULL) { //If it failed, print the error
		printf("Error: \"%s\"\n", SDL_GetError()); return 1;
	}
	GLuint tex3;
	glGenTextures(1, &tex3);

	//Load the texture into memory
	glActiveTexture(GL_TEXTURE3);

	glBindTexture(GL_TEXTURE_2D, tex3);
	//What to do outside 0-1 range
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	//How to filter
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, surface3->w, surface3->h, 0, GL_BGR, GL_UNSIGNED_BYTE, surface3->pixels);
	glGenerateMipmap(GL_TEXTURE_2D); //Mip maps the texture

	SDL_FreeSurface(surface3);
	//// End Allocate Texture ///////




	//// VAO ///////
	//Build a Vertex Array Object (VAO) to store mapping of shader attributse to VBO
	GLuint vao;
	glGenVertexArrays(1, &vao); //Create a VAO
	glBindVertexArray(vao); //Bind the above created VAO to the current context


	/// VBO ///////
	//Allocate memory on the graphics card to store geometry (vertex buffer object)
	GLuint vbo[1];
	glGenBuffers(1, vbo);  //Create 1 buffer called vbo
	glBindBuffer(GL_ARRAY_BUFFER, vbo[0]); //Set the vbo as the active array buffer (Only one buffer can be active at a time)
	glBufferData(GL_ARRAY_BUFFER, totalNumVerts * 8 * sizeof(float), modelData, GL_STATIC_DRAW); //upload vertices to vbo
	//GL_STATIC_DRAW means we won't change the geometry, GL_DYNAMIC_DRAW = geometry changes infrequently
	//GL_STREAM_DRAW = geom. changes frequently.  This effects which types of GPU memory is used



	/// SHADER /////////
	int texturedShader = InitShader("C:/Users/kevin/Desktop/MultiObjectTextures/textured-Vertex.glsl", "C:/Users/kevin/Desktop/MultiObjectTextures/textured-Fragment.glsl");

	//Tell OpenGL how to set fragment shader input 
	GLint posAttrib = glGetAttribLocation(texturedShader, "position");
	glVertexAttribPointer(posAttrib, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), 0);
	//Attribute, vals/attrib., type, isNormalized, stride, offset
	glEnableVertexAttribArray(posAttrib);

	

	GLint normAttrib = glGetAttribLocation(texturedShader, "inNormal");
	glVertexAttribPointer(normAttrib, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(5 * sizeof(float)));
	glEnableVertexAttribArray(normAttrib);

	GLint texAttrib = glGetAttribLocation(texturedShader, "inTexcoord");
	glEnableVertexAttribArray(texAttrib);
	glVertexAttribPointer(texAttrib, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));

	GLint uniView = glGetUniformLocation(texturedShader, "view");
	GLint uniProj = glGetUniformLocation(texturedShader, "proj");

	glBindVertexArray(0); //Unbind the VAO in case we want to create a new one	


	glEnable(GL_DEPTH_TEST);

	printf("%s\n", INSTRUCTIONS);







	
	//************
	// before even loop, we can start load our map
	//*************


	ifstream mapFile;

	mapFile.open("C:/Users/kevin/Desktop/OpenGLStarterCode/models/map.txt");
	mapFile >> mapW;
	mapFile >> mapH;
	printf("%d\n",mapW);
	printf("%d\n", mapH);

	totalObject = mapW * mapH;
	

	mapArray = new char[totalObject];
	for (int i = 0; i < totalObject; i++) {
		mapFile >> mapArray[i];
		//set up the camera postion and direction
		if (mapArray[i] == 'S') {
			//calulate the camera postion
			camPosx = 0;
			camPosy = ((i) % 5) * 0.2;
			camPosz = ((i) / 5) * -0.2;
			printf("initial camera postion: \t %f, %f, %f \n", camPosx, camPosy, camPosz);
			printf("initial go Dir: \t\t %f, %f, %f \n", goDirx , goDiry , goDirz );

			camDirx = camPosx+goDirx ;
			camDiry = camPosy+goDiry ;
			camDirz = camPosz+goDirz ;
			printf("initial camera direction: \t %f, %f, %f \n", camDirx, camDiry, camDirz);

		}

		
	}
	//printf(mapArray);
	
	mapFile.close();









	//Event Loop (Loop forever processing each event as fast as possible)
	SDL_Event windowEvent;
	bool quit = false;
	while (!quit ) {
		while (SDL_PollEvent(&windowEvent) ) {  //inspect all events in the queue
			if (windowEvent.type == SDL_QUIT) quit = true;
			//List of keycodes: https://wiki.libsdl.org/SDL_Keycode - You can catch many special keys
			//Scancode referes to a keyboard position, keycode referes to the letter (e.g., EU keyboards)
			if (windowEvent.type == SDL_KEYUP && windowEvent.key.keysym.sym == SDLK_ESCAPE)
				quit = true; //Exit event loop
			if (windowEvent.type == SDL_KEYUP && windowEvent.key.keysym.sym == SDLK_f) { //If "f" is pressed
				fullscreen = !fullscreen;
				SDL_SetWindowFullscreen(window, fullscreen ? SDL_WINDOW_FULLSCREEN : 0); //Toggle fullscreen 
			}

			//Fan
			if (  windowEvent.type == SDL_KEYUP && windowEvent.key.keysym.sym == SDLK_h) { //If "v" is pressed, top view mode
				topView = !topView;
				printf("top view is %s\n", topView ? "true" : "false");
			
				
			}

			//SJG: Use key input to change the state of the object
			//     We can use the ".mod" flag to see if modifiers such as shift are pressed
			if (!win && windowEvent.type == SDL_KEYDOWN && windowEvent.key.keysym.sym == SDLK_UP) { //If "up key" is pressed
				//if (windowEvent.key.keysym.mod & KMOD_SHIFT) objx -= .1; //Is shift pressed?
				float newobjY = objy +  goDiry;
				float newobjZ = objz +  goDirz;
				if (isWalkable(StartY + newobjY, StartZ + newobjZ)) {
					objy = newobjY;
					objz = newobjZ;
					camPosy = StartY + newobjY;
					camPosz = StartZ + newobjZ;
					updateDir();
					checkForEvents(camPosy, camPosz);
					
				}
				/*camPosx += camDirx * 0.1;
				camPosy += camDiry * 0.1;
				camPosz += camDirz * 0.1;*/
			}
			if (!win && windowEvent.type == SDL_KEYDOWN && windowEvent.key.keysym.sym == SDLK_DOWN) { //If "down key" is pressed
				float newobjY = objy - goDiry;
				float newobjZ = objz - goDirz;
				if (isWalkable(StartY + newobjY, StartZ + newobjZ)) {
					objy = newobjY;
					objz = newobjZ;
					camPosy = StartY + newobjY;
					camPosz = StartZ + newobjZ;
					updateDir();
					checkForEvents(camPosy, camPosz);
					
				}
				//if (windowEvent.key.keysym.mod & KMOD_SHIFT) objx += .1; //Is shift pressed?
				/*camPosx -= camDirx * 0.1;
				camPosy -= camDiry * 0.1;
				camPosz -= camDirz * 0.1;*/
			}
			if (!win && windowEvent.type == SDL_KEYDOWN && windowEvent.key.keysym.sym == SDLK_LEFT) { //If "left key" is pressed
				if (windowEvent.key.keysym.mod & KMOD_SHIFT) {
					float newobjY = objy - goDirz;
					float newobjZ = objz + goDiry;
					if (isWalkable(StartY + newobjY, StartZ + newobjZ)) {
						objy = newobjY;
						objz = newobjZ;
						camPosy = StartY + newobjY;
						camPosz = StartZ + newobjZ;
						updateDir();
						checkForEvents(camPosy, camPosz);
						
					}
				}
				else {
					w += 000.1; //w is how many degree we rotate from counterclockwise 
					updateDir();
					
				}
			}
			if (!win && windowEvent.type == SDL_KEYDOWN && windowEvent.key.keysym.sym == SDLK_RIGHT) { //If "right key" is pressed
				if (windowEvent.key.keysym.mod & KMOD_SHIFT) {
					float newobjY = objy + goDirz;
					float newobjZ = objz - goDiry;
					if (isWalkable(StartY + newobjY, StartZ + newobjZ)) {
						objy = newobjY;
						objz = newobjZ;
						camPosy = StartY + newobjY;
						camPosz = StartZ + newobjZ;
						updateDir();
						checkForEvents(camPosy, camPosz);
						
					}
				}
				else {
					w -= 000.1;
					updateDir();
				}
			}

		




			if (windowEvent.type == SDL_KEYUP && windowEvent.key.keysym.sym == SDLK_r) { //If "c" is pressed
				colR = rand01();
				colG = rand01();
				colB = rand01();
			}

		

		}


	


		// Clear the screen to default color
		glClearColor(.2f, 0.4f, 0.8f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glUseProgram(texturedShader);


		timePast = SDL_GetTicks() / 1000.f;

		if (topView) {
			camPosx = 2, camPosy = 0.4, camPosz = -0.4;
			camDirx = 0, camDiry = 0.4, camDirz = -0.4;
			upx = 0, upy = 0, upz = 1;
		}
		else {
			camPosx = 0;
			camPosy = StartY + objy;
			camPosz = StartZ + objz;
			camDirx = 0;
			camDiry = camPosy + goDiry;
			camDirz = camPosz + goDirz;
			upx = 1, upy = 0, upz = 0;
		}

		glm::mat4 view = glm::lookAt(
			glm::vec3(camPosx, camPosy, camPosz),  //Cam Position
			glm::vec3(camDirx, camDiry, camDirz),  //Look at point direction
			glm::vec3(upx, upy, upz)); //Up
		glUniformMatrix4fv(uniView, 1, GL_FALSE, glm::value_ptr(view));

		glm::mat4 proj = glm::perspective(3.14f / 2, screenWidth / (float)screenHeight, 0.001f, 30.0f); //FOV, aspect, near, far
		glUniformMatrix4fv(uniProj, 1, GL_FALSE, glm::value_ptr(proj));


		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, tex0);
		glUniform1i(glGetUniformLocation(texturedShader, "tex0"), 0);

		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, tex1);
		glUniform1i(glGetUniformLocation(texturedShader, "tex1"), 1);

		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, tex2);
		glUniform1i(glGetUniformLocation(texturedShader, "tex2"), 2);

		glActiveTexture(GL_TEXTURE3);
		glBindTexture(GL_TEXTURE_2D, tex3);
		glUniform1i(glGetUniformLocation(texturedShader, "tex3"), 3);

		glBindVertexArray(vao);
		drawMap(texturedShader);

		SDL_GL_SwapWindow(window); //Double buffering
	}

	//Clean Up
	glDeleteProgram(texturedShader);
	glDeleteBuffers(1, vbo);
	glDeleteVertexArrays(1, &vao);

	SDL_GL_DeleteContext(context);
	SDL_Quit();
	return 0;
}

void drawObject(int shaderProgram, GLint uniColorID, GLint uniTexID, char c, float Tx, float Ty, float Tz ) {
	glm::mat4 model = glm::mat4(1);
	GLint uniModel = glGetUniformLocation(shaderProgram, "model");

	//goal
	if (c == 'G' && !playOwnItem[c]) { //stands for goal, spining teapot
		//Rotate model (matrix) based on how much time has past
		model = glm::mat4(1);
		model = glm::translate(model, glm::vec3(Tx, Ty, Tz));
		model = glm::rotate(model, timePast * 3.14f / 2, glm::vec3(0.0f, 1.0f, 1.0f));
		model = glm::rotate(model, timePast * 3.14f / 4, glm::vec3(1.0f, 0.0f, 0.0f));
		model = glm::scale(model,glm::vec3(.1f,.1f,.1f)); //An example of scale
		uniModel = glGetUniformLocation(shaderProgram, "model");
		glUniformMatrix4fv(uniModel, 1, GL_FALSE, glm::value_ptr(model)); //pass model matrix to shader

		//Set which texture to use (-1 = no texture)
		glUniform1i(uniColorID, -1);
		glUniform1i(uniTexID, -1);

		//Draw an instance of the model (at the position & orientation specified by the model matrix above)
		glDrawArrays(GL_TRIANGLES, startVertTeapot, numVertsTeapot); //(Primitive Type, Start Vertex, Num Verticies)
	}


	//start point
	if (c == 'S' && topView) {
		model = glm::mat4(1);
		model = glm::translate(model, glm::vec3(Tx, Ty, Tz));
		//model = glm::rotate(model,  w, glm::vec3(camPosx, camPosy, camPosz));
		model = glm::scale(model, glm::vec3(.1f, .1f, .1f)); //scale this model
		//Translate the model (matrix) based on where objx/y/z is
		// ... these variables are set when the user presses the arrow keys
		model = glm::translate(model, glm::vec3(objx*10, objy*10, objz*10));

		//Set which texture to use (1 = brick texture ... bound to GL_TEXTURE1)
		glUniform1i(uniColorID, -1);
		glUniform1i(uniTexID, 1);
		glUniformMatrix4fv(uniModel, 1, GL_FALSE, glm::value_ptr(model));

		//Draw an instance of the model (at the position & orientation specified by the model matrix above)
		glDrawArrays(GL_TRIANGLES, startVertKnot, numVertsKnot); //(Primitive Type, Start Vertex, Num Verticies)
	}

	//walls
	if (c == 'W' ) {
		//Translate the model (matrix) left and back
		model = glm::mat4(1); //Load intentity
		model = glm::translate(model, glm::vec3(Tx, Ty, Tz));
		model = glm::scale(model,2.f*glm::vec3(.1f, .1f, .1f)); //scale example
		glUniformMatrix4fv(uniModel, 1, GL_FALSE, glm::value_ptr(model));

		//Set which texture to use (0 = wood texture ... bound to GL_TEXTURE0)
		
		
		glUniform1i(uniTexID, 1);

		//Draw an instance of the model (at the position & orientation specified by the model matrix above)
		glDrawArrays(GL_TRIANGLES, startVertCube, numVertsCube); //(Primitive Type, Start Vertex, Num Verticies)
	}

	//floor
	if (c == 'F') {
		//Translate the model (matrix) left and back
		model = glm::mat4(1); //Load intentity
		model = glm::translate(model, glm::vec3(Tx, Ty, Tz));
		model = glm::scale(model, 2.f * glm::vec3(.1f, .1f, .1f)); //scale example
		glUniformMatrix4fv(uniModel, 1, GL_FALSE, glm::value_ptr(model));

		//Set which texture to use (0 = wood texture ... bound to GL_TEXTURE0)

		glUniform1i(uniTexID, 2);

		//Draw an instance of the model (at the position & orientation specified by the model matrix above)
		glDrawArrays(GL_TRIANGLES, startVertCube, numVertsCube); //(Primitive Type, Start Vertex, Num Verticies)
	}


	//doors
	if (c == 'A' || c == 'B' || c == 'C' || c == 'D' || c == 'E') {
		if(DoorOpen[c]){}
		else {
			model = glm::mat4(1); //Load intentity
			model = glm::translate(model, glm::vec3(Tx, Ty, Tz));
			model = glm::scale(model, 2.f * glm::vec3(.1f, .1f, .1f)); //scale example

			uniModel = glGetUniformLocation(shaderProgram, "model");

			glUniformMatrix4fv(uniModel, 1, GL_FALSE, glm::value_ptr(model));


			//Set which texture to use (0 = wood texture ... bound to GL_TEXTURE0)
			glUniform1i(uniTexID, 0);
			if (c == 'A') {
				glUniform1i(uniTexID, 21);
			}
			if (c == 'B') {
				glUniform1i(uniTexID, 22);
			}
			if (c == 'C') {
				glUniform1i(uniTexID, 23);
			}
			if (c == 'D') {
				glUniform1i(uniTexID, 24);
			}
			if (c == 'E') {
				glUniform1i(uniTexID, 25);
			}

			//Draw an instance of the model (at the position & orientation specified by the model matrix above)
			glDrawArrays(GL_TRIANGLES, startVertCube, numVertsCube); //(Primitive Type, Start Vertex, Num Verticies)
		}
	}

	//keys
	if (c == 'a' || c == 'b' || c == 'c' || c == 'd' || c == 'e' ) { //stands for keys
		if (playOwnItem[c]) {
			//Rotate model (matrix) based on how much time has past
			model = glm::mat4(1);
			model = glm::translate(model, glm::vec3(camPosx+0.008-0.002*(c-'a'), camPosy+goDiry, camPosz+goDirz));
			model = glm::rotate(model, timePast * 3.14f / 2, glm::vec3(0.0f, 1.0f, 1.0f));
			model = glm::rotate(model, timePast * 3.14f / 4, glm::vec3(1.0f, 0.0f, 0.0f));
			model = glm::scale(model, glm::vec3(.001f, .001f, .001f)); //An example of scale
			uniModel = glGetUniformLocation(shaderProgram, "model");
			glUniformMatrix4fv(uniModel, 1, GL_FALSE, glm::value_ptr(model)); //pass model matrix to shader

			//Set which texture to use (-1 = no texture)
			glUniform1i(uniTexID, -1);
			if (c == 'a') {
				glUniform1i(uniTexID, 11);
			}
			if (c == 'b') {
				glUniform1i(uniTexID, 12);
			}
			if (c == 'c') {
				glUniform1i(uniTexID, 13);
			}
			if (c == 'd') {
				glUniform1i(uniTexID, 14);
			}
			if (c == 'e') {
				glUniform1i(uniTexID, 15);
			}

			//Draw an instance of the model (at the position & orientation specified by the model matrix above)
			glDrawArrays(GL_TRIANGLES, startVertCube, numVertsCube); //(Primitive Type, Start Vertex, Num Verticies)
		}
		else {

			//Rotate model (matrix) based on how much time has past
			model = glm::mat4(1);
			model = glm::translate(model, glm::vec3(Tx, Ty, Tz));
			model = glm::rotate(model, timePast * 3.14f / 2, glm::vec3(0.0f, 1.0f, 1.0f));
			model = glm::rotate(model, timePast * 3.14f / 4, glm::vec3(1.0f, 0.0f, 0.0f));
			model = glm::scale(model, glm::vec3(.05f, .05f, .05f)); //An example of scale
			uniModel = glGetUniformLocation(shaderProgram, "model");
			glUniformMatrix4fv(uniModel, 1, GL_FALSE, glm::value_ptr(model)); //pass model matrix to shader

			//Set which texture to use (-1 = no texture)
			glUniform1i(uniTexID, -1);
			if (c == 'a') {
				glUniform1i(uniTexID, 11);
			}
			if (c == 'b') {
				glUniform1i(uniTexID, 12);
			}
			if (c == 'c') {
				glUniform1i(uniTexID, 13);
			}
			if (c == 'd') {
				glUniform1i(uniTexID, 14);
			}
			if (c == 'e') {
				glUniform1i(uniTexID, 15);
			}

			//Draw an instance of the model (at the position & orientation specified by the model matrix above)
			glDrawArrays(GL_TRIANGLES, startVertCube, numVertsCube); //(Primitive Type, Start Vertex, Num Verticies)
		}
	}




}
//in this case, model1 will be teapot and model2 will be knot.
void drawMap(int shaderProgram) {

	//GLint uniColor = glGetUniformLocation(shaderProgram, "inColor");
	//glm::vec3 colVec(colR, colG, colB);
	//glUniform3fv(uniColor, 1, glm::value_ptr(colVec));

	GLint uniColorID = glGetUniformLocation(shaderProgram, "ColorID");
	GLint uniTexID = glGetUniformLocation(shaderProgram, "texID");



	//add up lit to the map
	for (int i = -1; i < mapW + 1; i++) {
		drawObject(shaderProgram, uniColorID,uniTexID, 'W', 0, i*0.2, 0.2);
	}
	//add bot lit 
	for (int i = -1; i < mapW + 1; i++) {
		drawObject(shaderProgram, uniColorID,uniTexID, 'W', 0, i * 0.2, -0.2*mapH);
	}

	//left lit
	for (int j = 0; j < mapH; j++) {
		drawObject(shaderProgram, uniColorID,uniTexID, 'W', 0, -0.2, -0.2 * j);
	}

	//right lit
	for (int j = 0; j < mapH; j++) {
		drawObject(shaderProgram, uniColorID,uniTexID, 'W', 0, 0.2*mapW, -0.2 * j);
	}



	float tempX = 0;
	float tempY = 0;
	float tempZ = 0;
	for (int i = 0; i < mapH; i++) {
		tempY = 0;
		for (int j = 0; j < mapW; j++) {
			int index = i * mapH + j;
			//printf("%c\n",mapArray[index]);
			if (mapArray[index] == 'S') {
				StartX = tempX;
				StartY = tempY;
				StartZ = tempZ;
			}
			drawObject(shaderProgram, uniColorID,uniTexID, mapArray[index], tempX, tempY, tempZ);
			drawObject(shaderProgram, uniColorID,uniTexID, 'F', tempX-0.2, tempY, tempZ);
			tempY += 0.2;
		}
		tempZ -= 0.2;
	}


	//you win square
	//drawObject(shaderProgram, uniColorID, uniTexID, 'W', 4, 0.1 * mapW, -0.1 * mapH);
	if (win) {
		glm::mat4 model = glm::mat4(1);
		GLint uniModel = glGetUniformLocation(shaderProgram, "model");
		model = glm::mat4(1); //Load intentity
		model = glm::translate(model, glm::vec3(0, 0.1 * (mapW-1), 0.1 * (mapH-1)));
		model = glm::scale(model, 5.f * glm::vec3(.1f, .1f, .1f)); //scale example
		glUniformMatrix4fv(uniModel, 1, GL_FALSE, glm::value_ptr(model));
		//Set which texture to use (0 = wood texture ... bound to GL_TEXTURE0)
		glUniform1i(uniTexID, 3);
		//Draw an instance of the model (at the position & orientation specified by the model matrix above)
		glDrawArrays(GL_TRIANGLES, startVertCube, numVertsCube); //(Primitive Type, Start Vertex, Num Verticies)
	}







	

	

}

// Create a NULL-terminated string by reading the provided file
static char* readShaderSource(const char* shaderFile) {
	FILE* fp;
	long length;
	char* buffer;

	// open the file containing the text of the shader code
	fp = fopen(shaderFile, "r");

	// check for errors in opening the file
	if (fp == NULL) {
		printf("can't open shader source file %s\n", shaderFile);
		return NULL;
	}

	// determine the file size
	fseek(fp, 0, SEEK_END); // move position indicator to the end of the file;
	length = ftell(fp);  // return the value of the current position

	// allocate a buffer with the indicated number of bytes, plus one
	buffer = new char[length + 1];

	// read the appropriate number of bytes from the file
	fseek(fp, 0, SEEK_SET);  // move position indicator to the start of the file
	fread(buffer, 1, length, fp); // read all of the bytes

	// append a NULL character to indicate the end of the string
	buffer[length] = '\0';

	// close the file
	fclose(fp);

	// return the string
	return buffer;
}

// Create a GLSL program object from vertex and fragment shader files
GLuint InitShader(const char* vShaderFileName, const char* fShaderFileName) {
	GLuint vertex_shader, fragment_shader;
	GLchar* vs_text, * fs_text;
	GLuint program;

	// check GLSL version
	printf("GLSL version: %s\n\n", glGetString(GL_SHADING_LANGUAGE_VERSION));

	// Create shader handlers
	vertex_shader = glCreateShader(GL_VERTEX_SHADER);
	fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);

	// Read source code from shader files
	vs_text = readShaderSource(vShaderFileName);
	fs_text = readShaderSource(fShaderFileName);

	// error check
	if (vs_text == NULL) {
		printf("Failed to read from vertex shader file %s\n", vShaderFileName);
		exit(1);
	}
	else if (DEBUG_ON) {
		printf("Vertex Shader:\n=====================\n");
		printf("%s\n", vs_text);
		printf("=====================\n\n");
	}
	if (fs_text == NULL) {
		printf("Failed to read from fragent shader file %s\n", fShaderFileName);
		exit(1);
	}
	else if (DEBUG_ON) {
		printf("\nFragment Shader:\n=====================\n");
		printf("%s\n", fs_text);
		printf("=====================\n\n");
	}

	// Load Vertex Shader
	const char* vv = vs_text;
	glShaderSource(vertex_shader, 1, &vv, NULL);  //Read source
	glCompileShader(vertex_shader); // Compile shaders

	// Check for errors
	GLint  compiled;
	glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &compiled);
	if (!compiled) {
		printf("Vertex shader failed to compile:\n");
		if (DEBUG_ON) {
			GLint logMaxSize, logLength;
			glGetShaderiv(vertex_shader, GL_INFO_LOG_LENGTH, &logMaxSize);
			printf("printing error message of %d bytes\n", logMaxSize);
			char* logMsg = new char[logMaxSize];
			glGetShaderInfoLog(vertex_shader, logMaxSize, &logLength, logMsg);
			printf("%d bytes retrieved\n", logLength);
			printf("error message: %s\n", logMsg);
			delete[] logMsg;
		}
		exit(1);
	}

	// Load Fragment Shader
	const char* ff = fs_text;
	glShaderSource(fragment_shader, 1, &ff, NULL);
	glCompileShader(fragment_shader);
	glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, &compiled);

	//Check for Errors
	if (!compiled) {
		printf("Fragment shader failed to compile\n");
		if (DEBUG_ON) {
			GLint logMaxSize, logLength;
			glGetShaderiv(fragment_shader, GL_INFO_LOG_LENGTH, &logMaxSize);
			printf("printing error message of %d bytes\n", logMaxSize);
			char* logMsg = new char[logMaxSize];
			glGetShaderInfoLog(fragment_shader, logMaxSize, &logLength, logMsg);
			printf("%d bytes retrieved\n", logLength);
			printf("error message: %s\n", logMsg);
			delete[] logMsg;
		}
		exit(1);
	}

	// Create the program
	program = glCreateProgram();

	// Attach shaders to program
	glAttachShader(program, vertex_shader);
	glAttachShader(program, fragment_shader);

	// Link and set program to use
	glLinkProgram(program);

	return program;
}
