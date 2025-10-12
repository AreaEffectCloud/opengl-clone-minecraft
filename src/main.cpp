#include <iostream>
#include <string>
#include <fstream>
#include <sstream>

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

#include "include/shader.h"
#include "include/camera.h"

const int WORLD_SIZE = 16;
int world[WORLD_SIZE][WORLD_SIZE][WORLD_SIZE];

int SCR_WIDTH = 800, SCR_HEIGHT = 600;
const char* vertex_shader_source = "./../src/assets/shader/vertex_shader.glsl";
const char* fragment_shader_source = "./../src/assets/shader/fragment_shader.glsl";

Camera camera(glm::vec3(0.0f, 1.0f, 3.0f));
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

float deltaTime = 0.0f;
float lastFrame = 0.0f;

static void framebuffer_size_callback(GLFWwindow* window, int width, int height);
static void set_full_screen(bool full_screen, GLFWmonitor* monitor, int &SCR_WIDTH, int &SCR_HEIGHT);
static void key_callback(GLFWwindow* window);
static void mouse_callback(GLFWwindow* window, double xpos, double ypos);

int main() {
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return -1;
    }
    // full screen
    bool full_screen = false;
    GLFWmonitor* monitor = NULL;
    set_full_screen(full_screen, monitor, SCR_WIDTH, SCR_HEIGHT);
    // create a windowed mode window
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "OpenGL", monitor, NULL);
    if (!window) {
        // window or context creation failed
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    // to use the OpenGL API
    glfwMakeContextCurrent(window);

    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetCursorPosCallback(window, mouse_callback);

    // use an extension loader library
    gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);

    // define only 8 vertixes
    float vertices[] = {
        // positions          // colors           // texture coordinates
        // 1. front (+Z) (頂点 0-3)
        -0.5f, -0.5f,  0.5f,   0.0f, 0.0f, 1.0f,   0.0f, 1.0f, // 0: 左下
         0.5f, -0.5f,  0.5f,   1.0f, 0.0f, 1.0f,   1.0f, 1.0f, // 1: 右下
         0.5f,  0.5f,  0.5f,   1.0f, 1.0f, 1.0f,   1.0f, 0.0f, // 2: 右上
        -0.5f,  0.5f,  0.5f,   0.0f, 1.0f, 1.0f,   0.0f, 0.0f, // 3: 左上

        // 2. back (-Z) (頂点 4-7)
         0.5f, -0.5f, -0.5f,   1.0f, 0.0f, 0.0f,   0.0f, 1.0f, // 4: 右下
        -0.5f, -0.5f, -0.5f,   0.0f, 0.0f, 0.0f,   1.0f, 1.0f, // 5: 左下
        -0.5f,  0.5f, -0.5f,   0.0f, 1.0f, 0.0f,   1.0f, 0.0f, // 6: 左上
         0.5f,  0.5f, -0.5f,   1.0f, 1.0f, 0.0f,   0.0f, 0.0f, // 7: 右上
        
        // 3. left (-X) (頂点 8-11)
        -0.5f, -0.5f, -0.5f,   0.0f, 0.0f, 0.0f,   0.0f, 1.0f, // 8: 下奥
        -0.5f, -0.5f,  0.5f,   0.0f, 0.0f, 1.0f,   1.0f, 1.0f, // 9: 下前
        -0.5f,  0.5f,  0.5f,   0.0f, 1.0f, 1.0f,   1.0f, 0.0f, // 10: 上前
        -0.5f,  0.5f, -0.5f,   0.0f, 1.0f, 0.0f,   0.0f, 0.0f, // 11: 上奥

        // 4. right (+X) (頂点 12-15)
         0.5f, -0.5f,  0.5f,   1.0f, 0.0f, 1.0f,   0.0f, 1.0f, // 12: 下前
         0.5f, -0.5f, -0.5f,   1.0f, 0.0f, 0.0f,   1.0f, 1.0f, // 13: 下奥
         0.5f,  0.5f, -0.5f,   1.0f, 1.0f, 0.0f,   1.0f, 0.0f, // 14: 上奥
         0.5f,  0.5f,  0.5f,   1.0f, 1.0f, 1.0f,   0.0f, 0.0f, // 15: 上前

        // 5. bottom (-Y) (頂点 16-19)
        -0.5f, -0.5f, -0.5f,   0.0f, 0.0f, 0.0f,   0.0f, 1.0f, // 16: 左奥
         0.5f, -0.5f, -0.5f,   1.0f, 0.0f, 0.0f,   1.0f, 1.0f, // 17: 右奥
         0.5f, -0.5f,  0.5f,   1.0f, 0.0f, 1.0f,   1.0f, 0.0f, // 18: 右前
        -0.5f, -0.5f,  0.5f,   0.0f, 0.0f, 1.0f,   0.0f, 0.0f, // 19: 左前

        // 6. top (+Y) (頂点 20-23)
        -0.5f,  0.5f,  0.5f,   0.0f, 1.0f, 1.0f,   0.0f, 1.0f, // 20: 左前
         0.5f,  0.5f,  0.5f,   1.0f, 1.0f, 1.0f,   1.0f, 1.0f, // 21: 右前
         0.5f,  0.5f, -0.5f,   1.0f, 1.0f, 0.0f,   1.0f, 0.0f, // 22: 右奥
        -0.5f,  0.5f, -0.5f,   0.0f, 1.0f, 0.0f,   0.0f, 0.0f  // 23: 左奥
    };

    // element buffer object
    unsigned int indices[] = {
        0, 1, 2,     2, 3, 0,     // front
        4, 5, 6,     6, 7, 4,     // back
        8, 9, 10,    10, 11, 8,   // left
        12, 13, 14,  14, 15, 12,  // right
        16, 17, 18,  18, 19, 16,  // bottom
        20, 21, 22,  22, 23, 20   // top
    };

    const size_t VERTEX_ARRAY_SIZE = sizeof(vertices) / sizeof(vertices[0]);
    const int NUM_VERTICES = VERTEX_ARRAY_SIZE / 6;

    unsigned int VBO, VAO, EBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
 
    // pos: layout 0
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // color: layout 1 
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    // texture coor: layout 2
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);
    glBindVertexArray(0);

    Shader ourShader(vertex_shader_source, fragment_shader_source);

    unsigned int texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    int width, height, nrChannels;
    unsigned char* data = stbi_load("./../src/assets/block/cobblestone.png", &width, &height, &nrChannels, 0);
    if (data) {
        GLenum format = GL_RGB;
        if (nrChannels == 4) format = GL_RGBA;
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    } else {
        std::cerr << "Failed to load texture" << std::endl;
    }
    stbi_image_free(data);

    ourShader.use();
    ourShader.setInt("our_texture", 0);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK); // culling back side
    glFrontFace(GL_CCW); // define front side as counter clockwise

    // main loop
    while (!glfwWindowShouldClose(window)) {
        float currentFrame = (float)glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;
        key_callback(window);

        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        ourShader.use();
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture);

        // mvp
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        ourShader.setMat4("projection", projection);
        glm::mat4 view = camera.GetViewMatrix();
        ourShader.setMat4("view", view);
        glm::mat4 model = glm::mat4(1.0f);
        ourShader.setMat4("model", model);

        // drawing
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
        
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
    glDeleteTextures(1, &texture);
    glfwTerminate();
    return 0;
}

static void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

static void set_full_screen(bool full_screen, GLFWmonitor* monitor, int &SCR_WIDTH, int &SCR_HEIGHT) {
    if (full_screen) {
        monitor = glfwGetPrimaryMonitor();
        const GLFWvidmode* mode = glfwGetVideoMode(monitor);
        
        glfwWindowHint(GLFW_RED_BITS, mode->redBits);
        glfwWindowHint(GLFW_GREEN_BITS, mode->greenBits);
        glfwWindowHint(GLFW_BLUE_BITS, mode->blueBits);
        glfwWindowHint(GLFW_REFRESH_RATE, mode->refreshRate);
        
        SCR_WIDTH = mode->width;
        SCR_HEIGHT = mode->height;
    }
}

static void key_callback(GLFWwindow* window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GLFW_TRUE);

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.ProcessKeyboard(FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.ProcessKeyboard(BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.ProcessKeyboard(LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.ProcessKeyboard(RIGHT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
        camera.ProcessKeyboard(UP, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
        camera.ProcessKeyboard(DOWN, deltaTime);
}

static void mouse_callback(GLFWwindow* window, double xposIn, double yposIn) {
    float xpos = static_cast<float>(xposIn);
    float ypos = static_cast<float>(yposIn);

    if (firstMouse) {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // y軸は下が正のため反転
    lastX = xpos;
    lastY = ypos;

    camera.ProcessMouseMovement(xoffset, yoffset);
}
