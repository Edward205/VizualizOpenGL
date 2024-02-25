#pragma once
#include "UIcommon.h"
#include <SDL.h>
#include <SDL_image.h>
class button {
public:
	bool init(int x, int y, int width, int height, const char* texturePath, CommonUIShader *commonUI)
	{
		if (!commonUI->initialised)
		{
			std::cerr << "CommonUI is not initialised, cannot load a button." << std::endl;
			return false;
		}

		buttonTexture = loadTextureFromFile(texturePath);
		if (buttonTexture == -1)
			return false;

		// Set the texture parameters
		//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		this->x = x;
		this->y = y;
		this->width = width;
		this->height = height;
		this->commonUI = commonUI;
	}

	void render()
	{
		glBindTexture(GL_TEXTURE_2D, buttonTexture);

		commonUI->bind();
		commonUI->use();

		commonUI->setMat4("projection", commonUI->calculateProjection(640.0f, 480.0f));

		glm::mat4 trans = glm::mat4(1.0f);
		trans = glm::scale(trans, glm::vec3(width, height, 0));
		commonUI->setMat4("transformation", trans);

		glm::mat4 view = glm::mat4(1.0f);
		view = glm::translate(view, glm::vec3(x, y, 0.0f));
		commonUI->setMat4("view", view);

		commonUI->setFloat("colorRatio", 0); // vrem doar textura, fara culoare

		commonUI->render();
	}
	void setPosition(int x, int y)
	{
		this->x = x;
		this->y = y;
	}

	void dealocate()
	{
		glDeleteTextures(1, &buttonTexture);
	}
private:
	int x, y, width, height;
	CommonUIShader *commonUI;
	GLuint buttonTexture = -1;
	GLuint loadTextureFromFile(const char* texturePath)
	{
		// Load the PNG image using SDL
		SDL_Surface* surface = IMG_Load(texturePath);
		if (!surface) {
			printf("SDL could not load button texture: %s\n", SDL_GetError());
			return -1;
		}

		// Get the number of channels and the texture format
		GLint nOfColors = surface->format->BytesPerPixel;
		GLenum texture_format;
		if (nOfColors == 4) { // Contains an alpha channel
			if (surface->format->Rmask == 0x000000ff)
				texture_format = GL_RGBA;
			else
				texture_format = GL_BGRA;
		}
		else if (nOfColors == 3) { // No alpha channel
			if (surface->format->Rmask == 0x000000ff)
				texture_format = GL_RGB;
			else
				texture_format = GL_BGR;
		}
		else {
			printf("warning: the image is not truecolor.. this will probably break\n");
			return -1;
		}

		GLuint texture = -1;

		glGenTextures(1, &texture);
		glBindTexture(GL_TEXTURE_2D, texture);

		// Upload the texture data to OpenGL
		glTexImage2D(GL_TEXTURE_2D, 0, texture_format, surface->w, surface->h, 0, texture_format, GL_UNSIGNED_BYTE, surface->pixels);

		// Free the SDL surface
		SDL_FreeSurface(surface);

		return texture;
	}
};