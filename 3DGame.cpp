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
using namespace std;

int screenWidth = 800;
int screenHeight = 600;
float timePast = 0;

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
float colR = 1, colG = 1, colB = 1;
float camPosx = 3, camPosy = 0.4, camPosz = -0.4;
float camDirx = 0, camDiry = 0.4, camDirz = -0.4;


bool DEBUG_ON = true;
GLuint InitShader(const char* vShaderFileName, const char* fShaderFileName);
bool fullscreen = false;
void Win2PPM(int width, int height);

//srand(time(NULL));
float rand01() {
	return rand() / (float)RAND_MAX;
}

void drawMap(int shaderProgram);
void drawObject(int shaderProgram, GLint uniTexID, char c, float Tx, float Ty, float Tz);

int main(int argc, char* argv[]) {
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
	SDL_Surface* surface = SDL_LoadBMP("C:/Users/kevin/Desktop/MultiObjectTextures/wood.bmp");
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
	if (surface == NULL) { //If it failed, print the error
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
	if (surface == NULL) { //If it failed, print the error
		printf("Error: \"%s\"\n", SDL_GetError()); return 1;
	}
	GLuint tex2;
	glGenTextures(1, &tex2);

	//Load the texture into memory
	glActiveTexture(GL_TEXTURE1);

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

	//GLint colAttrib = glGetAttribLocation(phongShader, "inColor");
	//glVertexAttribPointer(colAttrib, 3, GL_FLOAT, GL_FALSE, 8*sizeof(float), (void*)(3*sizeof(float)));
	//glEnableVertexAttribArray(colAttrib);

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
		//if (mapArray[i] == 'S') {
		//	//calulate the camera postion
		//	camPosx = 0.1;
		//	camPosy = ((i+1) % 5) * 0.2 - 0.1;
		//	camPosz = ((i+1) / 5) * -0.2 - 0.1;
		//	printf("%f, %f, %f \n", camPosx, camPosy, camPosz);
		//	camDirx = 0;
		//	camDiry = 1;
		//	camDirz = 0;
		//}

		
	}
	//printf(mapArray);
	
	mapFile.close();









	//Event Loop (Loop forever processing each event as fast as possible)
	SDL_Event windowEvent;
	bool quit = false;
	while (!quit) {
		while (SDL_PollEvent(&windowEvent)) {  //inspect all events in the queue
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
			if (windowEvent.type == SDL_KEYUP && windowEvent.key.keysym.sym == SDLK_v) { //If "v" is pressed, top view mode
				camPosx = 3, camPosy = 0.4, camPosz = -0.4;
				camDirx = 0, camDiry = 0.4, camDirz = -0.4;
				
			}

			//SJG: Use key input to change the state of the object
			//     We can use the ".mod" flag to see if modifiers such as shift are pressed
			if (windowEvent.type == SDL_KEYDOWN && windowEvent.key.keysym.sym == SDLK_UP) { //If "up key" is pressed
				//if (windowEvent.key.keysym.mod & KMOD_SHIFT) objx -= .1; //Is shift pressed?
				objz += .1;
				/*camPosx += camDirx * 0.1;
				camPosy += camDiry * 0.1;
				camPosz += camDirz * 0.1;*/
			}
			if (windowEvent.type == SDL_KEYDOWN && windowEvent.key.keysym.sym == SDLK_DOWN) { //If "down key" is pressed
				objz -= .1;
				//if (windowEvent.key.keysym.mod & KMOD_SHIFT) objx += .1; //Is shift pressed?
				/*camPosx -= camDirx * 0.1;
				camPosy -= camDiry * 0.1;
				camPosz -= camDirz * 0.1;*/
			}
			if (windowEvent.type == SDL_KEYDOWN && windowEvent.key.keysym.sym == SDLK_LEFT) { //If "left key" is pressed
				objy -= .1;
			}
			if (windowEvent.type == SDL_KEYDOWN && windowEvent.key.keysym.sym == SDLK_RIGHT) { //If "right key" is pressed
				objy += .1;
			}
			if (windowEvent.type == SDL_KEYUP && windowEvent.key.keysym.sym == SDLK_c) { //If "c" is pressed
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

		glm::mat4 view = glm::lookAt(
			glm::vec3(camPosx, camPosy, camPosz),  //Cam Position
			glm::vec3(camDirx, camDiry, camDirz),  //Look at point direction
			glm::vec3(0.0f, 0.0f, 1.0f)); //Up
		glUniformMatrix4fv(uniView, 1, GL_FALSE, glm::value_ptr(view));

		glm::mat4 proj = glm::perspective(3.14f / 4, screenWidth / (float)screenHeight, 1.0f, 10.0f); //FOV, aspect, near, far
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

void drawObject(int shaderProgram, GLint uniTexID, char c, float Tx, float Ty, float Tz ) {
	glm::mat4 model = glm::mat4(1);
	GLint uniModel = glGetUniformLocation(shaderProgram, "model");

	//goal
	if (c == 'G') { //stands for goal, spining teapot
		//Rotate model (matrix) based on how much time has past
		model = glm::mat4(1);
		model = glm::translate(model, glm::vec3(Tx, Ty, Tz));
		model = glm::rotate(model, timePast * 3.14f / 2, glm::vec3(0.0f, 1.0f, 1.0f));
		model = glm::rotate(model, timePast * 3.14f / 4, glm::vec3(1.0f, 0.0f, 0.0f));
		model = glm::scale(model,glm::vec3(.1f,.1f,.1f)); //An example of scale
		uniModel = glGetUniformLocation(shaderProgram, "model");
		glUniformMatrix4fv(uniModel, 1, GL_FALSE, glm::value_ptr(model)); //pass model matrix to shader

		//Set which texture to use (-1 = no texture)
		glUniform1i(uniTexID, -1);

		//Draw an instance of the model (at the position & orientation specified by the model matrix above)
		glDrawArrays(GL_TRIANGLES, startVertTeapot, numVertsTeapot); //(Primitive Type, Start Vertex, Num Verticies)
	}
	//if (c == 't') { //stands for small teapot for now
	//	//Translate the model (matrix) left and back
	//	model = glm::mat4(1); //Load intentity
	//	model = glm::translate(model, glm::vec3(Tx, Ty, Tz));
	//	//model = glm::scale(model,2.f*glm::vec3(1.f,1.f,0.5f)); //scale example
	//	glUniformMatrix4fv(uniModel, 1, GL_FALSE, glm::value_ptr(model));

	//	//Set which texture to use (0 = wood texture ... bound to GL_TEXTURE0)
	//	glUniform1i(uniTexID, 0);

	//	//Draw an instance of the model (at the position & orientation specified by the model matrix above)
	//	glDrawArrays(GL_TRIANGLES, startVertTeapot, numVertsTeapot); //(Primitive Type, Start Vertex, Num Verticies)
	//}

	//start point
	if (c == 'S') {
		model = glm::mat4(1);
		model = glm::translate(model, glm::vec3(Tx, Ty, Tz));
		model = glm::scale(model, glm::vec3(.1f, .1f, .1f)); //scale this model
		//Translate the model (matrix) based on where objx/y/z is
		// ... these variables are set when the user presses the arrow keys
		model = glm::translate(model, glm::vec3(objx, objy, objz));

		//Set which texture to use (1 = brick texture ... bound to GL_TEXTURE1)
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
		/*if (c == 'A') {
			colR = 1;
			colG = 0;
			colB = 0;
		}*/
		model = glm::mat4(1); //Load intentity
		model = glm::translate(model, glm::vec3(Tx, Ty, Tz));
		model = glm::scale(model, 2.f * glm::vec3(.1f, .1f, .1f)); //scale example
		
		uniModel = glGetUniformLocation(shaderProgram, "model");

		glUniformMatrix4fv(uniModel, 1, GL_FALSE, glm::value_ptr(model));

		//Set which texture to use (0 = wood texture ... bound to GL_TEXTURE0)
		glUniform1i(uniTexID, 0);

		//Draw an instance of the model (at the position & orientation specified by the model matrix above)
		glDrawArrays(GL_TRIANGLES, startVertCube, numVertsCube); //(Primitive Type, Start Vertex, Num Verticies)
	}

	//keys
	if (c == 'a' || c == 'b' || c == 'c' || c == 'd' || c == 'e') { //stands for keys
		/*if (c == 'a') {
			colR = 1;
			colG = 0;
			colB = 0;
		}*/
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

		//Draw an instance of the model (at the position & orientation specified by the model matrix above)
		glDrawArrays(GL_TRIANGLES, startVertCube, numVertsCube); //(Primitive Type, Start Vertex, Num Verticies)
	}




}
//in this case, model1 will be teapot and model2 will be knot.
void drawMap(int shaderProgram) {

	GLint uniColor = glGetUniformLocation(shaderProgram, "inColor");
	glm::vec3 colVec(colR, colG, colB);
	glUniform3fv(uniColor, 1, glm::value_ptr(colVec));

	GLint uniTexID = glGetUniformLocation(shaderProgram, "texID");


	//add up lit to the map
	for (int i = -1; i < mapW + 1; i++) {
		drawObject(shaderProgram, uniTexID, 'W', 0, i*0.2, 0.2);
	}
	//add bot lit 
	for (int i = -1; i < mapW + 1; i++) {
		drawObject(shaderProgram, uniTexID, 'W', 0, i * 0.2, -0.2*mapH);
	}

	//left lit
	for (int j = 0; j < mapH; j++) {
		drawObject(shaderProgram, uniTexID, 'W', 0, -0.2, -0.2 * j);
	}

	//right lit
	for (int j = 0; j < mapH; j++) {
		drawObject(shaderProgram, uniTexID, 'W', 0, 0.2*mapW, -0.2 * j);
	}



	float tempX = 0;
	float tempY = 0;
	float tempZ = 0;
	for (int i = 0; i < mapH; i++) {
		tempY = 0;
		for (int j = 0; j < mapW; j++) {
			int index = i * mapH + j;
			//printf("%c\n",mapArray[index]);
			drawObject(shaderProgram, uniTexID, mapArray[index], tempX, tempY, tempZ);
			drawObject(shaderProgram, uniTexID, 'F', tempX-0.2, tempY, tempZ);
			tempY += 0.2;
		}
		tempZ -= 0.2;
	}




	//drawObject(shaderProgram, uniTexID, 'W', 0, 0, 0);


	
	/*drawObject(shaderProgram, uniTexID, "cube", 0, 0, 0);
	drawObject(shaderProgram, uniTexID, "cube", 0, 0, -0.2);
	drawObject(shaderProgram, uniTexID, "cube", 0, 0.4, -0.2);
	drawObject(shaderProgram, uniTexID, "cube", 0, 0.4, -0.4);*/








	

	

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
