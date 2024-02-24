#pragma once
CommonGL commonGL;

class visualFFT {
public:
    GLuint gProgramID;

    float *myArray;
	bool init(int x, int y, int width, int height, CommonGL *commonGL, float *fftBins)
	{
        this->myArray = fftBins;
        this->commonGL = commonGL;

        // Program
        gProgramID = glCreateProgram();

        // Vertex shader
        GLuint vertexShader = commonGL->loadShaderFromFile("fft_vertex.glsl", GL_VERTEX_SHADER);
        if (!vertexShader)
            return false;

        // Fragment shader
        GLuint fragmentShader = commonGL->loadShaderFromFile("fft_fragment.glsl", GL_FRAGMENT_SHADER);
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
            // positions          // colors           // texture coords
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

        /*
        // FFT texture attribute
        glGenBuffers(1, &gUBO);
        glBindBuffer(GL_UNIFORM_BUFFER, gUBO);
        glBufferData(GL_UNIFORM_BUFFER, sizeof(fftBins1), fftBins1, GL_STATIC_DRAW);

        glBindBufferBase(GL_UNIFORM_BUFFER, bindingPoint, gUBO);*/

        // Generate 2D Texture
        updateTexture();
        // Set texture wrapping and filtering

        // Check for errors
        std::cout << glGetError() << std::endl;

        //Create IBO
        glGenBuffers(1, &gIBO);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gIBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

        this->x = x;
        this->y = y;
        this->width = width;
        this->height = height;

        return true;
	}
    void render()
    {
        updateTexture();
        glBindTexture(GL_TEXTURE_1D, textureID);

        //glBindBuffer(GL_UNIFORM_BUFFER, gUBO);
        //glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(fftBins1), fftBins1);
        
        glUseProgram(gProgramID);
        glBindBuffer(GL_ARRAY_BUFFER, gVBO);
        
        //GLuint blockIndex = glGetUniformBlockIndex(gProgramID, "MyBlock");
        //glUniformBlockBinding(gProgramID, blockIndex, bindingPoint);

        commonGL->setMat4(gProgramID, "projection", commonGL->calculateProjection(640.0f, 480.0f)); // TODO: de optimizat, nu trebuie recalculata de fiecare data

        glm::mat4 trans = glm::mat4(1.0f);
        trans = glm::scale(trans, glm::vec3(width, height, 0));
        commonGL->setMat4(gProgramID, "transformation", trans);

        glm::mat4 view = glm::mat4(1.0f);
        view = glm::translate(view, glm::vec3(x, y, 0.0f));
        commonGL->setMat4(gProgramID, "view", view);

        commonGL->setVec2(gProgramID, "xy", x - width, y);

        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    }
    void setPosition(int x, int y)
    {
        this->x = x + width; // TODO: un argument daca vrem ca centrul sa fie intr-un colt sau la mijloc
        this->y = y + height;
    }

    ~visualFFT()
    {
        //delete myArray;
        // este treaba celui care ne furnizeaza fftBins (in constructor) sa faca curatenie
    }
private:
    int x, y, width, height;
    CommonGL *commonGL;
    GLuint gVBO = 0;
    GLuint gIBO = 0;
    GLuint gVAO = 0;
    GLuint gUBO;
    GLuint bindingPoint = 0;
    GLuint textureID;

    void updateTexture() {
        if (textureID == 0) {
            // If textureID is not initialized, generate it
            glGenTextures(1, &textureID);
            glBindTexture(GL_TEXTURE_1D, textureID);

            // Set the texture wrapping and filtering parameters
            glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        }
        else {
            // If textureID is already initialized, just bind it
            glBindTexture(GL_TEXTURE_1D, textureID);
        }

        // Set the texture data
        glTexImage1D(GL_TEXTURE_1D, 0, GL_R32F, 1024, 0, GL_RED, GL_FLOAT, myArray);
    }
};