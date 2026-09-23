#pragma once

#include <glm/glm.hpp>
#include "../graphics/Shader.h"

class World;

class DroppedItem
{
public:
    DroppedItem(const glm::vec3& position, int blockID, int quantity);

    void Update(World& world);
    void Draw(Shader& shader) const;

    glm::vec3 position;
    float rotationAngle;
    float bobOffset;
    int itemID;
    float time;
    int quantity;
    glm::vec3 velocity;
    float lastUpdate;

    friend std::ostream& operator<<(std::ostream& os, const DroppedItem& item);

    bool operator==(const DroppedItem& other) const
    {
        return itemID == other.itemID &&
               glm::distance(position, other.position) < 0.01f  && time == other.time;
    }


private:
    unsigned int VAO, VBO;

    void InitMesh(int blockID);
    float HashPositionToAngle() const;
};