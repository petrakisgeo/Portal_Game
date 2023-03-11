#include <glfw3.h>
#include <glm/gtc/matrix_transform.hpp>
#include "camera.h"

using namespace glm;

Camera::Camera(GLFWwindow* window,glm::vec3 pos) : window(window) {
    position = pos;
    horizontalAngle = 3.14f;
    verticalAngle = 0.0f;
    FoV = 45.0f;
    speed = 3.0f;
    mouseSpeed = 0.001f;
    fovSpeed = 2.0f;
}


bool Camera::checkRayPlaneIntersection(vec3 planePosition, vec3 planeNormal) {

    float distanceFromPlane;
    double xPos, yPos;
    glfwGetCursorPos(window, &xPos, &yPos); //viewport coordinates

    //kanonikopoioume sto diastima -1,1
    float x = (2.0f * xPos) / 1024.0f - 1.0f;
    float y = 1.0f - (2.0f * yPos) / 768.0f;
    vec3 ray_nds = vec3(x, y, 1.0f);

    vec4 ray_clip = vec4(ray_nds.x, ray_nds.y, -1.0, 1.0);

    vec4 ray_eye = inverse(projectionMatrix) * ray_clip;

    ray_eye = vec4(ray_eye.x, ray_eye.y, -1.0, 0.0);

    vec4 ray_wor = inverse(viewMatrix) * ray_eye;
    vec3 ray_world = vec3(ray_wor.x, ray_wor.y, ray_wor.z); //dianisma

    ray_world = normalize(ray_world);

    if (dot(planeNormal, ray_world) == 0) return false; //plane parallhlo me ray
    else
    {
        distanceFromPlane = -(dot(position, planeNormal) + distance(position, planePosition)) / dot(planeNormal, ray_world);
        if (distanceFromPlane == 0 || distanceFromPlane < 0) return false;
    }
    return true;
}

vec3 Camera::rayCastToPlane(vec3 planePosition, vec3 planeNormal)
{
    float distanceFromPlane;
    vec3 intersectionPoint;
    double xPos, yPos;
    glfwGetCursorPos(window, &xPos, &yPos); //viewport coordinates

    //kanonikopoioume sto diastima -1,1
    float x = (2.0f * xPos) / 1024.0f - 1.0f;
    float y = 1.0f - (2.0f * yPos) / 768.0f;
    vec3 ray_nds = vec3(x, y, 1.0f);

    vec4 ray_clip = vec4(ray_nds.x, ray_nds.y, -1.0, 1.0);

    vec4 ray_eye = inverse(projectionMatrix) * ray_clip;

    ray_eye = vec4(ray_eye.x, ray_eye.y, -1.0, 0.0);

    vec4 ray_wor = inverse(viewMatrix) * ray_eye;
    vec3 ray_world = vec3(ray_wor.x, ray_wor.y, ray_wor.z); //dianisma

    ray_world = normalize(ray_world);

    distanceFromPlane = -(dot(position, planeNormal) + distance(position, planePosition)) / dot(planeNormal, ray_world); //eksiswnoume dianismatikes plane kai ray equations
    intersectionPoint = position + distanceFromPlane * ray_world;
    
    return intersectionPoint;
}

void Camera::update() {
    // glfwGetTime is called only once, the first time this function is called
    static double lastTime = glfwGetTime();

    // Compute time difference between current and last frame
    double currentTime = glfwGetTime();
    float deltaTime = float(currentTime - lastTime);

    // Get mouse position
    double xPos, yPos;
    glfwGetCursorPos(window, &xPos, &yPos);

    int width, height;
    glfwGetWindowSize(window, &width, &height);

    // Reset mouse position for next frame
    glfwSetCursorPos(window, width / 2, height / 2);

    // Task 5.1: simple camera movement that moves in +-z and +-x axes
    /*/
    // Move forward
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
        position -= vec3(0, 0, 1) * deltaTime * speed;
    }
    // Move backward
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
        position += vec3(0, 0, 1) * deltaTime * speed;
    }
    // Strafe right
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
        position += vec3(1, 0, 0) * deltaTime * speed;
    }
    // Strafe left
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
        position -= vec3(1, 0, 0) * deltaTime * speed;
    }

    // Task 5.2: update view matrix so it always looks at the origin
    projectionMatrix = perspective(radians(FoV), 4.0f / 3.0f, 0.1f, 100.0f);
    viewMatrix = lookAt(
        position,
        vec3(0, 0, 0),
        vec3(0, 1, 0)
    );
    //*/

    // Task 5.3: Compute new horizontal and vertical angles, given windows size
    //*/
    // and cursor position
    horizontalAngle += mouseSpeed * float(width / 2 - xPos);
    verticalAngle += mouseSpeed * float(height / 2 - yPos);

    // Task 5.4: right and up vectors of the camera coordinate system
    // use spherical coordinates
    vec3 direction(
        cos(verticalAngle) * sin(horizontalAngle),
        sin(verticalAngle),
        cos(verticalAngle) * cos(horizontalAngle)
    );

    // Right vector
    vec3 right(
        sin(horizontalAngle - 3.14f / 2.0f),
        0,
        cos(horizontalAngle - 3.14f / 2.0f)
    );
    // Up vector
    vec3 up = cross(right, direction);

    // Task 5.5: update camera position using the direction/right vectors
    // Move forward
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
        position += direction * deltaTime * speed;
    }
    // Move backward
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
        position -= direction * deltaTime * speed;
    }
    // Strafe right
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
        position += right * deltaTime * speed;
    }
    // Strafe left
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
        position -= right * deltaTime * speed;
    }

    // Task 5.6: handle zoom in/out effects
    if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) {
        FoV -= fovSpeed;
    }
    if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) {
        FoV += fovSpeed;
    }
    if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) {
        speed = 6.0f; //sprint
    }
    else speed = 3.0f; //walk
    // Task 5.7: construct projection and view matrices
    projectionMatrix = perspective(radians(FoV), 4.0f / 3.0f, 0.1f, 100.0f);
    viewMatrix = lookAt(
        position,
        position + direction,
        up
    );
    //*/

    // Homework XX: perform orthographic projection

    // For the next frame, the "last time" will be "now"
    lastTime = currentTime;
}

