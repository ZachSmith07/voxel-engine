#include "Cube.h"
#include "BlockDatabase.h"
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

extern unsigned int atlasTexture;

Cube::Cube(glm::vec3 pos, int blockID, glm::vec3 scl)
    : position(pos), scale(scl)
{
    setupMesh();
    bakeRodData();
}

void Cube::setupMesh()
{
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);

    glBufferData(GL_ARRAY_BUFFER, sizeof(vertexData), nullptr, GL_DYNAMIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(0);

    glBindVertexArray(0);
}

void Cube::bakeRodData()
{
    const float min = -0.5f;
    const float max = +0.5f;
    const float rodHalf = 0.003f; // Half thickness -> 0.1 total thickness

    glm::vec3 edges[12][2] = {
        {{min, min, min}, {max, min, min}},
        {{max, min, min}, {max, max, min}},
        {{max, max, min}, {min, max, min}},
        {{min, max, min}, {min, min, min}},

        {{min, min, max}, {max, min, max}},
        {{max, min, max}, {max, max, max}},
        {{max, max, max}, {min, max, max}},
        {{min, max, max}, {min, min, max}},

        {{min, min, min}, {min, min, max}},
        {{max, min, min}, {max, min, max}},
        {{max, max, min}, {max, max, max}},
        {{min, max, min}, {min, max, max}}
    };

    int vertexOffset = 0;

    for (int i = 0; i < 12; ++i)
    {
        glm::vec3 a = edges[i][0];
        glm::vec3 b = edges[i][1];

        glm::vec3 center = (a + b) * 0.5f;
        glm::vec3 dir = glm::normalize(b - a);

        float halfLength = glm::length(b - a) * 0.5f;

        glm::vec3 up = fabs(dir.y) < 0.99f ? glm::vec3(0,1,0) : glm::vec3(1,0,0);
        glm::vec3 right = glm::normalize(glm::cross(dir, up)) * rodHalf;
        glm::vec3 forward = glm::normalize(glm::cross(right, dir)) * rodHalf;

        glm::vec3 verts[8] = {
            center - dir * halfLength - right - forward,
            center - dir * halfLength + right - forward,
            center - dir * halfLength + right + forward,
            center - dir * halfLength - right + forward,
            center + dir * halfLength - right - forward,
            center + dir * halfLength + right - forward,
            center + dir * halfLength + right + forward,
            center + dir * halfLength - right + forward
        };

        unsigned int idx[36] = {
            0,1,2, 2,3,0,
            4,5,6, 6,7,4,
            0,1,5, 5,4,0,
            2,3,7, 7,6,2,
            1,2,6, 6,5,1,
            3,0,4, 4,7,3
        };

        for (int j = 0; j < 36; ++j)
        {
            glm::vec3 v = verts[idx[j]];
            vertexData[vertexOffset++] = v.x;
            vertexData[vertexOffset++] = v.y;
            vertexData[vertexOffset++] = v.z;
        }
    }

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertexOffset * sizeof(float), vertexData, GL_STATIC_DRAW);
}

void Cube::Draw(Shader &shader) const
{
    glm::mat4 model = glm::translate(glm::mat4(1.0f), position);
    model = glm::scale(model, scale);
    shader.SetUniformMat4("u_Model", glm::value_ptr(model));

    glBindVertexArray(VAO);

    glDrawArrays(GL_TRIANGLES, 0, 12 * 36);

    glBindVertexArray(0);
}