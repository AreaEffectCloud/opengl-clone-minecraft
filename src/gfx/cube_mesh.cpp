#include "cube_mesh.hpp"
#include <vector>

namespace gfx {
    CubeMesh::CubeMesh() {
        // cube vertices
        float vertices[] = {
            // Position (x, y, z)  //UV (u, v)
            
            // Front face (Z+)
            -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,  0.0f,
             0.5f, -0.5f,  0.5f,  1.0f, 0.0f,  0.0f,
             0.5f,  0.5f,  0.5f,  1.0f, 1.0f,  0.0f,
            -0.5f,  0.5f,  0.5f,  0.0f, 1.0f,  0.0f,

            // Back face (Z-)
            -0.5f, -0.5f, -0.5f,  1.0f, 0.0f,  0.0f,
             0.5f, -0.5f, -0.5f,  0.0f, 0.0f,  0.0f,
             0.5f,  0.5f, -0.5f,  0.0f, 1.0f,  0.0f,
            -0.5f,  0.5f, -0.5f,  1.0f, 1.0f,  0.0f,

            // Top face (Y+)
            -0.5f,  0.5f, -0.5f,  0.0f, 1.0f,  1.0f,
             0.5f,  0.5f, -0.5f,  1.0f, 1.0f,  1.0f,
             0.5f,  0.5f,  0.5f,  1.0f, 0.0f,  1.0f,
            -0.5f,  0.5f,  0.5f,  0.0f, 0.0f,  1.0f,

            // Bottom face (Y-)
            -0.5f, -0.5f, -0.5f,  1.0f, 1.0f,  2.0f,
             0.5f, -0.5f, -0.5f,  0.0f, 1.0f,  2.0f,
             0.5f, -0.5f,  0.5f,  0.0f, 0.0f,  2.0f,
            -0.5f, -0.5f,  0.5f,  1.0f, 0.0f,  2.0f,

            // Right face (X+)
            0.5f, -0.5f, -0.5f,  1.0f, 0.0f,   0.0f,
            0.5f,  0.5f, -0.5f,  1.0f, 1.0f,   0.0f,
            0.5f,  0.5f,  0.5f,  0.0f, 1.0f,   0.0f,
            0.5f, -0.5f,  0.5f,  0.0f, 0.0f,   0.0f,

            // Left face (X-)
            -0.5f, -0.5f, -0.5f,  0.0f, 0.0f,  0.0f,
            -0.5f,  0.5f, -0.5f,  0.0f, 1.0f,  0.0f,
            -0.5f,  0.5f,  0.5f,  1.0f, 1.0f,  0.0f,
            -0.5f, -0.5f,  0.5f,  1.0f, 0.0f,  0.0f
        };

        unsigned int indices[] = {
            // Front face (Z+) : 0-1-2-3
            0, 1, 2,     2, 3, 0,
            // Back face (Z-) : 4-5-6-7 (外側から見て反時計回り)
            5, 4, 7,     7, 6, 5,
            // Top face (Y+) : 8-9-10-11
            8, 11, 10,   10, 9, 8,
            // Bottom face (Y-) : 12-13-14-15
            12, 13, 14,  14, 15, 12,
            // Right face (X+) : 16-17-18-19
            19, 16, 17,  17, 18, 19,
            // Left face (X-) : 20-21-22-23
            20, 23, 22,  22, 21, 20
        };

        glGenVertexArrays(1, &m_vao);
        glGenBuffers(1, &m_vbo);
        glGenBuffers(1, &m_ebo);

        glBindVertexArray(m_vao);

        // Bind and set VBO data
        glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

        // Bind and set EBO data
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ebo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

        // vertices は 1頂点あたり 6要素 (x, y, z, u, v, face)
        GLsizei stride = 6 * sizeof(float);

        // aPos (location = 0)
        glEnableVertexAttribArray(0); // position
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)0);

        // glDisableVertexAttribArray(1); // normal (not used)

        // aTex (location = 2)
        glEnableVertexAttribArray(2); // uv
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, stride, (void*)(3 * sizeof(float)));

        glEnableVertexAttribArray(4); // face index
        glVertexAttribPointer(4, 1, GL_FLOAT, GL_FALSE, stride, (void*)(5 * sizeof(float)));

        glBindVertexArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }

    CubeMesh::~CubeMesh() {
        if (m_vbo) glDeleteBuffers(1, &m_vbo);
        if (m_vao) glDeleteVertexArrays(1, &m_vao);
        if (m_ebo) glDeleteBuffers(1, &m_ebo);
    }
} // namespace gfx