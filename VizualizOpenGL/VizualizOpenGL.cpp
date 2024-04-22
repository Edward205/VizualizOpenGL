#define _CRT_SECURE_NO_WARNINGS
#include <SDL.h>
#include <SDL_image.h>
#include <glad/glad.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <SDL_opengl.h>

#include <stdio.h>
#include <string>
#include <iostream>
#include <fstream>
#include <cstring>

#include "glm/glm.hpp"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "GLcommon.h"
#include "UIcommon.h"
#include "button.h"
#include "audio.h"
#include "fft_visual.h"
#include "ui.h"

#include "kiss_fft.h"

/*
TODO:
De eliminat atributul de culori din CommonUI buton
De adaugat procedura de dealocare la inchidere
*/

//Screen dimension constants
const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 480;

//Starts up SDL, creates window, and initializes OpenGL
bool init();

//Initializes rendering program and clear color
bool initGL();

//Input handler
void handleKeys(unsigned char key, int x, int y);

//Per frame update
void update();

//Renders quad to the screen
void render();

//Frees media and shuts down SDL
void close();

//The window we'll be rendering to
SDL_Window* gWindow = NULL;

//OpenGL context
SDL_GLContext gContext;

//Render flag
bool gRenderQuad = true;

//Graphics program
GLuint gProgramID = 0;
GLint gVertexPos2DLocation = -1;
GLuint gVBO = 0;
GLuint imVBO = 0;
GLuint gIBO = 0;
GLuint gVAO = 0;
GLuint texture = 0;

std::pair<int, int> mousePos;

bool init()
{
	//Initialization flag
	bool success = true;

	//Initialize SDL
	if (SDL_Init(SDL_INIT_VIDEO) < 0)
	{
		printf("SDL could not initialize! SDL Error: %s\n", SDL_GetError());
		success = false;
	}
	else
	{
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
		
		//Create window
		gWindow = SDL_CreateWindow("Vizualizator", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN);
		if (gWindow == NULL)
		{
			printf("Window could not be created! SDL Error: %s\n", SDL_GetError());
			success = false;
		}
		else
		{
			//Create context
			gContext = SDL_GL_CreateContext(gWindow);
			if (gContext == NULL)
			{
				printf("OpenGL context could not be created! SDL Error: %s\n", SDL_GetError());
				success = false;
			}
			else
			{
				if (!gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress))
				{
					std::cout << "Failed to initialize GLAD" << std::endl;
					return -1;
				}

				//Use Vsync
				//if (SDL_GL_SetSwapInterval(1) < 0)
				//{
				//	printf("Warning: Unable to set VSync! SDL Error: %s\n", SDL_GetError());
				//}

				//Initialize OpenGL
				if (!initGL())
				{
					printf("Unable to initialize OpenGL!\n");
					success = false;
				}
			}
		}
	}

	return success;
}
CommonGL cacaGL;
CommonUIShader commonUI;
button testButton;
visualFFT visual_fft;
GradientBackground bottomGradient;
double volumeLevel = 0;
WavPlayer player("test.wav");
TextRenderer text;
bool initGL()
{
	//Success flag
	bool success = true;

	
	if (!cacaGL.init())
		return false; // TODO: cerr
	if(!commonUI.init(&cacaGL))
		return false;

	if (!text.init())
		return false;


	float colors[12] = {
		1.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 1.0f,
		1.0f, 1.0f, 0.0f,
	};
	bottomGradient.init(0, 400, 300, 80, &commonUI, colors);
	// BUG: daca gradient este initializat dupa butoane, primim eroare de acces la randare

	testButton.init(100, 100, 100, 100, "forward.png", &commonUI);
	visual_fft.init(200, 330, 128, 60, &cacaGL, player.fftBins);
	visual_fft.setPosition(200, 330); // de ce trebuie asta?


	// Set the texture parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	std::cout << glGetError() << std::endl;
	return success;
}

void handleKeys(unsigned char key, int x, int y)
{
	mousePos.first = x;
	mousePos.second = y;
}

void update()
{
	//No per frame update needed
}

void render()
{
	//Clear color buffer
	glClear(GL_COLOR_BUFFER_BIT);

	bottomGradient.render();

	if (visual_fft.selectedAmplitude != -1)
	{
		volumeLevel = visual_fft.selectedAmplitudeLevel;
	}
	testButton.setPosition(SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2 - volumeLevel * 200);
	testButton.render();

	visual_fft.render();


	//Disable vertex position
	//glDisableVertexAttribArray( gVertexPos2DLocation );

	//Unbind program
	glUseProgram(NULL);
	
}

void close()
{
	//Deallocate program
	glDeleteProgram(gProgramID);

	//Destroy window
	SDL_DestroyWindow(gWindow);
	gWindow = NULL;

	//Quit SDL subsystems
	SDL_Quit();
}

int main(int argc, char* args[])
{
	if (argc > 1)
	{
		player.init(args[1]);
	}
	else 
	{
		player.init("test.wav");
	}

	//Start up SDL and create window
	if (!init())
	{
		printf("Failed to initialize!\n");
	}
	else
	{
		//Main loop flag
		bool quit = false;

		//Event handler
		SDL_Event e;

		//Enable text input
		//SDL_StartTextInput();

		//While application is running
		while (!quit)
		{
			//Handle events on queue
			while (SDL_PollEvent(&e) != 0)
			{
				int x = 0, y = 0;
				SDL_GetMouseState(&x, &y);
				handleKeys(e.text.text[0], x, y);

				visual_fft.handleInput(e);

				//User requests quit
				if (e.type == SDL_QUIT)
				{
					quit = true;
				}
				//Handle keypress with current mouse position
				else if (e.type == SDL_TEXTINPUT)
				{
				}
			}

			//Render quad
			render();

			player.play();
			volumeLevel = player.processAudio();

			SDL_GL_SwapWindow(gWindow);
			//Update screen
		}

		//Disable text input
		SDL_StopTextInput();
	}

	//Free resources and close SDL
	close();

	return 0;
}
