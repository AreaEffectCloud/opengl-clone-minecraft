#include <iostream>
#include <cstdlib>
#include <chrono>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "world/world.hpp"
#include "world/world_renderer.hpp"
#include "util/camera.h"

using namespace ocm;

static const unsigned int SCR_WIDTH = 800, SCR_HEIGHT = 600;

static Camera camera(glm::vec3(5.0f, 20.0f, 40.0f));
static float lastX = (float)SCR_WIDTH / 2.0f;
static float lastY = (float)SCR_HEIGHT / 2.0f;
static bool firstMouse = true;

static float deltaTime = 0.0f;
static float lastFrame = 0.0f;

static void framebuffer_size_callback(GLFWwindow* window, int width, int height);
static void mouse_callback(GLFWwindow* window, double xpos, double ypos);
static void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
static void key_callback(GLFWwindow* window);

int main(int argc, char** argv) {
    uint32_t seed = 0;
    if (argc >= 2) {
        seed = static_cast<uint32_t>(std::strtoul(argv[1], nullptr, 10));
    } else {
        seed = static_cast<uint32_t>(std::chrono::system_clock::now().time_since_epoch().count());
    }

    World world;
    world.init(seed);
    // generate a few neighboring chunks optionally
    // For initial stage we generate only (0,0)
    world.dump_stats();

    if (!glfwInit()) {
        std::fprintf(stderr, "failed to initialize GLFW ☠☠☠\n");
        return EXIT_FAILURE;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // for MacOS
#endif
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // Create a window
    GLFWwindow* window = glfwCreateWindow((int)SCR_WIDTH, (int)SCR_HEIGHT, "worldgen - Debug", nullptr, nullptr);
    if (!window) {
        // window or context creation failed
        std::fprintf(stderr, "Failed to create GLFW window\n");
        glfwTerminate();
        return EXIT_FAILURE;
    }
    // to use the OpenGL API
    glfwMakeContextCurrent(window);

    // use an extension loader library
    if (gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::fprintf(stderr, "Failed to initialize GLFW\n");
        glfwDestroyWindow(window);
        glfwTerminate();
        return EXIT_FAILURE;
    }

    glViewport(0, 0, (int)SCR_WIDTH, (int)SCR_HEIGHT);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glEnable(GL_DEPTH_TEST);

    // initialize WorldRenderer
    WorldRenderer worldrenderer;
    if (!worldrenderer.init()) {
        std::fprintf(stderr, "WorldRenderer::init failed\n");
        // 描画が動作しないが、ワールド自体は初期化済みなので続行は可能
    }

    // world の spawn チャンクをアップロードしてインスタンスバッファを作る
    worldrenderer.upload_world(world);

    glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);

    while(!glfwWindowShouldClose(window)) {
        float currentFrame = (float)glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        key_callback(window);
        glClearColor(0.53f, 0.81f, 0.92f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
        glm::mat4 view = camera.GetViewMatrix();
        projection = glm::perspective(glm::radians(camera.Zoom),
                                    (float)SCR_WIDTH / (float)SCR_HEIGHT,
                                    0.1f, 1000.0f);
        glm::mat4 viewProj = projection * view;
    
        worldrenderer.draw(glm::value_ptr(viewProj));
    
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    worldrenderer.~WorldRenderer();
    world.destroy();
    glfwDestroyWindow(window);
    glfwTerminate();

    return EXIT_SUCCESS;
    // sample some blocks at the center column for verification
    // int cx = ocm::CHUNK_SIZE_X / 2;
    // int cz = ocm::CHUNK_SIZE_Z / 2;
    // int world_x = cx;
    // int world_z = cz;

    // int h = world.sample_height(world_x, world_z);
    // std::cout << "[Main] Sampled height at (" << world_x << ", " << world_z << ") = " << h << std::endl;

    // print topmost block ID at column center
    // ocm::BlockID id = world.get_block(world_x, h, world_z);
    // std::cout << "[Main] Top block ID at column = " << static_cast<int>(id) << std::endl;
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

void key_callback(GLFWwindow* window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.ProcessKeyboard(FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.ProcessKeyboard(BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.ProcessKeyboard(LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.ProcessKeyboard(RIGHT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
        camera.ProcessKeyboard(TOP, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
        camera.ProcessKeyboard(BOTTOM, deltaTime);
}

void mouse_callback(GLFWwindow* window, double xposIn, double yposIn) {
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

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
    camera.Zoom -= (float)yoffset;
    if (camera.Zoom < 1.0f)
        camera.Zoom = 1.0f;
    if (camera.Zoom > 45.0f)
        camera.Zoom = 45.0f;
}
