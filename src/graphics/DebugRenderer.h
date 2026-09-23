#pragma once

#include <glm/glm.hpp>
#include <vector>

class DebugRenderer {
public:
    void Init();
    void DrawLine(const glm::vec3& start, const glm::vec3& end);
    void Render(const glm::mat4& viewProj);
    void Clear();

private:
    std::vector<glm::vec3> lines;
    unsigned int VAO, VBO;
};
