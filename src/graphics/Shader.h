#ifndef SHADER_H
#define SHADER_H

#include <string>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

class Shader
{
public:
    Shader(const char* vertexPath, const char* fragmentPath);
    ~Shader();

    void Bind() const;
    void Unbind() const;

    // Uniform setters
    int  GetUniformLocation(const std::string& name);
    void SetUniformMat4(const std::string& name, const float* value);
    void SetUniform1i(const std::string& name, int value);
    void SetUniform1f(const std::string& name, float value);
    void SetUniform3f(const std::string& name, float v0, float v1, float v2);
    void SetUniformVec3(const std::string& name, const glm::vec3& value);
    void SetUniformVec3(const std::string& name, float v0, float v1, float v2);


private:
    unsigned int ID;
};

#endif
