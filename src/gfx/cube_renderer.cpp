#include "cube_renderer.hpp"
#include "cube_mesh.hpp"
#include "shader_utils.hpp"
#include <cstdio>
#include <cstring>
#include <vector>
#include <iostream>
#include <windows.h>

std::string vertex_shader_path = "../src/assets/shader/vertex_shader.glsl";
std::string fragment_shader_path = "../src/assets/shader/fragment_shader.glsl";

std::string vertex_shader_source = loadShaderSourceFromFile(vertex_shader_path);
std::string fragment_shader_source = loadShaderSourceFromFile(fragment_shader_path);

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
            std::fprintf(stderr, "Shader compilation error: %s\n", buf);
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
        glBindAttribLocation(program, 1, "aNormal");
        glBindAttribLocation(program, 2, "aTex");
        glBindAttribLocation(program, 3, "aInstancePos");

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
        if (vertex_shader_source.empty() || fragment_shader_source.empty()) {
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
        std::printf("[CubeRenderer] Creating instance VBO...\n");
        glGenBuffers(1, &m_instance_vbo);
        std::printf("[CubeRenderer] Generated instance VBO: ID=%u\n", (unsigned int)m_instance_vbo);
        std::printf("[CubeRenderer] Finished creating instance VBO...\n");

        // bind cube VAO and enable instanced attrib
        std::printf("[CubeRenderer] Binding VAO...\n");
        glBindVertexArray(m_cube_mesh->vao());
        std::printf("[CubeRenderer] Finished binding VAO...\n");

        glBindBuffer(GL_ARRAY_BUFFER, m_instance_vbo);
        checkOpenGLError("After glBindBuffer for instance VBO");

        glEnableVertexAttribArray(3); // aInstancePos
        glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
        // glVertexAttribDivisor(3, 1); // advance per instance
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);

        printf("[CubeRenderer] initialized: program=%u, cube_vao=%u\n", (unsigned)m_program, (unsigned)m_cube_mesh->vao());

        return true;
    }

    void CubeRenderer::update_instances(const std::vector<Vec3f>& positions) {
        glBindBuffer(GL_ARRAY_BUFFER, m_instance_vbo);
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
        if (!m_program || !m_cube_mesh) return;

        glUseProgram(m_program);
        checkOpenGLError("After glUseProgram");

        GLint loc = glGetUniformLocation(m_program, "uViewProj");
        if (loc >= 0) glUniformMatrix4fv((GLint)loc, 1, GL_FALSE, viewProj4x4);
                
        glBindVertexArray(m_cube_mesh->vao());
        checkOpenGLError("After glBindVertexArray");

        // glDrawArraysInstanced(GL_TRIANGLES, 0, m_cube_mesh->vertex_count(), m_instance_count);
        glDrawArrays(GL_TRIANGLES, 0, m_cube_mesh->vertex_count() * m_instance_count);
        checkOpenGLError("After glDrawArrays");
        // std::printf("[CubeRenderer] glDrawArrays completed\n");

        glBindVertexArray(0);
        glUseProgram(0);
    }
} // namespace gfx