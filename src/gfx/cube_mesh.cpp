#include "cube_mesh.hpp"
#include <vector>

namespace gfx {
    CubeMesh::CubeMesh() {
         // cube vertices: pos(x,y,z), normal(x,y,z), uv(u,v)
        float vertices[] = {
            // front face (+Z)
            -0.5f, -0.5f,  0.5f,  0,0,1,  0.0f, 0.0f,
             0.5f, -0.5f,  0.5f,  0,0,1,  1.0f, 0.0f,
             0.5f,  0.5f,  0.5f,  0,0,1,  1.0f, 1.0f,
             0.5f,  0.5f,  0.5f,  0,0,1,  1.0f, 1.0f,
            -0.5f,  0.5f,  0.5f,  0,0,1,  0.0f, 1.0f,
            -0.5f, -0.5f,  0.5f,  0,0,1,  0.0f, 0.0f,
            // back face (-Z)
            -0.5f, -0.5f, -0.5f,  0,0,-1,  1.0f, 0.0f,
             0.5f, -0.5f, -0.5f,  0,0,-1,  0.0f, 0.0f,
             0.5f,  0.5f, -0.5f,  0,0,-1,  0.0f, 1.0f,
             0.5f,  0.5f, -0.5f,  0,0,-1,  0.0f, 1.0f,
            -0.5f,  0.5f, -0.5f,  0,0,-1,  1.0f, 1.0f,
            -0.5f, -0.5f, -0.5f,  0,0,-1,  1.0f, 0.0f,
            // left face (-X)
            -0.5f,  0.5f,  0.5f, -1,0,0,  1.0f, 0.0f,
            -0.5f,  0.5f, -0.5f, -1,0,0,  1.0f, 1.0f,
            -0.5f, -0.5f, -0.5f, -1,0,0,  0.0f, 1.0f,
            -0.5f, -0.5f, -0.5f, -1,0,0,  0.0f, 1.0f,
            -0.5f, -0.5f,  0.5f, -1,0,0,  0.0f, 0.0f,
            -0.5f,  0.5f,  0.5f, -1,0,0,  1.0f, 0.0f,
            // right face (+X)
             0.5f,  0.5f,  0.5f,  1,0,0,  0.0f, 0.0f,
             0.5f,  0.5f, -0.5f,  1,0,0,  0.0f, 1.0f,
             0.5f, -0.5f, -0.5f,  1,0,0,  1.0f, 1.0f,
             0.5f, -0.5f, -0.5f,  1,0,0,  1.0f, 1.0f,
             0.5f, -0.5f,  0.5f,  1,0,0,  1.0f, 0.0f,
             0.5f,  0.5f,  0.5f,  1,0,0,  0.0f, 0.0f,
            // top face (+Y)
            -0.5f,  0.5f, -0.5f,  0,1,0,  0.0f, 1.0f,
             0.5f,  0.5f, -0.5f,  0,1,0,  1.0f, 1.0f,
             0.5f,  0.5f,  0.5f,  0,1,0,  1.0f, 0.0f,
             0.5f,  0.5f,  0.5f,  0,1,0,  1.0f, 0.0f,
            -0.5f,  0.5f,  0.5f,  0,1,0,  0.0f, 0.0f,
            -0.5f,  0.5f, -0.5f,  0,1,0,  0.0f, 1.0f,
            // bottom face (-Y)
            -0.5f, -0.5f, -0.5f,  0,-1,0,  1.0f, 1.0f,
             0.5f, -0.5f, -0.5f,  0,-1,0,  0.0f, 1.0f,
             0.5f, -0.5f,  0.5f,  0,-1,0,  0.0f, 0.0f,
             0.5f, -0.5f,  0.5f,  0,-1,0,  0.0f, 0.0f,
            -0.5f, -0.5f,  0.5f,  0,-1,0,  1.0f, 0.0f,
            -0.5f, -0.5f, -0.5f,  0,-1,0,  1.0f, 1.0f
        };

        m_vertex_count = 36;

        glGenVertexArrays(1, &m_vao);
        glGenBuffers(1, &m_vbo);

        glBindVertexArray(m_vao);
        glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

        glEnableVertexAttribArray(0); // position
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);

        glEnableVertexAttribArray(1); // normal
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));

        glEnableVertexAttribArray(2); // uv
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));

        glBindVertexArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }

    CubeMesh::~CubeMesh() {
        if (m_vbo) glDeleteBuffers(1, &m_vbo);
        if (m_vao) glDeleteVertexArrays(1, &m_vao);
    }
} // namespace gfx