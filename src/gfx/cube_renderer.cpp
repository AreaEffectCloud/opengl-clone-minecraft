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

// Textures
std::vector<std::string> texturePaths = {
    "../src/assets/textures/block_placeholder.png",
    "../src/assets/textures/dirt.png",
    "../src/assets/textures/grass_block_top.png",
    "../src/assets/textures/grass_block_side.png",
    "../src/assets/textures/sand.png",
    "../src/assets/textures/stone.png",
    "../src/assets/textures/cobblestone.png",
    "../src/assets/textures/water.png",
    "../src/assets/textures/coal_ore.png",
    "../src/assets/textures/iron_ore.png",
};

namespace gfx {
    CubeRenderer::CubeRenderer() = default;

    CubeRenderer::~CubeRenderer() {
        if (m_program) glDeleteProgram(m_program);
        if (m_textureArray) glDeleteTextures(1, &m_textureArray);
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
            glActiveTexture(GL_TEXTURE0);
            glGenTextures(1, &m_textureArray);
            glBindTexture(GL_TEXTURE_2D_ARRAY, m_textureArray);

            int width, height, nrChannels;
            unsigned char* data = stbi_load(texturePaths[0].c_str(), &width, &height, &nrChannels, 4);
            if (data) {
                std::printf("[CubeRenderer / Texture] Data sent GPU: Size: %dx%d, Channels: %d, \nSource: %s\n", width, height, nrChannels, texturePaths[0].c_str());
                stbi_image_free(data);
            } else {
                std::printf("[CubeRenderer] Failed to load texture at path: %s\n", texturePaths[0].c_str());
                return false;
            }

            glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_RGBA8, width, height, static_cast<GLsizei>(texturePaths.size()), 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
            glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

            for (size_t i = 0; i < texturePaths.size(); i++) {
                unsigned char* subData = stbi_load(texturePaths[i].c_str(), &width, &height, &nrChannels, 4);
                if (subData) {
                    glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, i, width, height, 1, GL_RGBA, GL_UNSIGNED_BYTE, subData);
                    stbi_image_free(subData);
                } else {
                    std::printf("[CubeRenderer] Failed to load texture at path: %s\n", texturePaths[i].c_str());
                    return false;
                }
            }

            glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR);
            glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glGenerateMipmap(GL_TEXTURE_2D_ARRAY);
        }

        if (m_program == 0) {
            std::printf("[CubeRenderer] Initialization failed.\n");
            return false;
        }

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
        glUniform1f(glGetUniformLocation(m_program, "uFogNear"), 200.0f); // start distance
        glUniform1f(glGetUniformLocation(m_program, "uFogFar"), 300.0f); // end distance

        glm::vec3 sunDir = glm::normalize(glm::vec3(0.4f, 1.0f, 0.5f));
        glUniform3fv(glGetUniformLocation(m_program, "uSunDir"), 1, &sunDir[0]);

        // bind texture
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D_ARRAY, m_textureArray);
        glUniform1i(glGetUniformLocation(m_program, "uTextureArray"), 0);

        // High Contrast
        // glEnable(GL_FRAMEBUFFER_SRGB);
        // glDisable(0x809D); // disable multisampling

        // depth and face culling
        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LESS);
        
        // culling
        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);
        glFrontFace(GL_CCW); // define front side as counter clockwise

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    }

    void CubeRenderer::update_chunk_mesh(ocm::Chunk& chunk, const MeshData& data) {
        // 不透明メッシュ
        update_buffer(chunk.vao, chunk.vbo, chunk.ebo, data.opaque_vertices, data.opaque_indices);
        chunk.indexCount = static_cast<int>(data.opaque_indices.size());

        // 透明メッシュ
        update_buffer(chunk.trans_vao, chunk.trans_vbo, chunk.trans_ebo, data.trans_vertices, data.trans_indices);
        chunk.trans_indexCount = static_cast<int>(data.trans_indices.size());
    }

    void CubeRenderer::update_buffer(
        uint32_t& vao, uint32_t& vbo, uint32_t& ebo,
        const std::vector<ChunkVertex>& vertices, 
        const std::vector<uint32_t>& indices
    ) {
        if (vertices.empty()) return;
        if (vao == 0) {
            glGenVertexArrays(1, &vao);
            glGenBuffers(1, &vbo);
            glGenBuffers(1, &ebo);
        }

        // Bind and upload data
        glBindVertexArray(vao);
        // transfer vertex data
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(ChunkVertex), vertices.data(), GL_STATIC_DRAW);

        // transfer index data
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(uint32_t), indices.data(), GL_STATIC_DRAW);

        // set vertex attribute pointers
        GLsizei stride = sizeof(ChunkVertex);

        // aPos
        glEnableVertexAttribArray(0); // position
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)0);
        // aTex
        glEnableVertexAttribArray(1); // uv
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, stride, (void*)(3 * sizeof(float)));
        // aFaceID
        glEnableVertexAttribArray(2); // face index
        glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, stride, (void*)(5 * sizeof(float)));
        // aBlockID
        glEnableVertexAttribArray(3); // block ID
        glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, stride, (void*)(6 * sizeof(float)));
        
        // unbind VAO
        glBindVertexArray(0);
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
    }

    void CubeRenderer::draw_chunk_transparent(const ocm::Chunk& chunk) {
        if (chunk.trans_vao == 0 || chunk.trans_indexCount == 0) return;

        glm::vec3 chunkPos(
            static_cast<float>(chunk.cx() * ocm::CHUNK_SIZE_X),
            0.0f,
            static_cast<float>(chunk.cz() * ocm::CHUNK_SIZE_Z)
        );

        GLint chunkPosLoc = glGetUniformLocation(m_program, "uChunkPos");
        glUniform3fv(chunkPosLoc, 1, &chunkPos[0]);

        // 水用の VAO をバインド
        glBindVertexArray(chunk.trans_vao);
        // EBO を使用して描画
        glDrawElements(GL_TRIANGLES, chunk.trans_indexCount, GL_UNSIGNED_INT, 0);
        // 水を裏面からも見えるように
        glDisable(GL_CULL_FACE);

        // glBindVertexArray(0);
    }
} // namespace gfx