#pragma once

#include <vector>
#include <cstdint>
#include "../world/chunk.hpp"
#include "vertex.hpp"

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace gfx {
    class CubeRenderer {
        public:
            CubeRenderer();
            ~CubeRenderer();

            bool init();

            // フレーム開始時の共通設定
            void setup_frame(const float* viewProj4x4, const glm::vec3& camPos);

            // 特定のチャンクのメッシュ(VBO/VAO)を生成・更新
            void update_chunk_mesh(
                ocm::Chunk& chunk, 
                const std::vector<ChunkVertex>& vertices, 
                const std::vector<uint32_t>& indices
            );

            // 指定されたチャンクのVAOをバインドして描画
            void draw_chunk(const ocm::Chunk& chunk);

            GLuint program() const noexcept { return m_program; };
            GLuint textureArray() const noexcept { return m_textureArray; };

        private:
            GLuint m_program = 0;
            GLuint m_textureArray = 0;
            GLuint compile_shader(const char* source, GLenum shader_type);
            GLuint link_program(GLuint vertex_shader, GLuint fragment_shader);
    };
} // namespace gfx