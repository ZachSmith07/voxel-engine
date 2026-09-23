#include "DebugRenderer.h"
#include <GL/glew.h>
#include <GLFW/glfw3.h>

void DebugRenderer::Init() {
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * 2 * 1000, nullptr, GL_DYNAMIC_DRAW); // up to 1000 lines

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);
    glEnableVertexAttribArray(0);
}

void DebugRenderer::DrawLine(const glm::vec3& start, const glm::vec3& end) {
    lines.push_back(start);
    lines.push_back(end);
}

void DebugRenderer::Render(const glm::mat4& viewProj) {
    if (lines.empty()) return;

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(glm::vec3) * lines.size(), lines.data());

    // Setup shader
    static unsigned int shader = 0;
    if (!shader) {
        // Simple passthrough shader
        const char* vs = R"(
        #version 330 core
        layout(location = 0) in vec3 aPos;
        uniform mat4 uVP;
        void main() { gl_Position = uVP * vec4(aPos, 1.0); }
        )";

        const char* fs = R"(
        #version 330 core
        out vec4 FragColor;
        void main() { FragColor = vec4(1.0, 0.2, 0.2, 1.0); }
        )";

        unsigned int vsID = glCreateShader(GL_VERTEX_SHADER);
        unsigned int fsID = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(vsID, 1, &vs, nullptr); glCompileShader(vsID);
        glShaderSource(fsID, 1, &fs, nullptr); glCompileShader(fsID);
        shader = glCreateProgram();
        glAttachShader(shader, vsID);
        glAttachShader(shader, fsID);
        glLinkProgram(shader);
        glDeleteShader(vsID);
        glDeleteShader(fsID);
    }

    glUseProgram(shader);
    glUniformMatrix4fv(glGetUniformLocation(shader, "uVP"), 1, GL_FALSE, &viewProj[0][0]);
    glDrawArrays(GL_LINES, 0, lines.size());
}

void DebugRenderer::Clear() {
    lines.clear();
}
