#include "cube_mesh.hpp"
#include <vector>

namespace gfx {
    CubeMesh::CubeMesh() {
        // cube vertices
        float vertices[] = {
            // Position (x, y, z)  //UV (u, v)
            // front face (+Z)
            -0.5f, -0.5f,  0.5f,   0.0f, 0.0f,
             0.5f, -0.5f,  0.5f,   1.0f, 0.0f,
             0.5f,  0.5f,  0.5f,   1.0f, 1.0f,
             0.5f,  0.5f,  0.5f,   1.0f, 1.0f,
            -0.5f,  0.5f,  0.5f,   0.0f, 1.0f,
            -0.5f, -0.5f,  0.5f,   0.0f, 0.0f,
            // back face (-Z)
            -0.5f, -0.5f, -0.5f,   1.0f, 0.0f,
             0.5f, -0.5f, -0.5f,   0.0f, 0.0f,
             0.5f,  0.5f, -0.5f,   0.0f, 1.0f,
             0.5f,  0.5f, -0.5f,   0.0f, 1.0f,
            -0.5f,  0.5f, -0.5f,   1.0f, 1.0f,
            -0.5f, -0.5f, -0.5f,   1.0f, 0.0f,
            // left face (-X)
            -0.5f,  0.5f,  0.5f,   1.0f, 0.0f,
            -0.5f,  0.5f, -0.5f,   1.0f, 1.0f,
            -0.5f, -0.5f, -0.5f,   0.0f, 1.0f,
            -0.5f, -0.5f, -0.5f,   0.0f, 1.0f,
            -0.5f, -0.5f,  0.5f,   0.0f, 0.0f,
            -0.5f,  0.5f,  0.5f,   1.0f, 0.0f,
            // right face (+X)
             0.5f,  0.5f,  0.5f,   0.0f, 0.0f,
             0.5f,  0.5f, -0.5f,   0.0f, 1.0f,
             0.5f, -0.5f, -0.5f,   1.0f, 1.0f,
             0.5f, -0.5f, -0.5f,   1.0f, 1.0f,
             0.5f, -0.5f,  0.5f,   1.0f, 0.0f,
             0.5f,  0.5f,  0.5f,   0.0f, 0.0f,
            // top face (+Y)
            -0.5f,  0.5f, -0.5f,   0.0f, 1.0f,
             0.5f,  0.5f, -0.5f,   1.0f, 1.0f,
             0.5f,  0.5f,  0.5f,   1.0f, 0.0f,
             0.5f,  0.5f,  0.5f,   1.0f, 0.0f,
            -0.5f,  0.5f,  0.5f,   0.0f, 0.0f,
            -0.5f,  0.5f, -0.5f,   0.0f, 1.0f,
            // bottom face (-Y)
            -0.5f, -0.5f, -0.5f,   1.0f, 1.0f,
             0.5f, -0.5f, -0.5f,   0.0f, 1.0f,
             0.5f, -0.5f,  0.5f,   0.0f, 0.0f,
             0.5f, -0.5f,  0.5f,   0.0f, 0.0f,
            -0.5f, -0.5f,  0.5f,   1.0f, 0.0f,
            -0.5f, -0.5f, -0.5f,   1.0f, 1.0f
        };

        unsigned int indices[] = {
            0, 1, 2,  2, 3, 0,       // Front
            4, 5, 6,  6, 7, 4,       // Back
            8, 9, 10, 10, 11, 8,     // Top
            12, 13, 14, 14, 15, 12,  // Bottom
            16, 17, 18, 18, 19, 16,  // Right
            20, 21, 22, 22, 23, 20   // Left
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

        // aPos (location = 0)
        glEnableVertexAttribArray(0); // position
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);

        // aNormal
        // glEnableVertexAttribArray(1); // normal
        // glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
        glDisableVertexAttribArray(1); // not used

        // aTex
        glEnableVertexAttribArray(2); // uv
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));

        glBindVertexArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }

    CubeMesh::~CubeMesh() {
        if (m_vbo) glDeleteBuffers(1, &m_vbo);
        if (m_vao) glDeleteVertexArrays(1, &m_vao);
        // if (m_ebo) glDeleteBuffers(1, &m_ebo);
    }
} // namespace gfx