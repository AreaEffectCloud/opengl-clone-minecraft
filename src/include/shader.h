#ifndef SHADER_H
#define SHADER_H

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <string>

class Shader {
    public:
        unsigned int ID;
        Shader(const char* vertex_shader_path, const char* fragment_shader_path);
        void use();
        void setMat4(const std::string& name, const glm::mat4& mat) const;
        void setInt(const std::string& name, int value) const;

    private:
        void checkCompileErrors(unsigned int shader, std::string type);
};

#endif