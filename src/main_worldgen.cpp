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
#include "util/camera.hpp"
#include "gfx/cube_renderer.hpp"

using namespace ocm;

static const unsigned int SCR_WIDTH = 800, SCR_HEIGHT = 600;
static const unsigned int POSITION_X = 400, POSITION_Y = 50;

static util::Camera camera;
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
    std::printf("\nRunning the program... \n\n[main_worldgen] Starting world generation\n");
    uint32_t seed = 0;
    if (argc >= 2) {
        seed = static_cast<uint32_t>(std::strtoul(argv[1], nullptr, 10));
    } else {
        seed = static_cast<uint32_t>(std::chrono::system_clock::now().time_since_epoch().count());
    }
    std::printf("[main_worldgen] using seed=%u\n", seed);

    World world;
    world.init(seed);
    std::printf("[main_worldgen] Generating 4x4 world chunks...\n");
    world.generate_world(4, 4); // generate 4x4 chunks
    world.dump_stats();

    int sample_h = world.sample_height(CHUNK_SIZE_X * 2, CHUNK_SIZE_Z * 2);
    std::printf("[main_worldgen] sample height at center = %d\n", sample_h);

    if (!glfwInit()) {
        std::fprintf(stderr, "failed to initialize GLFW\n");
        return EXIT_FAILURE;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_ANY_PROFILE);
    glfwWindowHint(GLFW_TRANSPARENT_FRAMEBUFFER, GLFW_FALSE);

    glfwWindowHint(GLFW_POSITION_X, POSITION_X);
    glfwWindowHint(GLFW_POSITION_Y, POSITION_Y);

    // Create a window
    GLFWwindow* window = glfwCreateWindow((int)SCR_WIDTH, (int)SCR_HEIGHT, "main_worldgen.cpp - Debug", nullptr, nullptr);
    if (!window) {
        // window or context creation failed
        std::fprintf(stderr, "Failed to create GLFW window\n");
        glfwTerminate();
        return EXIT_FAILURE;
    }
    // to use the OpenGL API
    glfwMakeContextCurrent(window);

    // use an extension loader library
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::fprintf(stderr, "Failed to initialize GLFW\n");
        glfwDestroyWindow(window);
        glfwTerminate();
        return EXIT_FAILURE;
    }

    // texture
    glEnable(GL_FRAMEBUFFER_SRGB);
    glDisable(0x809D); // disable multisampling

    // depth and face culling
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    
    // culling
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW); // define front side as counter clockwise

    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glViewport(0, 0, (int)SCR_WIDTH, (int)SCR_HEIGHT);

    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // initialize WorldRenderer
    WorldRenderer worldrenderer;
    bool worldRendererReady = false;
    if (!worldrenderer.init()) {
        std::fprintf(stderr, "[main_worldgen] WorldRenderer::init failed\n");
        // 描画が動作しないが、ワールド自体は初期化済みなので続行は可能
    } else {
        worldRendererReady = true;
    }

    gfx::CubeRenderer cubeRenderer;
    bool cubeRendererReady = false;
    if (!cubeRenderer.init()) {
        std::fprintf(stderr, "CubeRenderer::init failed\n");
    } else {
        cubeRendererReady = true;
    }
    
    if (worldRendererReady) {
        std::printf("[main] Uploading combined 16-chunk mesh to GPU...\n");
        worldrenderer.upload_world(world);
        std::printf("[main] Upload complete.\n");
    }

    // --- position camera to look at the center of the spawn chunk ---
    {
        int centerX = (CHUNK_SIZE_X * 4) / 2;;
        int centerZ = (CHUNK_SIZE_Z * 4) / 2;
        float groundH = (float)world.sample_height(centerX, centerZ);

        glm::vec3 target = glm::vec3((float)centerX, groundH, (float)centerZ);
        glm::vec3 camPos = target + glm::vec3(-20.0f, 25.0f, -20.0f);
        camera.Position = camPos;

        glm::vec3 front = glm::normalize(target - camPos);
        camera.Front = front;
        camera.Yaw   = glm::degrees(std::atan2(front.z, front.x));
        camera.Pitch = glm::degrees(std::asin(front.y));
        
        camera.updateCameraVectors();

        std::printf("[Camera /main_worldgen] camera positioned at (%.2f, %.2f, %.2f) looking at (%.2f, %.2f, %.2f)\n",
            camera.Position.x, camera.Position.y, camera.Position.z,
            target.x, target.y, target.z);
    }

    while(!glfwWindowShouldClose(window)) {
        float currentFrame = (float)glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        key_callback(window);
        glClearColor(0.53f, 0.81f, 0.92f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
        glm::mat4 view = camera.GetViewMatrix();
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 1000.0f);
        glm::mat4 viewProj = projection * view;

        if (worldRendererReady)
            worldrenderer.draw(glm::value_ptr(viewProj), camera.Position);
        else if (cubeRendererReady)
            cubeRenderer.draw(glm::value_ptr(viewProj), camera.Position);

        GLenum err = glGetError();
        if (err != GL_NO_ERROR) {
            std::fprintf(stderr, "OpenGL error: 0x%X\n", err);
        }
    
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    worldrenderer.~WorldRenderer();
    world.destroy();
    glfwDestroyWindow(window);
    glfwTerminate();

    return EXIT_SUCCESS;
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

void key_callback(GLFWwindow* window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
        camera.ProcessKeyboard(util::FORWARD, deltaTime);
    }
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
        camera.ProcessKeyboard(util::BACKWARD, deltaTime);
    }
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
        camera.ProcessKeyboard(util::LEFT, deltaTime);
    }
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
        camera.ProcessKeyboard(util::RIGHT, deltaTime);
    }
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
        camera.ProcessKeyboard(util::TOP, deltaTime);
    }
    if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) {
        camera.ProcessKeyboard(util::BOTTOM, deltaTime);
    }
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
