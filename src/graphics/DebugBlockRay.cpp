#include "DebugBlockRay.h"
#include "Shader.h"
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/gtc/matrix_transform.hpp>

std::vector<glm::vec3> DebugBlockRay::hitPoints;

void DebugBlockRay::AddHitPoint(const glm::vec3& pos) {
    hitPoints.push_back(pos);
}

void DebugBlockRay::Clear() {
    hitPoints.clear();
}

void DebugBlockRay::Draw(Shader& shader) {
    glBindVertexArray(0); // just in case

    float size = 0.05f;
    for (const auto& p : hitPoints) {
        glm::mat4 model = glm::translate(glm::mat4(1.0f), p);
        model = glm::scale(model, glm::vec3(size));

        shader.SetUniformMat4("u_Model", &model[0][0]);

        // Render a cube at `p` (must have a cube VAO bound already)
        glDrawArrays(GL_TRIANGLES, 0, 36); // assuming bound unit cube VBO
    }
}
