#ifndef COLLISION_H
#define COLLISION_H

#include <glm/glm.hpp>

class Box;
class Sphere;
class Portal;
class Scene;
class Player;
struct AABB;

void handleBoxSphereCollision(Box& box, Sphere& sphere);
void handleSphereWithSphereCollision(Sphere& sphere1, Sphere& sphere2);
void handleSphereWithPortalCollision(Sphere& original,Sphere& duplicate, Portal* portal);
void handleSphereWithSceneCollision(Sphere& sphere, Scene& scene, Portal** portals);
void handlePlayerWithSceneCollision(Player& player, Scene& scene);

bool checkSphereWithPortalFrontAABBCollision(Sphere& sphere, Portal* portal);
bool checkSphereWithPortalBackAABBCollision(Sphere& sphere, Portal* portal);
bool checkSphereWithPortalPlaneCollision(Sphere& sphere, Portal* portal);
bool checkSphereWithSceneCollision(Sphere& sphere, Scene& scene, Portal** portals, glm::vec3& collisionVector);
bool checkPlayerWithSceneWallsCollision(Player& player, Scene& scene, Portal** portals, glm::vec3& collisionVector);
bool checkPlayerWithSceneFloorsCollision(Player& player, Scene& scene, Portal** portals, glm::vec3& collisionVector);

bool checkAABBs(AABB* a, AABB* b);
#endif
