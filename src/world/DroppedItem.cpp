#include "DroppedItem.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <GLFW/glfw3.h>
#include "../graphics/objects/BlockDatabase.h"
#include <iostream>
#include <functional>
#include "World.h"
#include "../player/ItemDatabase.h"

namespace std
{
    template <>
    struct hash<DroppedItem>
    {
        size_t operator()(const DroppedItem &item) const
        {
            size_t hx = std::hash<int>()(static_cast<int>(item.position.x));
            size_t hy = std::hash<int>()(static_cast<int>(item.position.y));
            size_t hz = std::hash<int>()(static_cast<int>(item.position.z));
            size_t hb = std::hash<int>()(item.itemID);

            return (((hx ^ (hy << 1)) >> 1) ^ (hz << 1)) ^ (hb << 2);
        }
    };
}

extern unsigned int atlasTexture;

DroppedItem::DroppedItem(const glm::vec3 &pos, int block, int quantity)
    : position(pos + glm::vec3(0.5f, 0.1f, 0.5f)), rotationAngle(0.0f), bobOffset(0.0f), itemID(block), time(glfwGetTime()), quantity(quantity), velocity(0.0f, 2.0f, 0.0f), lastUpdate(glfwGetTime())
{
    InitMesh(ITEMS[block].blockPlace);
}

std::ostream &operator<<(std::ostream &os, const DroppedItem &item)
{
    os << "DroppedItem { "
       << "Position: (" << item.position.x << ", " << item.position.y << ", " << item.position.z << "), "
       << "ItemID: " << item.itemID << ", "
       << "RotationAngle: " << item.rotationAngle << ", "
       << "BobOffset: " << item.bobOffset << ", "
       << "Time: " << item.time
       << " }";
    return os;
}

// float DroppedItem::HashPositionToAngle() const
// {
//     float value = position.x * 12.9898f + position.y * 78.233f + position.z * 37.719f;
//     return fmod(sin(value) * 43758.5453f, 360.0f);
// }

void DroppedItem::Update(World &world)
{
    float newTime = glfwGetTime();

    
    float deltaTime = newTime - lastUpdate;

    if (deltaTime > 0.05f)
    {
        deltaTime = 0.05f;
    }

    // gravity
    velocity -= glm::vec3(0.0f, 9.8f * deltaTime, 0.0f);

    // max fall speed
    if (velocity.length() > 30.0f)
    {
        velocity = glm::normalize(velocity) * 30.0f;
    }

    glm::vec3 nextPosition = position + velocity * deltaTime;

    // collision check at new position (just check bottom point for now)
    glm::vec3 checkPos = nextPosition + glm::vec3(0.0f, 0.0f, 0.0f); // 0.4 is approximate height above block of the item

    int blockX = static_cast<int>(floor(checkPos.x));
    int blockY = static_cast<int>(floor(checkPos.y));
    int blockZ = static_cast<int>(floor(checkPos.z));

    if (world.GetBlock(blockX, blockY, blockZ) != 0)
    {
        // stop falling: set vertical position to block top
        position.y = floor(checkPos.y) + 1.0f; // snap to top of block
        velocity.y = 0.0f;
    }
    else
    {
        position = nextPosition;
    }

    lastUpdate = newTime;

    rotationAngle = (newTime - time) * 90.0f;
    bobOffset = sin((newTime - time) * 2.0f) * 0.1f;
}

void DroppedItem::Draw(Shader &shader) const
{
    glm::mat4 model = glm::translate(glm::mat4(1.0f), position + glm::vec3(0.0f, bobOffset + 1.5f, 0.0f));
    model = glm::rotate(model, glm::radians(rotationAngle), glm::vec3(0.0f, 1.0f, 0.0f));
    model = glm::scale(model, glm::vec3(0.4f));

    shader.SetUniformMat4("u_Model", glm::value_ptr(model));

    glBindVertexArray(VAO);
    glBindTexture(GL_TEXTURE_2D, atlasTexture);
    shader.SetUniform1i("u_Texture", 0);

    glDrawArrays(GL_TRIANGLES, 0, 36);

    glBindVertexArray(0);
}

void DroppedItem::InitMesh(int blockID)
{
    const float tileSize = 1.0f / 16.0f;
    const float brightness = 1.0f;

    const glm::vec3 positions[8] = {
        {-0.5f, -0.5f, -0.5f},
        {0.5f, -0.5f, -0.5f},
        {0.5f, 0.5f, -0.5f},
        {-0.5f, 0.5f, -0.5f},
        {-0.5f, -0.5f, 0.5f},
        {0.5f, -0.5f, 0.5f},
        {0.5f, 0.5f, 0.5f},
        {-0.5f, 0.5f, 0.5f}};

    const int faces[6][6] = {
        {0, 1, 2, 2, 3, 0}, // -Z front
        {4, 5, 6, 6, 7, 4}, // +Z back
        {0, 4, 7, 7, 3, 0}, // -X left
        {1, 5, 6, 6, 2, 1}, // +X right
        {0, 1, 5, 5, 4, 0}, // -Y bottom
        {3, 2, 6, 6, 7, 3}  // +Y top
    };

    std::vector<float> vertices;

    for (int face = 0; face < 6; ++face)
    {
        TileCoord tile = BLOCKS[blockID].faceTiles[face];
        float uMin = tile.x * tileSize;
        float uMax = uMin + tileSize;
        float vMax = 1.0f - tile.y * tileSize; // vMax is bottom, vMin is top
        float vMin = vMax - tileSize;

        glm::vec2 uvs[6];

        // Define a "standard" UV mapping (no rotation relative to default vertex order)
        // Maps texture's BL to vertex 0 of the face quad (e.g., positions[faces[face][0]])
        // TR is (uMax, vMin), TL is (uMin, vMin), BL is (uMin, vMax), BR is (uMax, vMax)
        glm::vec2 standardUvs[6] = {
            {uMin, vMax}, // For vertex 0 (BL)
            {uMax, vMax}, // For vertex 1 (BR)
            {uMax, vMin}, // For vertex 2 (TR)
            {uMax, vMin}, // Duplicate of vertex 2
            {uMin, vMin}, // For vertex 3 (TL)
            {uMin, vMax}  // Duplicate of vertex 0
        };

        // Define a 180-degree rotated UV mapping
        // This maps the texture's corners such that it appears rotated 180 degrees.
        // It takes the original Top-Right corner and maps it to the position that
        // the Bottom-Left corner would normally go to, and so on.
        glm::vec2 rotated180Uvs[6] = {
            {uMax, vMin}, // Original TR goes to vertex 0's position
            {uMin, vMin}, // Original TL goes to vertex 1's position
            {uMin, vMax}, // Original BL goes to vertex 2's position
            {uMin, vMax}, // Duplicate of vertex 2
            {uMax, vMax}, // Original BR goes to vertex 3's position
            {uMax, vMin}  // Duplicate of vertex 0
        };

        // --- APPLY THE 180-DEGREE ROTATION TO ALL FACES ---
        // Based on your explicit request, we will use the rotated180Uvs for all faces.
        for (int i = 0; i < 6; ++i)
        {
            uvs[i] = rotated180Uvs[i];
        }

        for (int i = 0; i < 6; ++i)
        {
            const glm::vec3 &pos = positions[faces[face][i]];
            const glm::vec2 &uv = uvs[i];

            vertices.push_back(pos.x);
            vertices.push_back(pos.y);
            vertices.push_back(pos.z);

            vertices.push_back(uv.x);
            vertices.push_back(uv.y);

            vertices.push_back(brightness);
        }
    }

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void *)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void *)(5 * sizeof(float)));
    glEnableVertexAttribArray(2);

    glBindVertexArray(0);
}