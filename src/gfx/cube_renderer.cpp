#include "cube_renderer.hpp"
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

namespace gfx {
    CubeRenderer::CubeRenderer() = default;

    CubeRenderer::~CubeRenderer() {
        if (m_vao) glDeleteVertexArrays(1, &m_vao);
        if (m_vbo) glDeleteBuffers(1, &m_vbo);
        if (m_ebo) glDeleteBuffers(1, &m_ebo);
        if (m_program) glDeleteProgram(m_program);
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
            std::fprintf(stderr, "\n[CubeRenderer] Shader compilation error: %s", buf);
            std::printf("[CubeRenderer] Source: %s\n\n", shader_type == GL_VERTEX_SHADER ? "Vertex Shader; " : "Fragment Shader; ");
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
        glBindAttribLocation(program, 5, "aBlockID");

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
        std::printf("[CubeRenderer] Generating VAO, VBO, EBO...\n");
        glGenVertexArrays(1, &m_vao);
        glGenBuffers(1, &m_vbo);
        glGenBuffers(1, &m_ebo);
        std::printf("[CubeRenderer] Generated Chunk Buffers: VAO=%u, VBO=%u, EBO=%u\n", m_vao, m_vbo, m_ebo);

        // Texture Loading using stbi
        {
            glGenTextures(1, &glContext.textureID);
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
                stbi_image_free(data);
            } else {
                std::printf("[CubeRenderer] Failed to load texture at path: %s\n", texture_path);
                return false;
            }

        }
        // glBindBuffer(GL_ARRAY_BUFFER, 0);
        // glBindVertexArray(0);
        return true;
    }

    void CubeRenderer::update_mesh(const std::vector<ChunkVertex>& vertices, const std::vector<uint32_t>& indices) {
        if (vertices.empty() || indices.empty()) {
            m_index_count = 0;
            return;
        }

        m_index_count = static_cast<uint32_t>(indices.size());

        // Bind and upload data
        glBindVertexArray(m_vao);

        // transfer vertex data
        glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(ChunkVertex), vertices.data(), GL_DYNAMIC_DRAW);

        // transfer index data
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ebo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(uint32_t), indices.data(), GL_DYNAMIC_DRAW);

        // set vertex attribute pointers
        GLsizei stride = sizeof(ChunkVertex);

        // aPos
        glEnableVertexAttribArray(0); // position
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)0);

        // aTex
        glEnableVertexAttribArray(2); // uv
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, stride, (void*)(3 * sizeof(float)));

        // aFaceID
        glEnableVertexAttribArray(4); // face index
        glVertexAttribPointer(4, 1, GL_FLOAT, GL_FALSE, stride, (void*)(5 * sizeof(float)));

        glEnableVertexAttribArray(5); // block ID
        glVertexAttribPointer(5, 1, GL_FLOAT, GL_FALSE, stride, (void*)(6 * sizeof(float)));

        // unbind VAO
        glBindVertexArray(0);

        // unbind other buffers
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    }

    void CubeRenderer::draw(const float* viewProj4x4) {
        if (m_index_count == 0) return;

        glUseProgram(m_program);

        // get uniforms locations
        GLint vpLoc = glGetUniformLocation(m_program, "uViewProj");
        if (vpLoc >= 0) glUniformMatrix4fv(vpLoc, 1, GL_FALSE, viewProj4x4);

        // GLint blockTypeLoc = glGetUniformLocation(m_program, "uBlockType");
        // if (blockTypeLoc >= 0) glUniform1i(blockTypeLoc, 3); // default block type

        // bind texture
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, glContext.textureID);

        // unbind VAO and draw
        glBindVertexArray(m_vao);

        glDrawElements(GL_TRIANGLES, m_index_count, GL_UNSIGNED_INT, 0);

        glBindVertexArray(0);
        glUseProgram(0);
    }
} // namespace gfx