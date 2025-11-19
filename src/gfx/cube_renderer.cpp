#include "cube_renderer.hpp"
#include "cube_mesh.hpp"
#include <cstdio>
#include <cstring>
#include <vector>

static const char* kVertexSource = R"glsl(
#version 330 core
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTex;
layout(location = 3) in vec3 aInstancePos;

uniform mat4 uViewProj;

out vec3 vNormal;
out vec2 vTex;

void main() {
    // translate cube by instance position (assume instance pos is world coords)
    vec3 p = aPos + aInstancePos;
    gl_Position = uViewProj * vec4(p, 1.0);
    vNormal = aNormal;
    vTex = aTex;
}
)glsl";

static const char* kFragmentSource = R"glsl(
#version 330 core
in vec3 vNormal;
in vec2 vTex;
out vec4 FragColor;
void main() {
    // simple lambert-like shading with constant light dir
    vec3 lightDir = normalize(vec3(0.5, 1.0, 0.3));
    float diff = max(dot(normalize(vNormal), lightDir), 0.1);
    vec3 base = vec3(0.8, 0.7, 0.5); // color (can be replaced per-block by texture)
    FragColor = vec4(base * diff, 1.0);
}
)glsl";

namespace gfx {

    CubeRenderer::CubeRenderer() = default;

    CubeRenderer::~CubeRenderer() {
        if (m_instance_vbo) glDeleteBuffers(1, &m_instance_vbo);
        if (m_program) glDeleteProgram(m_program);
        delete m_cube_mesh;
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
            std::fprintf(stderr, "Shader compilation error: %s\n", buf);
            glDeleteShader(shader);
            return 0;
        }
        return shader;
    }

    GLuint CubeRenderer::link_program(GLuint vertex_shader, GLuint fragment_shader) {
        GLuint program = glCreateProgram();
        glAttachShader(program, vertex_shader);
        glAttachShader(program, fragment_shader);
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
        m_cube_mesh = new CubeMesh();

        // compile shaders
        GLuint vertex_shader = compile_shader(kVertexSource, GL_VERTEX_SHADER);
        if (!vertex_shader) return false;
        GLuint fragment_shader = compile_shader(kFragmentSource, GL_FRAGMENT_SHADER);
        if (!fragment_shader) {
            glDeleteShader(vertex_shader);
            return false;
        }
        m_program = link_program(vertex_shader, fragment_shader);
        glDeleteShader(vertex_shader);
        glDeleteShader(fragment_shader);
        if (!m_program) return false;

        // create instance VBO
        glGenBuffers(1, &m_instance_vbo);

        // bind cube VAO and enable instanced attrib
        glBindVertexArray(m_cube_mesh->vao());
        glBindBuffer(GL_ARRAY_BUFFER, m_instance_vbo);

        glEnableVertexAttribArray(3); // aInstancePos
        glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
        glVertexAttribDivisor(3, 1); // advance per instance
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);

        return true;
    }

    void CubeRenderer::update_instances(const std::vector<Vec3f>& positions) {
        glBindBuffer(GL_ARRAY_BUFFER, m_instance_vbo);
        if (!positions.empty()) {
            glBufferData(GL_ARRAY_BUFFER, positions.size() * sizeof(Vec3f), positions.data(), GL_STATIC_DRAW);
        } else {
            // keep buffer empty
            glBufferData(GL_ARRAY_BUFFER, 0, nullptr, GL_STATIC_DRAW);
        }
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        m_instance_count = static_cast<GLuint>(positions.size());
    }

    void CubeRenderer::draw(const float* viewProj4x4) {
        if (!m_program || !m_cube_mesh) return;
        glUseProgram(m_program);
        GLint loc = glGetUniformLocation(m_program, "uViewProj");
        if (loc >= 0) glUniformMatrix4fv((GLint)loc, 1, GL_FALSE, viewProj4x4);
        
        glBindVertexArray(m_cube_mesh->vao());
        glDrawArraysInstanced(GL_TRIANGLES, 0, m_cube_mesh->vertex_count(), m_instance_count);
        glBindVertexArray(0);
        glUseProgram(0);
    }
} // namespace gfx