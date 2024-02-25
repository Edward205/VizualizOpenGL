#pragma once
#include <glad/glad.h>
#include <SDL_opengl.h>
#include <stdio.h>
#include <fstream>
#include <sstream>
#include <iostream>

#include "GLcommon.h"

class CommonUIShader {
public:
    GLuint gProgramID;
    bool initialised = false;
    bool init(CommonGL *commonGL)
    {
        if (initialised)
        {
            std::cerr << "Attempted to initialise CommonUI while it's already initalised." << std::endl;
            return false;
        }

        this->commonGL = commonGL;

        // Program
        gProgramID = glCreateProgram();

        // Vertex shader
        GLuint vertexShader = commonGL->loadShaderFromFile("vertex.glsl", GL_VERTEX_SHADER);
        if (!vertexShader)
            return false;

        // Fragment shader
        GLuint fragmentShader = commonGL->loadShaderFromFile("fragment.glsl", GL_FRAGMENT_SHADER);
        if (!fragmentShader)
            return false;

        // Link program
        glAttachShader(gProgramID, vertexShader);
        glAttachShader(gProgramID, fragmentShader);
        glLinkProgram(gProgramID);

        //Check for errors
        GLint programSuccess = GL_TRUE;
        glGetProgramiv(gProgramID, GL_LINK_STATUS, &programSuccess);
        if (programSuccess != GL_TRUE)
        {
            printf("Error linking program %d!\n", gProgramID);
            commonGL->printProgramLog(gProgramID);
            return false;
        }

        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);

        //VBO data
        float vertices[] = {
             // positions         // colors           // texture coords
             1.0f,  1.0f, 0.0f,   1.0f, 0.0f, 0.0f,   1.0f, 1.0f,   // top right
             1.0f, -1.0f, 0.0f,   0.0f, 1.0f, 0.0f,   1.0f, 0.0f,   // bottom right
            -1.0f, -1.0f, 0.0f,   0.0f, 0.0f, 1.0f,   0.0f, 0.0f,   // bottom left
            -1.0f,  1.0f, 0.0f,   1.0f, 1.0f, 0.0f,   0.0f, 1.0f    // top left 
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

        // Check for errors
        std::cout << glGetError() << std::endl;

        //Create IBO
        glGenBuffers(1, &gIBO);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gIBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

        initialised = true;
        return true;
    }
    void use()
    {
        glUseProgram(gProgramID);
    }
    void bind()
    {
        glBindBuffer(GL_ARRAY_BUFFER, gVBO);
        //glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gIBO); nu cred ca ne trebuie? TODO: de testat cu mai multe instante
    }
    void render()
    {
        //use();
        //bind();
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    }

    void setMat4(const std::string& name, const glm::mat4& mat) const
    {
        glUniformMatrix4fv(glGetUniformLocation(gProgramID, name.c_str()), 1, GL_FALSE, &mat[0][0]);
    }
    void setVec4(const std::string& name, const glm::vec4& value) const
    {
        glUniform4fv(glGetUniformLocation(gProgramID, name.c_str()), 1, &value[0]);
    }
    void setFloat(const std::string& name, float value) const
    {
        glUniform1f(glGetUniformLocation(gProgramID, name.c_str()), value);
    }
    glm::mat4 calculateProjection(float screen_width, float screen_height)
    {
        float left = 0.0f;
        float right = screen_width;
        float bottom = screen_height;
        float top = 0.0f;
        float nearVal = -1.0f;
        float farVal = 1.0f;
        return glm::ortho(left, right, bottom, top, nearVal, farVal);
    }
    void dealocate()
    {
        glDeleteProgram(gProgramID);
    }
private:
    GLuint gVBO = 0;
    GLuint gIBO = 0;
    GLuint gVAO = 0;
    CommonGL *commonGL;
};
