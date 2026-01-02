#include "cube_renderer.hpp"
#include "vertex.hpp"
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
            std::fprintf(stderr, "[CubeRenderer] Program linking error: %s\n", buf);
            glDeleteProgram(program);
            return 0;
        }
        return program;
    }

    bool CubeRenderer::init() {
        std::printf("[CubeRenderer] Initializing...\n");
        if (vertex_shader_path.empty() || fragment_shader_path.empty()) {
            std::fprintf(stderr, "[CubeRenderer] Failed to load shader sources: %s, %s\n", vertex_shader_path.c_str(), fragment_shader_path.c_str());
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
                stbi_image_free(data);
            } else {
                std::printf("[CubeRenderer] Failed to load texture at path: %s\n", texture_path);
                return false;
            }
        }

        if (m_program == 0) {
            std::printf("[CubeRenderer] Initialization failed.\n");
            return false;
        }

        // glBindBuffer(GL_ARRAY_BUFFER, 0);
        // glBindVertexArray(0);
        return true;
    }

    void CubeRenderer::setup_frame(const float* viewProj4x4, const glm::vec3& camPos) {
        glUseProgram(m_program);

        // get uniforms locations
        GLint vpLoc = glGetUniformLocation(m_program, "uViewProj");
        if (vpLoc >= 0) glUniformMatrix4fv(vpLoc, 1, GL_FALSE, viewProj4x4);

        // set camera position uniform
        GLint viewPosLoc = glGetUniformLocation(m_program, "uViewPos");
        if (viewPosLoc >= 0) glUniform3fv(viewPosLoc, 1, &camPos[0]);

        // set fog uniforms
        glUniform3f(glGetUniformLocation(m_program, "uFogColor"), 0.53f, 0.81f, 0.92f);
        glUniform1f(glGetUniformLocation(m_program, "uFogNear"), 150.0f); // start distance
        glUniform1f(glGetUniformLocation(m_program, "uFogFar"), 200.0f); // end distance

        // bind texture
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, glContext.textureID);
        glUniform1i(glGetUniformLocation(m_program, "uTexture"), 0);

        // texture
        glEnable(GL_FRAMEBUFFER_SRGB);
        glDisable(0x809D); // disable multisampling

        glDisable(GL_CULL_FACE);
        glDisable(GL_DEPTH_TEST);

        // depth and face culling
        // glEnable(GL_DEPTH_TEST);
        // glDepthFunc(GL_LESS);
        
        // culling
        // glEnable(GL_CULL_FACE);
        // glCullFace(GL_BACK);
        // glFrontFace(GL_CCW); // define front side as counter clockwise

        // glBindVertexArray(0);
        glUseProgram(0);
    }

    void CubeRenderer::update_chunk_mesh(
        ocm::Chunk& chunk,
        const std::vector<ChunkVertex>& vertices, 
        const std::vector<uint32_t>& indices
    ) {
        if (chunk.vao == 0) {
            glGenVertexArrays(1, &chunk.vao);
            glGenBuffers(1, &chunk.vbo);
            glGenBuffers(1, &chunk.ebo);
        }

        // Bind and upload data
        glBindVertexArray(chunk.vao);

        // transfer vertex data
        glBindBuffer(GL_ARRAY_BUFFER, chunk.vbo);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(ChunkVertex), vertices.data(), GL_STATIC_DRAW);

        // transfer index data
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, chunk.ebo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(uint32_t), indices.data(), GL_STATIC_DRAW);

        // set vertex attribute pointers
        GLsizei stride = 28;

        // aPos
        glEnableVertexAttribArray(0); // position
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)0);
        // aTex
        glEnableVertexAttribArray(1); // uv
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, stride, (void*)12);
        // aFaceID
        glEnableVertexAttribArray(2); // face index
        glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, stride, (void*)20);
        // aBlockID
        glEnableVertexAttribArray(3); // block ID
        glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, stride, (void*)24);

        chunk.indexCount = static_cast<uint32_t>(indices.size());
        // unbind VAO
        glBindVertexArray(0);

        // unbind other buffers
        // glBindBuffer(GL_ARRAY_BUFFER, 0);
        // glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    }

    void CubeRenderer::draw_chunk(const ocm::Chunk& chunk) {
        if (chunk.vao == 0 || chunk.indexCount == 0) return;

        glm::vec3 chunkPos(
            static_cast<float>(chunk.cx() * ocm::CHUNK_SIZE_X),
            0.0f,
            static_cast<float>(chunk.cz() * ocm::CHUNK_SIZE_Z)
        );

        GLint chunkPosLoc = glGetUniformLocation(m_program, "uChunkPos");
        glUniform3fv(chunkPosLoc, 1, &chunkPos[0]);

        glBindVertexArray(chunk.vao);
        glDrawElements(GL_TRIANGLES, chunk.indexCount, GL_UNSIGNED_INT, 0);

        // std::printf("[CubeRenderer] Draw Chunk at (%d, %d): %u indices\n", chunk.cx(), chunk.cz(), chunk.indexCount);
    }
} // namespace gfx