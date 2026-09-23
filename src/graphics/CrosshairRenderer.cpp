#include "CrosshairRenderer.h"
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/gtc/type_ptr.hpp>

CrosshairRenderer::CrosshairRenderer() {
    float crosshairVertices[] = {
        // Vertical line
        0.0f, -0.03f,
        0.0f,  0.03f,
        // Horizontal line
        -0.02f, 0.0f,
         0.02f, 0.0f
    };

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);
    
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(crosshairVertices), crosshairVertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glBindVertexArray(0);
}

CrosshairRenderer::~CrosshairRenderer() {
    glDeleteBuffers(1, &VBO);
    glDeleteVertexArrays(1, &VAO);
}

void CrosshairRenderer::Draw(Shader& shader) {
    glDisable(GL_DEPTH_TEST); // ensure crosshair is always visible

    shader.Bind();

    glBindVertexArray(VAO);
    glDrawArrays(GL_LINES, 0, 4);
    glBindVertexArray(0);

    shader.Unbind();

    glEnable(GL_DEPTH_TEST);
}
