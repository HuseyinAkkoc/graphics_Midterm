#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "Math.h"
// Check the source code/ Linker issue/ 4.40 pm
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <cassert>
#include <cstdlib> // For rand()

void OnResize(GLFWwindow* window, int width, int height);
void OnInput(GLFWwindow* window);

const unsigned int SCR_WIDTH = 1920;
const unsigned int SCR_HEIGHT = 1080;

GLuint CreateShader(GLint type, const char* path) {
    GLuint shader = 0;
    try {
        // Load text file
        std::ifstream file;
        file.exceptions(std::ifstream::failbit | std::ifstream::badbit);
        file.open(path);

        
        std::stringstream stream;
        stream << file.rdbuf();
        file.close();

        // Verify shader type matches shader file extension
        const char* ext = strrchr(path, '.');
        switch (type) {
        case GL_VERTEX_SHADER:
            assert(strcmp(ext, ".vert") == 0);
            break;
        case GL_FRAGMENT_SHADER:
            assert(strcmp(ext, ".frag") == 0);
            break;
        default:
            assert(false && "Invalid shader type");
            break;
        }

        // Compile text as a shader
        std::string str = stream.str();
        const char* src = str.c_str();
        shader = glCreateShader(type);
        glShaderSource(shader, 1, &src, NULL);
        glCompileShader(shader);

        // Check for compilation errors
        GLint success;
        GLchar infoLog[512];
        glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
        if (!success) {
            glGetShaderInfoLog(shader, 512, NULL, infoLog);
            std::cout << "Shader failed to compile: \n" << infoLog << std::endl;
        }
    }
    catch (std::ifstream::failure& e) {
        std::cout << "Shader (" << path << ") not found: " << e.what() << std::endl;
    }
    return shader;
}

GLuint CreateProgram(GLuint vs, GLuint fs) {
    GLuint shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vs);
    glAttachShader(shaderProgram, fs);
    glLinkProgram(shaderProgram);

    //  Ask professor about linker 2019 error// Search it.
    // Check for linking errors
    int success;
    char infoLog[512];
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        shaderProgram = GL_NONE;
        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
    }

    return shaderProgram;
}

struct Vertex {
    Vector3 position;
    Vector3 color;
};

using Vertices = std::vector<Vertex>;

int main() {
    glfwInit();
    // Try with 3.3 and 4.6 version// 3.3 is working//4.35pm//
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL", NULL, NULL);
    if (window == NULL) {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, OnResize);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    GLuint vsDefault = CreateShader(GL_VERTEX_SHADER, "./assets/shaders/Default.vert");
    GLuint fsDefault = CreateShader(GL_FRAGMENT_SHADER, "./assets/shaders/Default.frag");
    GLuint shaderDefault = CreateProgram(vsDefault, fsDefault);
    glUseProgram(shaderDefault);

    // Define the initial triangle vertices and colors
    Vertex triangle[3] = {
        {{-1.0f, -1.0f, 0.0f}, {1.0f, 0.0f, 0.0f}}, // Red
        {{1.0f, -1.0f, 0.0f}, {0.0f, 1.0f, 0.0f}}, // Green
        {{0.0f, 1.0f, 0.0f}, {0.0f, 0.0f, 1.0f}}  // Blue
    };

    // Generate 30,000 vertices of the Sierpiński triangle
    // try doing other than 30000 vertices. Not tried yet.
    Vertices vertices(30000);
    Vertex currentVertex = { {0.0f, 0.0f, 0.0f}, {1.0f, 1.0f, 1.0f} }; // Initial vertex
    for (int i = 0; i < 30000; i++) {
        int n = rand() % 3; // Randomly select a triangle vertex
        currentVertex.position = (currentVertex.position + triangle[n].position) / 2.0f;
        currentVertex.color = triangle[n].color;
        vertices.push_back(currentVertex);
    }
    // Note to myself!!! watch LearnOpenGl tutorial 3rd video. Chech vbo and vao again.!!
    // Create a Vertex Array Object (VAO) and a Vertex Buffer Object (VBO)
    GLuint vao, vbo;
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);

    // Bind the VAO and VBO
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);

    // Upload the vertex data to the GPU
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);

    // Specify the vertex attribute layout
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, position));
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, color));
    glEnableVertexAttribArray(1);

    while (!glfwWindowShouldClose(window)) {
        OnInput(window);

        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        glDrawArrays(GL_POINTS, 0, vertices.size());
        // don't forget the swap back and front buffer.
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}
// Chcek to delete shahders after finish compiling.
// Read learnopenGl.com about it// CHECKED//4.47PM//LOOKS LIKE DON'T NEED// DONE
void OnInput(GLFWwindow* window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
}

void OnResize(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}