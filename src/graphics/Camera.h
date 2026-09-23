#ifndef CAMERA_H
#define CAMERA_H

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

extern float Fov;
extern float AspectRatio;
extern float NearPlane;
extern float FarPlane;

class Camera {
public:
    glm::vec3 Position;
    glm::vec3 Front;
    glm::vec3 Up;
    glm::vec3 Right;
    glm::vec3 WorldUp;

    float Yaw;
    float Pitch;

    float MovementSpeed;
    float MouseSensitivity;

    Camera(glm::vec3 position, glm::vec3 up, float yaw, float pitch);

    glm::mat4 GetViewMatrix();
    void ProcessKeyboard(int direction, float deltaTime);
    void ProcessMouseMovement(float xoffset, float yoffset);

    glm::vec3 GetDirectionVector(float length = 0.1f) const;


    bool IsChunkVisible(float chunkX, float chunkY, float chunkZ, float chunkSize, float viewDistance = 64.0f) const;

    glm::mat4 GetProjection() const;


private:
    void updateCameraVectors();
};

enum Camera_Movement {
    FORWARD,
    BACKWARD,
    LEFT,
    RIGHT
};

#endif
