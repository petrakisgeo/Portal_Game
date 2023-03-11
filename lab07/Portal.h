#pragma once

#define PORTAL_H

#include "RigidBody.h"
#include <GL/glew.h>
#include <glfw3.h>
#include "common/camera.h"
#include "Box.h"

using namespace std;
using namespace glm;

class Drawable;


class Portal {
public:
	//ta ftiaxnw se drawables giati thelw na krataw ta vertices tous
	Drawable* portal,*box;

	AABB* frontBoundingBox;
	AABB* backBoundingBox;
	float portalBoxDistance = 1.0f;

	static int num_of_portals_spawned;
	const std::vector<vec3> vertices = {
		vec3(-1.0,-1.5,0.0),vec3(1.0f,1.5f,0.0f),vec3(-1.0,1.5,0.0f), //triangle1
		vec3(1.0f,-1.5f,0.0f), vec3(1.0f,1.5f,0.0f), vec3(-1.0f,-1.5f,0.0f) }; //triangle2

	const std::vector<vec3> normals = {
		vec3(0.0f,0.0f,1.0f),vec3(0.0f,0.0f,1.0f),vec3(0.0f,0.0f,1.0f),
		vec3(0.0f,0.0f,1.0f),vec3(0.0f,0.0f,1.0f),vec3(0.0f,0.0f,1.0f) }; //koitane pros ton aksona z

	const std::vector<vec2> uvs = {
		vec2(0.0f, 1.0f), vec2(1.0f, 0.0f), vec2(0.0f, 0.0f),
		vec2(1.0f, 1.0f), vec2(1.0f, 0.0f), vec2(0.0f, 1.0f) }; 

	vec4 position;
	vec4 portalNormal;
	vec4 portalUpVector;
	GLuint texture;
	Portal* dst;

	mat4 modelMatrix;

	Portal::Portal(GLFWwindow* window);
	void Portal::draw(GLuint modelMatrixLocation);
	void Portal::drawOutline(GLuint modelMatrixLocation, GLuint textureSamplerLocation);
	void Portal::drawBoundingBox(GLuint modelMatrixLocation);
	void Portal::erasePortal();
	void Portal::calculatePortalAABBs(mat4 modelMatrix = mat4(1.0f));
	static void Portal::spawnPortal(Portal& portal, vec3 wallPosition, vec3 wallNormal, GLFWwindow* window);
};

