#pragma once
#include <glad/glad.h>
#include <SDL_opengl.h>
#include <stdio.h>
#include <fstream>
#include <sstream>
#include <iostream>

class CommonGL {
public:
	bool init() {
		// nimic de initializat inca
        return true;
	}
    void printShaderLog(GLuint shader)
    {
        //Make sure name is shader
        if (glIsShader(shader))
        {
            //Shader log length
            int infoLogLength = 0;
            int maxLength = infoLogLength;

            //Get info string length
            glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &maxLength);

            //Allocate string
            char* infoLog = new char[maxLength];

            //Get info log
            glGetShaderInfoLog(shader, maxLength, &infoLogLength, infoLog);
            if (infoLogLength > 0)
            {
                //Print Log
                printf("%s\n", infoLog);
            }

            //Deallocate string
            delete[] infoLog;
        }
        else
        {
            printf("Name %d is not a shader\n", shader);
        }
    }
    void printProgramLog(GLuint program)
    {
        //Make sure name is shader
        if (glIsProgram(program))
        {
            //Program log length
            int infoLogLength = 0;
            int maxLength = infoLogLength;

            //Get info string length
            glGetProgramiv(program, GL_INFO_LOG_LENGTH, &maxLength);

            //Allocate string
            char* infoLog = new char[maxLength];

            //Get info log
            glGetProgramInfoLog(program, maxLength, &infoLogLength, infoLog);
            if (infoLogLength > 0)
            {
                //Print Log
                printf("%s\n", infoLog);
            }

            //Deallocate string
            delete[] infoLog;
        }
        else
        {
            printf("Name %d is not a program\n", program);
        }
    }

    GLuint loadShaderFromFile(const char* filePath, GLenum shaderType) {
        GLuint shader = glCreateShader(shaderType);

        std::string shaderCode;
        std::ifstream shaderFile;

        // ensure ifstream object can throw exceptions:
        shaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
        try
        {
            // open file
            shaderFile.open(filePath);
            std::stringstream shaderStream;
            // read file buffer contents into streams
            shaderStream << shaderFile.rdbuf();
            // close file handlers
            shaderFile.close();
            // convert stream into string
            shaderCode = shaderStream.str();
        }
        catch (std::ifstream::failure& e)
        {
            std::cout << "Could not read shader file " << filePath << ", error: " << e.what() << std::endl;
            return -1;
        }
        const char* cShaderCode = shaderCode.c_str();

        //Set shader source
        glShaderSource(shader, 1, &cShaderCode, NULL);

        //Compile shader source
        glCompileShader(shader);

        //Check shader for errors
        GLint fShaderCompiled = GL_FALSE;
        glGetShaderiv(shader, GL_COMPILE_STATUS, &fShaderCompiled);
        if (fShaderCompiled != GL_TRUE)
        {
            printf("Unable to compile fragment shader %d!\n", shader);
            printShaderLog(shader);
            return -1;
        }
        return shader;
    }

    void setMat4(GLuint gProgramID, const std::string& name, const glm::mat4& mat) const
    {
        glUniformMatrix4fv(glGetUniformLocation(gProgramID, name.c_str()), 1, GL_FALSE, &mat[0][0]);
    }
    void setVec4(GLuint gProgramID, const std::string& name, const glm::vec4& value) const
    {
        glUniform4fv(glGetUniformLocation(gProgramID, name.c_str()), 1, &value[0]);
    }
    void setVec2(GLuint gProgramID, const std::string& name, float x, float y) const
    {
        glUniform2f(glGetUniformLocation(gProgramID, name.c_str()), x, y);
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
};