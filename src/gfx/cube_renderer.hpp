#pragma once

#include <vector>
#include <cstdint>
#include <glad/glad.h>

namespace gfx {
    struct ChunkVertex { 
        float x, y, z; 
        float u, v; 
        float faceID;
        float blockID;
    };

    class CubeRenderer {
        public:
            CubeRenderer();
            ~CubeRenderer();

            bool init();

            // 構築されたメッシュをGPUに転送
            void update_mesh(const std::vector<ChunkVertex>& vertices, const std::vector<uint32_t>& indices);

            void draw(const float* viewProj4x4);

            GLuint program() const noexcept { return m_program; };

        private:
            GLuint m_program = 0;
            GLuint m_vao = 0;
            GLuint m_vbo = 0;
            GLuint m_ebo = 0;
            uint32_t m_index_count = 0;

            // GLuint m_instance_vbo = 0;
            // GLuint m_instance_count = 0;

            GLuint compile_shader(const char* source, GLenum shader_type);
            GLuint link_program(GLuint vertex_shader, GLuint fragment_shader);
    };
} // namespace gfx