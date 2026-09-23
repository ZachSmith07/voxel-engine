#include "Shader.h"
#include <fstream>
#include <sstream>
#include <iostream>

Shader::Shader(const char *vertexPath, const char *fragmentPath)
{
    std::ifstream vFile(vertexPath);
    if (!vFile.is_open())
    {
        std::cerr << "Error: Could not open vertex shader file: " << vertexPath << std::endl;
        // Handle error (e.g., exit, throw exception)
    }
    std::ifstream fFile(fragmentPath);
    if (!fFile.is_open())
    {
        std::cerr << "Error: Could not open fragment shader file: " << fragmentPath << std::endl;
        // Handle error
    }
    // std::ifstream fFile(fragmentPath);

    std::stringstream vStream, fStream;
    vStream << vFile.rdbuf();
    fStream << fFile.rdbuf();

    std::string vertexCode = vStream.str();

    // std::cout << "=== Vertex Shader Source ===\n"
    //           << vertexCode << "\n";

    std::string fragmentCode = fStream.str();

    const char *vSrc = vertexCode.c_str();
    const char *fSrc = fragmentCode.c_str();

    // === Compile Vertex Shader ===
    unsigned int vertex = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex, 1, &vSrc, nullptr);
    glCompileShader(vertex);

    int success;
    char infoLog[512];
    glGetShaderiv(vertex, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(vertex, 512, NULL, infoLog);
        std::cerr << "VERTEX SHADER COMPILATION FAILED:\n"
                  << infoLog << std::endl;
    }

    // === Compile Fragment Shader ===
    unsigned int fragment = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment, 1, &fSrc, nullptr);
    glCompileShader(fragment);

    glGetShaderiv(fragment, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(fragment, 512, NULL, infoLog);
        std::cerr << "FRAGMENT SHADER COMPILATION FAILED:\n"
                  << infoLog << std::endl;
    }

    // === Link Shader Program ===
    ID = glCreateProgram();
    glAttachShader(ID, vertex);
    glAttachShader(ID, fragment);
    glLinkProgram(ID);

    glGetProgramiv(ID, GL_LINK_STATUS, &success);
    if (!success)
    {
        glGetProgramInfoLog(ID, 512, NULL, infoLog);
        std::cerr << "SHADER PROGRAM LINKING FAILED:\n"
                  << infoLog << std::endl;
    }

    glDeleteShader(vertex);
    glDeleteShader(fragment);
}

Shader::~Shader()
{
    glDeleteProgram(ID);
}

// Bind the shader
void Shader::Bind() const
{
    glUseProgram(ID);
}

// Unbind the shader
void Shader::Unbind() const
{
    glUseProgram(0);
}

// Optional uniform setters
int Shader::GetUniformLocation(const std::string &name)
{
    return glGetUniformLocation(ID, name.c_str());
}

void Shader::SetUniformMat4(const std::string &name, const float *value)
{
    glUniformMatrix4fv(GetUniformLocation(name), 1, GL_FALSE, value);
}

void Shader::SetUniform1i(const std::string &name, int value)
{
    glUniform1i(GetUniformLocation(name), value);
}

void Shader::SetUniform1f(const std::string &name, float value)
{
    glUniform1f(GetUniformLocation(name), value);
}

void Shader::SetUniform3f(const std::string& name, float v0, float v1, float v2)
{
    glUniform3f(this->GetUniformLocation(name), v0, v1, v2);
}


void Shader::SetUniformVec3(const std::string& name, const glm::vec3& value)
{
    glUniform3f(GetUniformLocation(name), value.x, value.y, value.z);
}

void Shader::SetUniformVec3(const std::string& name, float v0, float v1, float v2)
{
    glUniform3f(GetUniformLocation(name), v0, v1, v2);
}