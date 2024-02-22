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
        myArray[5] = 0.5f;
        myArray[6] = 0.2f;
        myArray[7] = 0.8f;
        myArray[8] = 0.4f;
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

        // FFT texture attribute
        glGenBuffers(1, &gUBO);
        glBindBuffer(GL_UNIFORM_BUFFER, gUBO);
        glBufferData(GL_UNIFORM_BUFFER, sizeof(myArray), myArray, GL_STATIC_DRAW);

        glBindBufferBase(GL_UNIFORM_BUFFER, bindingPoint, gUBO);

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
        //glBindBuffer(GL_UNIFORM_BUFFER, gUBO);
        //glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(myArray), myArray);

        glUseProgram(gProgramID);
        glBindBuffer(GL_ARRAY_BUFFER, gVBO);
        GLuint blockIndex = glGetUniformBlockIndex(gProgramID, "MyBlock");
        glUniformBlockBinding(gProgramID, blockIndex, bindingPoint);

        commonGL->setMat4(gProgramID, "projection", commonGL->calculateProjection(640.0f, 480.0f)); // TODO: de optimizat, nu trebuie recalculata de fiecare data

        glm::mat4 trans = glm::mat4(1.0f);
        trans = glm::scale(trans, glm::vec3(width, height, 0));
        commonGL->setMat4(gProgramID, "transformation", trans);

        glm::mat4 view = glm::mat4(1.0f);
        view = glm::translate(view, glm::vec3(x, y, 0.0f));
        commonGL->setMat4(gProgramID, "view", view);

        commonGL->setVec2(gProgramID, "xy", x - width, y);

        std::cout << myArray[50] << std::endl;

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
};