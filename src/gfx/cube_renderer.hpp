#pragma once

#include <vector>
#include <cstdint>
#include <glad/glad.h>
#include "cube_mesh.hpp"

namespace gfx {
    struct Vec3f { float x, y, z; };

    class CubeRenderer {
        public:
            CubeRenderer();
            ~CubeRenderer();

            bool init();

            void update_instances(const std::vector<Vec3f>& positions);

            void draw(const float* viewProj4x4);

            GLuint program() const noexcept { return m_program; }

        private:
            GLuint m_program = 0;
            GLuint m_instance_vbo = 0;
            gfx::CubeMesh* m_cube_mesh = nullptr;
            GLuint m_instance_count = 0;

            GLuint compile_shader(const char* source, GLenum shader_type);
            GLuint link_program(GLuint vertex_shader, GLuint fragment_shader);
    };
} // namespace gfx