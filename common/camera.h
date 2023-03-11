#ifndef CAMERA_HPP
#define CAMERA_HPP


#include <glm/glm.hpp>

#include "glfw3.h"

class Camera {
public:
    GLFWwindow* window;
    glm::mat4 viewMatrix;
    glm::mat4 projectionMatrix;
    // Initial position : on +Z
    glm::vec3 position;
    // Initial horizontal angle : toward -Z
    float horizontalAngle;
    // Initial vertical angle : none
    float verticalAngle;
    // Field of View
    float FoV;
    float speed; // units / second
    float mouseSpeed;
    float fovSpeed;

    Camera(GLFWwindow* window,glm::vec3 pos);
    void update();
    bool checkRayPlaneIntersection(glm::vec3 planePosition, glm::vec3 planeNormal);
    glm::vec3 rayCastToPlane(glm::vec3 planePosition, glm::vec3 planeNormal);
};

#endif
