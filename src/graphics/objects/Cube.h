#pragma once

#include <glm/glm.hpp>
#include <string>
#include "../Shader.h"

class Cube {
public:
    Cube(glm::vec3 pos, int blockID, glm::vec3 scl = glm::vec3(1.0f));
    void Draw(Shader& shader) const;

private:
    glm::vec3 position;
    glm::vec3 scale;
    unsigned int VAO, VBO;
    float vertexData[12 * 36 * 3]; // 12 rods × 36 vertices per rod × 3 floats (x,y,z)
 // 6 faces * 6 vertices * (3 position + 2 UV)

    void setupMesh();
    void bakeRodData();
    
};
