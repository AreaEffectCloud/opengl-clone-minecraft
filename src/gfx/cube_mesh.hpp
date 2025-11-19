#pragma once

// 単一キューブの頂点データ
#include <cstdint>
#include <glad/glad.h>

namespace gfx {

    class CubeMesh {
        public:
            CubeMesh();
            ~CubeMesh();

            GLuint vao() const noexcept { return m_vao; }
            GLsizei vertex_count() const noexcept { return m_vertex_count; }
        
        private:
            GLuint m_vao = 0;
            GLuint m_vbo = 0;
            GLsizei m_vertex_count = 0;
    };
} // namespace gfx