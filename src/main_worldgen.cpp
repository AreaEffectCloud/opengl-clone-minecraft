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

static Camera camera;
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
    std::printf("[main_worldgen] using seed=%u\n", seed);

    World world;
    world.init(seed);
    // generate a few neighboring chunks optionally
    // For initial stage we generate only (0,0)
    world.dump_stats();

    int sample_h = world.sample_height(CHUNK_SIZE_X / 2, CHUNK_SIZE_Z / 2);
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
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK); // culling back side
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
        std::fprintf(stderr, "WorldRenderer::init failed\n");
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

    // Add a cube instance for each non-air block in the world's spawn chunk
    // 
    // world の spawn チャンクをアップロードしてインスタンスバッファを作る
    bool didUploadInstances = false;
    if (worldRendererReady) {
        worldrenderer.upload_world(world);
        didUploadInstances = true;
    }
    // --- build instance list from world for debugging/verification ---
    std::vector<gfx::Vec3f> instances;
    // assume spawn chunk at (0,0)
    int base_cx = 0;
    int base_cz = 0;
    for (int z = 0; z < CHUNK_SIZE_Z; ++z) {
        for (int x = 0; x < CHUNK_SIZE_X; ++x) {
            for (int y = 0; y < CHUNK_SIZE_Y; ++y) {
                int world_x = base_cx * CHUNK_SIZE_X + x;
                int world_z = base_cz * CHUNK_SIZE_Z + z;
                auto id = world.get_block(world_x, y, world_z);
                if (id != BlockID::AIR) { // assume 0 == AIR
                    gfx::Vec3f p; p.x = static_cast<float>(world_x); p.y = static_cast<float>(y); p.z = static_cast<float>(world_z);
                    instances.push_back(p);
                }
            }
        }
    }
    std::printf("[Extra /main_worldgen] built %zu instances from world\n", instances.size());
    // If possible, upload instances to cubeRenderer (debug path) so we can be sure the instanced renderer path works.
    if (cubeRendererReady && !instances.empty()) {
        cubeRenderer.update_instances(instances);
        didUploadInstances = true;
        // prefer drawing with cubeRenderer for debug, disable worldrenderer drawing
        worldRendererReady = false;
        std::printf("[Extra /main_worldgen] uploaded %zu instances to cubeRenderer (debug path)\n", instances.size());
    }
    // もし world の高さがすべて 0 で描画対象がない場合のフォールバック:
    // world.sample_height を用いて判定する（中心点をチェック）。0 なら一時的にテストインスタンスを生成する。
    int centerX = CHUNK_SIZE_X / 2;
    int centerZ = CHUNK_SIZE_Z / 2;
    int centerH = world.sample_height(centerX, centerZ);
    if (centerH <= 0) {
        std::printf("[Extra /main_worldgen] detected zero-height world; creating fallback test instances\n");
        if (cubeRendererReady) {
            std::vector<gfx::Vec3f> testInstances;
            // create a CHUNK_SIZE_X x 2 x CHUNK_SIZE_Z block volume at world coords [0..CHUNK_SIZE_X-1] x [1..2] x [0..CHUNK_SIZE_Z-1]
            for (int z = 0; z < CHUNK_SIZE_Z; ++z) {
                for (int x = 0; x < CHUNK_SIZE_X; ++x) {
                    for (int y = 1; y <= 2; ++y) { // y=1,2 の薄い層
                        gfx::Vec3f p; p.x = static_cast<float>(x); p.y = static_cast<float>(y); p.z = static_cast<float>(z);
                        testInstances.push_back(p);
                    }
                }
            }
            std::printf("[Extra /main_worldgen] fallback will upload %zu test instances\n", testInstances.size());
            cubeRenderer.update_instances(testInstances);
            didUploadInstances = true;
            // Ensure we use cubeRenderer for draw by disabling worldRendererReady
            worldRendererReady = false;
            cubeRendererReady = true;
        } else {
            std::fprintf(stderr, "[Extra /main_worldgen] cubeRenderer not ready; cannot create fallback instances\n");
        }
    }
    if (!didUploadInstances) {
        std::fprintf(stderr, "[Extra /main_worldgen] warning: no instances uploaded (world may be empty)\n");
    }

    // --- position camera to look at the center of the spawn chunk ---
    // {
    //     glm::vec3 target = glm::vec3((float)(CHUNK_SIZE_X / 2), (float)world.sample_height(CHUNK_SIZE_X / 2, CHUNK_SIZE_Z / 2), (float)(CHUNK_SIZE_Z / 2));
    //     glm::vec3 camPos = target + glm::vec3(0.0f, 8.0f, 20.0f); // adjust as necessary
    //     camera.Position = camPos;
    //     glm::vec3 front = glm::normalize(target - camPos);
    //     // set Front directly (assumes Camera exposes Front member)
    //     camera.Front = front;
    //     // try to set yaw/pitch for compatibility (may be unused)
    //     float yaw = glm::degrees(atan2(front.z, front.x));
    //     float pitch = glm::degrees(asin(front.y));
    //     camera.Yaw = yaw;
    //     camera.Pitch = pitch;
    //     // if Camera has update function, call it (safe to call if exists)
    //     // camera.updateCameraVectors(); // uncomment if your Camera exposes this
    //     std::printf("[Camera /main_worldgen] camera positioned at (%.2f, %.2f, %.2f) looking at (%.2f, %.2f, %.2f)\n",
    //         camera.Position.x, camera.Position.y, camera.Position.z,
    //         target.x, target.y, target.z);
    // }

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
            worldrenderer.draw(glm::value_ptr(viewProj));
        else if (cubeRendererReady)
            cubeRenderer.draw(glm::value_ptr(viewProj));

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
        camera.ProcessKeyboard(FORWARD, deltaTime);
    }
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
        camera.ProcessKeyboard(BACKWARD, deltaTime);
    }
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
        camera.ProcessKeyboard(LEFT, deltaTime);
    }
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
        camera.ProcessKeyboard(RIGHT, deltaTime);
    }
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
        camera.ProcessKeyboard(TOP, deltaTime);
    }
    if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) {
        camera.ProcessKeyboard(BOTTOM, deltaTime);
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
