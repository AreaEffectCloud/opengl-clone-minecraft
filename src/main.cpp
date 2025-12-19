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

static const unsigned int SCR_WIDTH = 800, SCR_HEIGHT = 600;
const char* vertex_shader_source = "./../src/assets/shader/vertex_shader.glsl";
const char* fragment_shader_source = "./../src/assets/shader/fragment_shader.glsl";

// static Camera camera(glm::vec3(5.0f, 20.0f, 40.0f));
static float lastX = (float)SCR_WIDTH / 2.0f;
static float lastY = (float)SCR_HEIGHT / 2.0f;
static bool firstMouse = true;

static float deltaTime = 0.0f;
static float lastFrame = 0.0f;

static void framebuffer_size_callback(GLFWwindow* window, int width, int height);
// static void set_full_screen(bool full_screen, GLFWmonitor* monitor, int &SCR_WIDTH, int &SCR_HEIGHT);
static void mouse_callback(GLFWwindow* window, double xpos, double ypos);
static void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
static void key_callback(GLFWwindow* window);

int main() {
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return -1;
    }

    // glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    // glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    // #ifdef __APPLE__
    //     glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // for MacOS
    // #endif
    //     glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // full screen
    // bool full_screen = false;
    // GLFWmonitor* monitor = NULL;
    // set_full_screen(full_screen, monitor, SCR_WIDTH, SCR_HEIGHT);

    // create a windowed mode window
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "OpenGL", NULL, NULL);
    if (!window) {
        // window or context creation failed
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    // to use the OpenGL API
    glfwMakeContextCurrent(window);

    // use an extension loader library
    gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);

    glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    // glEnable(GL_DEPTH_TEST);

    Shader ourShader(vertex_shader_source, fragment_shader_source);

    struct World world;
    world_init(&world, (u64)123456789ULL, 4);
    ivec3s center = { 0, 0, 0 };
    struct Chunk* center_chunk = world_create_chunk(&world, center);
    if (!center_chunk) {
        std::cerr << "Failed to create center chunk" << std::endl;
    }

    // glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
    
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

    // glEnable(GL_DEPTH_TEST);
    // glEnable(GL_CULL_FACE);
    // glCullFace(GL_BACK); // culling back side
    // glFrontFace(GL_CCW); // define front side as counter clockwise

    // main loop
    while (!glfwWindowShouldClose(window)) {
        float currentFrame = (float)glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        key_callback(window);
        glfwPollEvents();

        glClearColor(0.35f, 0.55f, 0.85f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
    //     ourShader.use();

    //     glm::mat4 view = camera.GetViewMatrix();
    //     projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 1000.0f);

    //     size_t total = world.chunks_size * world.chunks_size;
    //     for (size_t i = 0; i < total; ++i) {
    //         struct Chunk* chunk = world.chunks[i];
    //         if (!chunk || !chunk->mesh->uploaded) continue;
            
    //         glm::mat4 model = glm::mat4(1.0f);
    //         // もし mesh がチャンクローカル座標で作られているなら translate を入れる:
    //         // model = glm::translate(model, glm::vec3((float)chunk->position.x, (float)chunk->position.y, (float)chunk->position.z));
    //         glm::mat4 mvp = projection * view * model;
    //         ourShader.setMat4("uMVP", mvp);

    //         chunkmesh_render(chunk->mesh, CHUNK_MESH_OPAQUE);
    //     }
    //     glfwSwapBuffers(window);
    }

    // world_destroy(&world);
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}