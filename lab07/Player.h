#pragma once

#ifndef PLAYER_H
#define PLAYER_H


#include "RigidBody.h"
#include <GL/glew.h>

#include "glfw3.h"


class Drawable;
class Cube;

class Player : public RigidBody {

public:

    Drawable* playerBody,*playerBox; //ehume to original drawable body kai ta 8 vertices pou xrhsimopoioume gia to AABB
    AABB* playerAABB;
    GLFWwindow* window;
    static bool spawned;

    glm::mat4 modelMatrix, playerView, playerProjection;
    GLuint modelMatrixLocation;

    float movementspeed;
    float horizontalAngle, verticalAngle;
    float mouseSpeed;
    glm::vec3 viewDirection,cameraPosition;


    Player::Player(GLFWwindow* window,glm::vec3 position, float w, float h,float m,float movspeed);
    void update(float t, float dt); //spacebar gia jump (initial v.y velocity) 
    void draw(GLuint location);
    void move(bool onFloor, float dt); //mporoun ta panta na ginontai sthn update.
    bool checkRayPlaneIntersection(glm::vec3 planePosition, glm::vec3 planeNormal);
    glm::vec3 rayCastToPlane(glm::vec3 planePosition, glm::vec3 planeNormal);
    void calculatePlayerAABB();
    void applyGravity();
    void Player::disableGravity();
    //void spawnPlayer(glm::vec3 pos);
};

#endif