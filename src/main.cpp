#include <iostream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// close window on ESC key press
static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GLFW_TRUE);
}

int main() {
    if (!glfwInit()) {
        // initialization failed
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return -1;
    }

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

    // use an extension loader library
    gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);

    printf("Renderer: %s.\n", glGetString(GL_RENDERER));
    printf("OpenGL version supported %s.\n", glGetString(GL_VERSION));

    // Define a triangle in a vertex buffer
    float points[] = {
        0.0f,  0.5f, 0.0f,  // Vertex 1 (X, Y)
        0.5f, -0.5f, 0.0f,  // Vertex 2 (X, Y)
        -0.5f, -0.5f, 0.0f   // Vertex 3 (X, Y)
    };

    GLuint vbo = 0;
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, 9 * sizeof(float), points, GL_STATIC_DRAW);

    GLuint vao = 0;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);

    // Shaders
    const char* vertex_shader =
        "#version 330 core\n"
        "in vec3 vp;"
        "void main() {"
        "  gl_Position = vec4(vp, 1.0);"
        "}";

    const char* fragment_shader =
        "#version 330 core\n"
        "out vec4 frag_color;"
        "void main() {"
        "  frag_color = vec4(0.6, 0.5, 0.7, 1.0);"
        "}";

    GLuint vs = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vs, 1, &vertex_shader, NULL);
    glCompileShader(vs);

    GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fs, 1, &fragment_shader, NULL);
    glCompileShader(fs);

    GLuint shader_program = glCreateProgram();
    glAttachShader(shader_program, fs);
    glAttachShader(shader_program, vs);
    glLinkProgram(shader_program);

    // main loop
    while (!glfwWindowShouldClose(window)) {
        // update window events
        glfwPollEvents();
        // wipe the drawing surface clear
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // put the shader program, and the VAO, in focus in openGL's state machine
        glUseProgram(shader_program);
        glBindVertexArray(vao);

        // draw points 0-3 from the currently bound VAO with current in-use shader
        glDrawArrays(GL_TRIANGLES, 0, 3);

        // put the stuff we've been drawing onto the visible area
        glfwSwapBuffers(window);

        glfwSetKeyCallback(window, key_callback);

        // int width, height;
        // glfwGetFramebufferSize(window, &width, &height);
        // glViewport(0, 0, width, height);
    }

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
