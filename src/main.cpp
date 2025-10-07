#include <iostream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

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

    // Full Screen
    bool full_screen = false;
    GLFWmonitor* monitor = NULL;
    int win_width = 800, win_height = 600;
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

    // position
    const char* vertex_shader =
        "#version 330 core\n"
        "layout(location = 0) in vec3 vertex_position;"
        "layout(location = 1) in vec3 vertex_color;"
        "out vec3 color;"
        "void main() {"
        "  color = vertex_color;"
        "  gl_Position = vec4(vertex_position, 1.0);"
        "}";
    
    // color
    const char* fragment_shader =
        "#version 330 core\n"
        "in vec3 color;"
        "out vec4 frag_color;"
        "void main() {"
        "  frag_color = vec4(color, 1.0);"
        "}";

    GLuint vs = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vs, 1, &vertex_shader, NULL);
    glCompileShader(vs);
    // shader error logs
    int params = -1;
    glGetShaderiv(vs, GL_COMPILE_STATUS, &params);
    if (GL_TRUE != params) {
        int max_length = 2048, actual_length = 0;
        char slog[2048];
        glGetShaderInfoLog(vs, max_length, &actual_length, slog);
        std::cerr << "ERROR: Vertex Shader index " << vs << " did not compile.\n" << slog << std::ends;
        return -1;
    }
    
    GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fs, 1, &fragment_shader, NULL);
    glCompileShader(fs);
    // shader error logs
    glGetShaderiv(fs, GL_COMPILE_STATUS, &params);
    if (GL_TRUE != params) {
        int max_length = 2048, actual_length = 0;
        char slog[2048];
        glGetShaderInfoLog(fs, max_length, &actual_length, slog);
        std::cerr << "ERROR: Fragment Shader index " << fs << " did not compile.\n" << slog << std::ends;
        return -1;
    }

    GLuint shader_program = glCreateProgram();
    glAttachShader(shader_program, fs);
    glAttachShader(shader_program, vs);

    glBindAttribLocation(shader_program, 0, "vertex_position");
    glBindAttribLocation(shader_program, 1, "vertex_color");

    glLinkProgram(shader_program);

    // Check for linking errors
    glGetProgramiv(shader_program, GL_LINK_STATUS, &params);
    if (GL_TRUE != params) {
        int max_length = 2048, actual_length = 0;
        char plog[2048];
        glGetProgramInfoLog(shader_program, max_length, &actual_length, plog);
        std::cerr << "ERROR: Could not link shader program GL index " << shader_program << ".\n" << plog << std::ends;
        return -1;
    }


    // Frame Rate Counter
    double previous_seconds = glfwGetTime();
    double title_countdown_seconds= 0.1;

    // main loop
    while (!glfwWindowShouldClose(window)) {
        // calculate fps
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

        // get the time uniform location
        current_seconds = glfwGetTime();
        int time_location = glGetUniformLocation(shader_program, "time");

        // update window events
        glfwPollEvents();
        // wipe the drawing surface clear
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        // put the shader program, and the VAO, in focus in openGL's state machine
        glUseProgram(shader_program);
        glUniform1f(time_location, (float)current_seconds);
        glBindVertexArray(vao);
        glDrawArrays(GL_TRIANGLES, 0, 3);
        // draw points 0-3 from the currently bound VAO with current in-use shader
        glDrawArrays(GL_TRIANGLES, 0, 3);

        // put the stuff we've been drawing onto the visible area
        glfwSwapInterval(1);
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
