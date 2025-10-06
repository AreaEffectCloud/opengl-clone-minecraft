#include <iostream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

const int screen_width = 640;
const int screen_height = 400;

void setup_3d_transform(unsigned int shaderProgramID) {
    // 3d to 2d
    glm::mat4 projection = glm::perspective(
        glm::radians(45.0f),
        (float)screen_width / (float)screen_height,
        0.1f,
        100.0f
    );
    unsigned int projectionLoc = glGetUniformLocation(shaderProgramID, "projection");
    glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));

    // define camera position and facing
    glm::mat4 view = glm::lookAt(
        glm::vec3(3.0f, 3.0f, 3.0f),
        glm::vec3(0.0f, 0.0f, 0.0f),
        glm::vec3(0.0f, 1.0f, 0.0f)
    );
    unsigned int viewLoc = glGetUniformLocation(shaderProgramID, "view");
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));

    // define block pos
    glm::mat4 model = glm::mat4(1.0f);

    // e.g. move a block to 5
    // model = glm::translate(model, glm::vec3(5.0f, 0.0f, 0.0f));

    unsigned int modelloc = glGetUniformLocation(shaderProgramID, "model");
    glUniformMatrix4fv(modelloc, 1, GL_FALSE, glm::value_ptr(model));
}


// close window on ESC key press
static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GLFW_TRUE);
}

int main() {
    glfwInit();

    // create a windowed mode window
    GLFWwindow* window = glfwCreateWindow(600, 400, "OpenGL Window", NULL, NULL);
    if (!window) {
        // window or context creation failed
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }

    // to use the OpenGL API
    glfwMakeContextCurrent(window);
    // use an exteension loader library
    gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);

    while (!glfwWindowShouldClose(window)) {
        // keep running
        glClear(GL_COLOR_BUFFER_BIT);
        glfwSwapBuffers(window);
        glfwPollEvents();

        // set the key callback
        glfwSetKeyCallback(window, key_callback);

        int width, height;
        glfwGetFramebufferSize(window, &width, &height);
        glViewport(0, 0, width, height);
    }

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
