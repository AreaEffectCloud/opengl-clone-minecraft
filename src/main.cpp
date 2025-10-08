#include <iostream>
#include <string>
#include <fstream>
#include <sstream>

#include <glad/glad.h>
#include <GLFW/glfw3.h>
// #include <glm/glm.hpp>
// #include <glm/gtc/matrix_transform.hpp>
// #include <glm/gtx/transform.hpp>

// close window on ESC key press
static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GLFW_TRUE);
}

static void set_full_screen(bool full_screen, GLFWmonitor* monitor, int &win_width, int &win_height) {
    if (full_screen) {
        monitor = glfwGetPrimaryMonitor();
        const GLFWvidmode* mode = glfwGetVideoMode(monitor);

        glfwWindowHint(GLFW_RED_BITS, mode->redBits);
        glfwWindowHint(GLFW_GREEN_BITS, mode->greenBits);
        glfwWindowHint(GLFW_BLUE_BITS, mode->blueBits);
        glfwWindowHint(GLFW_REFRESH_RATE, mode->refreshRate);

        win_width = mode->width;
        win_height = mode->height;
    }
}

static void display_fps(GLFWwindow* window, double &previous_seconds, double &title_countdown_seconds) {
    double current_seconds = glfwGetTime(); // get the current time
    double elapsed_seconds = current_seconds - previous_seconds;
    previous_seconds = current_seconds;

    title_countdown_seconds -= elapsed_seconds;
    if (title_countdown_seconds <= 0.0 && elapsed_seconds > 0.0) {
        double fps = 1.0 / elapsed_seconds;

        // create a string and put the FPS as the window title
        char tmp[256];
        sprintf(tmp, "FPS: %.2lf", fps);
        glfwSetWindowTitle(window, tmp);
        title_countdown_seconds = 0.1;
    }
}

// return the context of a shader file as a string
std::string readShaderFile(const char* filePath) {
    std::ifstream file(filePath);
    if (!file.is_open()) {
        std::cerr << "Failed to open shader file: " << filePath << std::endl;
        return "";
    }
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

// compile shader sources
GLuint compileShader(const char* shader_source, GLenum shader_type) {
    GLuint shader = glCreateShader(shader_type);
    glShaderSource(shader, 1, &shader_source, NULL);
    glCompileShader(shader);

    int params;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &params);
    if (params != GL_TRUE) {
        int max_length = 2048, actual_length = 0;
        char slog[2048];
        glGetShaderInfoLog(shader, max_length, &actual_length, slog);
        std::cerr << "ERROR: Shader compilation failed (" << (shader_type == GL_VERTEX_SHADER ? "Vertex" : "Fragment") << "): " << slog << std::ends;
        glDeleteShader(shader);
        return 0;
    }
    return shader;
}

GLuint loadShader(const char* vertex_shader_path, const char* fragment_shader_path) {
    std::string vertex_shader_code = readShaderFile(vertex_shader_path);
    std::string fragment_shader_code = readShaderFile(fragment_shader_path);
    if (vertex_shader_code.empty() || fragment_shader_code.empty()) {
        std::cerr << "ERROR: Could not read one or more shader files." << std::endl;
        return 0;
    }
    const char* vertex_shader_source = vertex_shader_code.c_str();
    const char* fragment_shader_source = fragment_shader_code.c_str();

    GLuint vs = compileShader(vertex_shader_source, GL_VERTEX_SHADER);
    GLuint fs = compileShader(fragment_shader_source, GL_FRAGMENT_SHADER);
    if (vs == 0 || fs == 0) {
        if (vs) glDeleteShader(vs);
        if (fs) glDeleteShader(fs);
        return 0;
    }

    GLuint program = glCreateProgram();
    glAttachShader(program, vs);
    glAttachShader(program, fs);

    glBindAttribLocation(program, 0, "vertex_position");
    glBindAttribLocation(program, 1, "vertex_color");
    glLinkProgram(program);

    int params;
    glGetProgramiv(program, GL_LINK_STATUS, &params);
    if (params != GL_TRUE) {
        int max_length = 2048, actual_length = 0;
        char plog[2048];
        glGetProgramInfoLog(program, max_length, &actual_length, plog);
        std::cerr << "ERROR: Program linking failed: " << plog << std::ends;
        glDeleteProgram(program);
        return 0;
    }

    glDetachShader(program, vs);
    glDetachShader(program, fs);
    glDeleteShader(vs);
    glDeleteShader(fs);
    return program;
}

int main() {
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return -1;
    }
    // full screen
    bool full_screen = false;
    int win_width = 800, win_height = 600;
    GLFWmonitor* monitor = NULL;
    set_full_screen(full_screen, monitor, win_width, win_height);

    // create a windowed mode window
    GLFWwindow* window = glfwCreateWindow(win_width, win_height, "OpenGL Window", monitor, NULL);
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
    // printf("Renderer: %s.\n", glGetString(GL_RENDERER));
    // printf("OpenGL version supported %s.\n", glGetString(GL_VERSION));

    // Define a triangle in a vertex buffer
    float points[] = {
        0.0f,  0.5f, 0.0f,  // Vertex 1 (X, Y)
        0.5f, -0.5f, 0.0f,  // Vertex 2 (X, Y)
        -0.5f, -0.5f, 0.0f   // Vertex 3 (X, Y)
    };

    float colors[] = {
        1.0f, 0.0f, 0.0f,  // Color for Vertex 1 (R, G, B)
        0.0f, 1.0f, 0.0f,  // Color for Vertex 2 (R, G, B)
        0.0f, 0.0f, 1.0f   // Color for Vertex 3 (R, G, B)
    };

    GLuint points_vbo = 0;
    glGenBuffers(1, &points_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, points_vbo);
    glBufferData(GL_ARRAY_BUFFER, 9 * sizeof(float), points, GL_STATIC_DRAW);

    GLuint colors_vbo = 0;
    glGenBuffers(1, &colors_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, colors_vbo);
    glBufferData(GL_ARRAY_BUFFER, 9 * sizeof(float), colors, GL_STATIC_DRAW);

    GLuint vao = 0;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, points_vbo);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
    glBindBuffer(GL_ARRAY_BUFFER, colors_vbo);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(0); // position
    glEnableVertexAttribArray(1); // color

    // Load the shaders
    GLuint shader_program = loadShader("./../src/vertex_shader.glsl", "./../src/fragment_shader.glsl");
    if (shader_program == 0) {
        glfwDestroyWindow(window);
        glfwTerminate();
        return -1;
    }

    // Frame Rate Counter
    double previous_seconds = glfwGetTime();
    double title_countdown_seconds= 0.1;

    // main loop
    while (!glfwWindowShouldClose(window)) {
        display_fps(window, previous_seconds, title_countdown_seconds);

        // get the time uniform location
        double current_seconds = glfwGetTime();
        int time_location = glGetUniformLocation(shader_program, "time");

        // update window events
        glfwPollEvents();
        // wipe the drawing surface clear
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glClearColor(0.0f, 0.0f, 0.4f, 1.0f);
        
        // put the shader program, and the VAO, in focus in openGL's state machine
        glUseProgram(shader_program);
        if (time_location != -1) {
            glUniform1f(time_location, (float)current_seconds);
        }
        glBindVertexArray(vao);
        glDrawArrays(GL_TRIANGLES, 0, 3);

        // put the stuff we've been drawing onto the visible area
        glfwSwapInterval(1);
        glfwSwapBuffers(window);

        glfwSetKeyCallback(window, key_callback);

        // int width, height;
        // glfwGetFramebufferSize(window, &width, &height);
        // glViewport(0, 0, width, height);
    }

    glDeleteProgram(shader_program);
    glDeleteVertexArrays(1, &vao);
    glDeleteBuffers(1, &points_vbo);
    glDeleteBuffers(1, &colors_vbo);
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
