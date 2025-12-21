#include "cube_renderer.hpp"
#include "cube_mesh.hpp"
#include "shader_utils.hpp"
#include <cstdio>
#include <cstring>
#include <vector>
#include <iostream>
#include <windows.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

std::string vertex_shader_path = "../src/assets/shader/vertex_shader.glsl";
std::string fragment_shader_path = "../src/assets/shader/fragment_shader.glsl";
std::string vertex_shader_source = loadShaderSourceFromFile(vertex_shader_path);
std::string fragment_shader_source = loadShaderSourceFromFile(fragment_shader_path);

// Texture Atlas
const char* texture_path = "../src/assets/textures/texture_atlas.png";

struct GLContext {
    GLuint programID;
    GLuint textureID;
};

static GLContext glContext;

// Utility function to check for OpenGL errors
void checkOpenGLError(const char* tag) {
    GLenum err;
    while ((err = glGetError()) != GL_NO_ERROR) {
        std::fprintf(stderr, "[ERROR][%s] OpenGL Error: %u\n", tag, err);
    }
}

namespace gfx {

    CubeRenderer::CubeRenderer() = default;

    CubeRenderer::~CubeRenderer() {
        if (m_instance_vbo) glDeleteBuffers(1, &m_instance_vbo);
        if (m_program) glDeleteProgram(m_program);
        delete m_cube_mesh;
    }

    GLuint CubeRenderer::compile_shader(const char* source, GLenum shader_type) {
        GLuint shader = glCreateShader(shader_type);
        glShaderSource(shader, 1, &source, nullptr);
        glCompileShader(shader);

        GLint success = 0;
        glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
        if (!success) {
            char buf[1024];
            glGetShaderInfoLog(shader, sizeof(buf), nullptr, buf);
            std::fprintf(stderr, "[CubeRenderer] Shader compilation error: %s", buf);
            std::printf("[CubeRenderer] Source: %s", shader_type == GL_VERTEX_SHADER ? "Vertex Shader; " : "Fragment Shader; ");
            glDeleteShader(shader);
            return 0;
        }
        return shader;
    }

    GLuint CubeRenderer::link_program(GLuint vertex_shader, GLuint fragment_shader) {
        GLuint program = glCreateProgram();
        glAttachShader(program, vertex_shader);
        glAttachShader(program, fragment_shader);

        glBindAttribLocation(program, 0, "aPos");
        glBindAttribLocation(program, 1, "aNormal"); // if needed in future
        glBindAttribLocation(program, 2, "aTex");
        glBindAttribLocation(program, 4, "aFaceID");

        glLinkProgram(program);

        GLint success = 0;
        glGetProgramiv(program, GL_LINK_STATUS, &success);
        if (!success) {
            char buf[1024];
            glGetProgramInfoLog(program, sizeof(buf), nullptr, buf);
            std::fprintf(stderr, "Program linking error: %s\n", buf);
            glDeleteProgram(program);
            return 0;
        }
        return program;
    }

    bool CubeRenderer::init() {
        m_cube_mesh = new CubeMesh();
        if (vertex_shader_path.empty() || fragment_shader_path.empty()) {
            std::fprintf(stderr, "Failed to load shader sources: %s, %s\n", vertex_shader_path.c_str(), fragment_shader_path.c_str());
            return false;
        }

        // compile shaders
        GLuint vertex_shader = compile_shader(vertex_shader_source.c_str(), GL_VERTEX_SHADER);
        if (!vertex_shader) return false;
        GLuint fragment_shader = compile_shader(fragment_shader_source.c_str(), GL_FRAGMENT_SHADER);
        if (!fragment_shader) {
            glDeleteShader(vertex_shader);
            return false;
        }

        // link program
        m_program = link_program(vertex_shader, fragment_shader);
        glDeleteShader(vertex_shader);
        glDeleteShader(fragment_shader);
        if (!m_program) return false;

        // create instance VBO
        std::printf("[CubeRenderer] Creating instance VBO and Binding VAO...\n");

        glGenBuffers(1, &m_instance_vbo);
        // bind cube VAO and enable instanced attrib
        glBindVertexArray(m_cube_mesh->vao());
        
        std::printf("[CubeRenderer] Generated instance VBO: ID=%u\n", (unsigned int)m_instance_vbo);
        std::printf("[CubeRenderer] Finished creating instance VBO and binding VAO....\n");
        
        glBindBuffer(GL_ARRAY_BUFFER, m_instance_vbo);
        checkOpenGLError("After glBindBuffer for instance VBO");

        // Texture Loading using stbi
        {
            glGenTextures(1, &glContext.textureID);
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, glContext.textureID);

            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

            int width, height, nrChannels;
            unsigned char* data = stbi_load(texture_path, &width, &height, &nrChannels, 0);
            if (data) {
                GLenum format = GL_RGB;
                if (nrChannels == 4) format = GL_RGBA;
                glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);

                std::printf("[CubeRenderer / Texture] Data sent GPU: Size: %dx%d, Channels: %d, \nSource: %s\n", width, height, nrChannels, texture_path);
                glGenerateMipmap(GL_TEXTURE_2D);

            } else {
                std::printf("[CubeRenderer] Failed to load texture at path: %s\n", texture_path);
                return false;
            }

            stbi_image_free(data);
        }

        // glEnableVertexAttribArray(3); // aInstancePos
        // glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
        printf("[CubeRenderer] initialized: program=%u, cube_vao=%u\n", (unsigned)m_program, (unsigned)m_cube_mesh->vao());

        return true;
    }

    void CubeRenderer::update_instances(const std::vector<Vec3f>& positions) {
        glBindBuffer(GL_ARRAY_BUFFER, m_instance_vbo);
        m_instances = positions;
        if (!positions.empty()) {
            glBufferData(GL_ARRAY_BUFFER, positions.size() * sizeof(Vec3f), positions.data(), GL_STATIC_DRAW);
        } else {
            // keep buffer empty
            glBufferData(GL_ARRAY_BUFFER, 0, nullptr, GL_STATIC_DRAW);
        }
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        m_instance_count = static_cast<GLuint>(positions.size());
    }

    void CubeRenderer::draw(const float* viewProj4x4) {
        if (!m_program || !m_cube_mesh || m_instances.empty()) return;
        glUseProgram(m_program);

        // get uniforms locations
        GLint vpLoc = glGetUniformLocation(m_program, "uViewProj");
        if (vpLoc >= 0) glUniformMatrix4fv(vpLoc, 1, GL_FALSE, viewProj4x4);

        GLint posLoc = glGetUniformLocation(m_program, "uModelPos");
        GLint blockTypeLoc = glGetUniformLocation(m_program, "uBlockType");
        // GLint texLoc = glGetUniformLocation(m_program, "uTexIndex");

        glBindVertexArray(m_cube_mesh->vao());

        for (const auto& pos : m_instances) {
            if (posLoc >= 0) glUniform3f(posLoc, pos.x, pos.y, pos.z);
            if (blockTypeLoc >= 0) glUniform1i(blockTypeLoc, 3); // grass block for testing
            // if (texLoc >= 0) glUniform1i(texLoc, 0);

            // Draw call for single instance
            glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, (void*)0);
        }
        glBindVertexArray(0);
        glUseProgram(0);
    }
} // namespace gfx