#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <chrono>
#include <algorithm>
#include "engine.h"

GLuint compileShader(const char* vertexPath, const char* fragmentPath) {
    std::string vertexCode, fragmentCode;
    std::ifstream vShaderFile, fShaderFile;

    vShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    fShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    try {
        vShaderFile.open(vertexPath);
        fShaderFile.open(fragmentPath);
        std::stringstream vShaderStream, fShaderStream;
        vShaderStream << vShaderFile.rdbuf();
        fShaderStream << fShaderFile.rdbuf();
        vShaderFile.close();
        fShaderFile.close();
        vertexCode = vShaderStream.str();
        fragmentCode = fShaderStream.str();
    } catch (std::ifstream::failure& e) {
        exit(-1);
    }

    const char* vShaderCode = vertexCode.c_str();
    const char* fShaderCode = fragmentCode.c_str();
    GLuint vertex, fragment;
    int success;
    char infoLog[512];

    vertex = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex, 1, &vShaderCode, NULL);
    glCompileShader(vertex);
    glGetShaderiv(vertex, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(vertex, 512, NULL, infoLog);
    }

    fragment = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment, 1, &fShaderCode, NULL);
    glCompileShader(fragment);
    glGetShaderiv(fragment, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(fragment, 512, NULL, infoLog);
    }

    GLuint shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertex);
    glAttachShader(shaderProgram, fragment);
    glLinkProgram(shaderProgram);
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
    }

    glDeleteShader(vertex);
    glDeleteShader(fragment);
    return shaderProgram;
}

int main() {
    glfwInit();

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(1000, 400, "Options Heatmap w/ CUDA", NULL, NULL);
    if (!window) { 
        glfwTerminate(); 
        return -1; 
    }
    glfwMakeContextCurrent(window);

    glewExperimental = GL_TRUE; 
    if (glewInit() != GLEW_OK) {
        return -1;
    }

    GLuint shaderProgram = compileShader("shaders/heatmap.vert", "shaders/heatmap.frag");

    unsigned short listenPort = 54000;
    MarketClient client(listenPort);

    allocateAndLaunchBlackScholes(client);

    float maxPremium = *std::max_element(client.hostPrices.begin(), client.hostPrices.end());

    float vertices[] = {
         1.0f,  1.0f,   1.0f, 1.0f, 
         1.0f, -1.0f,   1.0f, 0.0f, 
        -1.0f,  1.0f,   0.0f, 1.0f, 
         1.0f, -1.0f,   1.0f, 0.0f, 
        -1.0f, -1.0f,   0.0f, 0.0f, 
        -1.0f,  1.0f,   0.0f, 1.0f  
    };

    GLuint VBO, VAO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);

    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, 500, 200, 0, GL_RED, GL_FLOAT, client.hostPrices.data());

    glUseProgram(shaderProgram);
    glUniform1f(glGetUniformLocation(shaderProgram, "maxPrice"), maxPremium);

    while (!glfwWindowShouldClose(window)) {
        if (client.pollLiveMarketData()) {
            auto start = std::chrono::high_resolution_clock::now();
            allocateAndLaunchBlackScholes(client);
            auto end = std::chrono::high_resolution_clock::now();
            std::chrono::duration<double, std::milli> diff = end - start;
            
            int maxIdx = 0;
            for(int i = 0; i < client.numOptions; i++) {
                if(client.hostPrices[i] > client.hostPrices[maxIdx]) maxIdx = i;
            }

            float dynamicMaxPremium = client.hostPrices[maxIdx];
            glUniform1f(glGetUniformLocation(shaderProgram, "maxPrice"), dynamicMaxPremium);

            std::cout << "\nComputation Time (100,000 options): " << diff.count() << " ms" << std::endl;
            std::cout << "Most Expensive Premium: $" << dynamicMaxPremium << " (Strike: $" << client.hostStrikePrices[maxIdx] << " | Expiring in: " << client.hostTimeToMaturity[maxIdx] << " yrs)" << std::endl;

            glBindTexture(GL_TEXTURE_2D, texture);
            glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 500, 200, GL_RED, GL_FLOAT, client.hostPrices.data());
        }

        glClear(GL_COLOR_BUFFER_BIT);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture);
        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES, 0, 6); 
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glfwTerminate();
    return 0;
}