#pragma once
#include "UIcommon.h"
#include <SDL.h>
#include <SDL_image.h>
#include "glm/glm.hpp"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

class InfiniteBackground {
public:
	int x, y, width, height;
	bool init(int x, int y, int width, int height, const char* texturePath, CommonUIShader* commonUI)
	{

	}
};
// ^ astea doua v ar putea fi combinate dpdv al functiei
class GradientBackground {
public:
	bool init(int x, int y, int width, int height, CommonUIShader* commonUI, float* colors)
	{
		if (!commonUI->initialised)
		{
			std::cerr << "CommonUI is not initialised, cannot load a button." << std::endl;
			return false;
		}

		//VBO data
		float vertices[] = {
			// positions         // colors           // texture coords
			1.0f,  1.0f, 0.0f,   colors[0], colors[1], colors[2],   1.0f, 1.0f,   // top right
			1.0f, -1.0f, 0.0f,   colors[3], colors[4], colors[5],   1.0f, 0.0f,   // bottom right
		   -1.0f, -1.0f, 0.0f,   colors[6], colors[7], colors[8],   0.0f, 0.0f,   // bottom left
		   -1.0f,  1.0f, 0.0f,   colors[9], colors[10], colors[11],   0.0f, 1.0f    // top left 
		};
		unsigned int indices[] = {
			0, 1, 3, // first triangle
			1, 2, 3  // second triangle
		};

		//Create VBO
		glGenBuffers(1, &gVBO);
		glBindBuffer(GL_ARRAY_BUFFER, gVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

		//Create VAO
		glGenVertexArrays(1, &gVAO);
		glBindVertexArray(gVAO);

		// position attribute
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(0);
		// color attribute
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
		glEnableVertexAttribArray(1);
		// texture coord attribute
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
		glEnableVertexAttribArray(2);

		this->x = x;
		this->y = y;
		this->width = width;
		this->height = height;
		this->commonUI = commonUI;
	}
	bool render()
	{
		glBindTexture(GL_TEXTURE_2D, 0);

		glBindBuffer(GL_ARRAY_BUFFER, gVBO); // facem bind la VBO cu culorile

		commonUI->use();
		commonUI->setMat4("projection", commonUI->calculateProjection(640.0f, 480.0f));

		glm::mat4 trans = glm::mat4(1.0f);
		trans = glm::scale(trans, glm::vec3(width, height, 0));
		commonUI->setMat4("transformation", trans);

		glm::mat4 view = glm::mat4(1.0f);
		view = glm::translate(view, glm::vec3(x, y, 0.0f));
		commonUI->setMat4("view", view);

		commonUI->setFloat("colorRatio", 1); // vrem doar culoare, fara textura

		commonUI->render();
		
		return true;
	}
private:
	int x, y, width, height;
	CommonUIShader* commonUI;
	GLuint gVBO;
	GLuint gVAO;
};