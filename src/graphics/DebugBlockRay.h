#pragma once

#include <vector>
#include <glm/glm.hpp>
#include "../graphics/Shader.h"

class DebugBlockRay {
public:
    static void AddHitPoint(const glm::vec3& pos);
    static void Clear();
    static void Draw(Shader& shader);


private:
    static std::vector<glm::vec3> hitPoints;
};
