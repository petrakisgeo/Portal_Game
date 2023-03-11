#include "Scene.h"
#include <string.h>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>



using namespace glm;

bool checkIfOrthogonal(vec3 edge1, vec3 edge2);

int Scene::numOfWalls = 0; 

Scene::Scene()
{
	for (int i = 0; i < 10; i++)
	{
		walls[i] = new Wall;
		try
		{
			walls[i]->wallModel = new Drawable("newscene/wall_" + std::to_string(i) + ".obj");
			walls[i]->texture = loadSOIL("textures/portalgamewalltexture.png");
			//Scene::numOfWalls++;
		}
		catch (std::runtime_error)
		{
			break;
		} //an o drawable constructor kanei throw runtime error tote den iparxoun alla walls kai kanoume break
	}

	floors[0] = new Wall;
	floors[0]->wallModel = new Drawable("newscene/floor_0.obj");
	floors[0]->texture = loadSOIL("textures/portalgamewalltexture.png");
	walls[4] = floors[0];

	floors[1] = new Wall;
	floors[1]->wallModel = new Drawable("newscene/floor_1.obj");
	floors[1]->texture = loadSOIL("textures/portalgamewalltexture.png");
	walls[5] = floors[1];


	ceilings[0] = new Wall;
	ceilings[0]->wallModel = new Drawable("newscene/ceiling_0.obj");
	ceilings[0]->texture = loadSOIL("textures/portalgamewalltexture.png");
	walls[6] = ceilings[0];

	skybox = new Drawable("newscene/skybox.obj");
	skyboxTexture = loadSOIL("textures/sky_texture.png");
	trophy = new Drawable("newscene/trophy.obj");

}


void Scene::draw(GLuint modelMatrixLocation, GLuint textureSamplerLocation)
{
	glUniformMatrix4fv(modelMatrixLocation, 1, GL_FALSE, &modelMatrix[0][0]); //ta antikeimena ths skhnhs emfanizontai se ena shmeio kai den metakinountai
	//view kai projection matrices dinontai prin to draw sto main program
	for (int i = 0; i < 4; i++)
	{
		walls[i]->wallModel->bind();
		glActiveTexture(GL_TEXTURE3);
		glBindTexture(GL_TEXTURE_2D, walls[i]->texture);
		glUniform1i(textureSamplerLocation, 3);
		walls[i]->wallModel->draw();
	}
	ceilings[0]->wallModel->bind();
	ceilings[0]->wallModel->draw();
	for (int i = 0; i < 2; i++)
	{
		floors[i]->wallModel->bind();
		floors[i]->wallModel->draw();
	}


}
void Scene::drawTrophy(GLuint modelMatrixLocation, GLuint textureSamplerLocation)
{
	glUniformMatrix4fv(modelMatrixLocation, 1, GL_FALSE, &modelMatrix[0][0]);
	trophy->bind();
	trophy->draw();
}

void Scene::draw(GLuint modelMatrixLocation)
{
	glUniformMatrix4fv(modelMatrixLocation, 1, GL_FALSE, &modelMatrix[0][0]); //ta antikeimena ths skhnhs emfanizontai se ena shmeio kai den metakinountai
//view kai projection matrices dinontai prin to draw sto main program
	for (int i = 0; i < 4; i++)
	{
		walls[i]->wallModel->bind();
		walls[i]->wallModel->draw();
	}
	ceilings[0]->wallModel->bind();
	ceilings[0]->wallModel->draw();
	for (int i = 0; i < 2; i++)
	{
		floors[i]->wallModel->bind();
		floors[i]->wallModel->draw();
	}
	trophy->bind();
	trophy->draw();
}
//okay h skhnh sxedon mono me xrwma........

void Scene::drawSkybox(GLuint modelMatrixLocation,GLuint textureSamplerLocation)
{
	mat4 skyscale = scale(mat4(), vec3(3.0, 3.0, 3.0));

	skybox->bind();
	glUniformMatrix4fv(modelMatrixLocation, 1, GL_FALSE, &skyscale[0][0]);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, skyboxTexture);
	glUniform1i(textureSamplerLocation, 1);
	skybox->draw();
}

bool checkIfOrthogonal(vec3 edge1, vec3 edge2)
{
	float dotProduct = dot(edge1, edge2);
	if (abs(dotProduct) > 0.1f) return false;
	else return true;
}

bool Scene::checkPointInsideWall(vec3 point, Wall* wall)
{
	vec3 xAxis = vec3(1.0f, 0.0f, 0.0f);
	float minX = 10000.0f, maxX = -10000.0f;
	float minY = 10000.0f, maxY = -10000.0f;
	float minZ = 10000.0f, maxZ = -10000.0f;
	vec3 yAxis = vec3(0.0f, 1.0f, 0.0f);
	vec3 zAxis = vec3(0.0f, 0.0f, 1.0f);

	std::vector<vec3> wallVertices = wall->wallModel->vertices;
	vec3 normal = wall->wallModel->normals[0];

	if (normal == yAxis || normal==-yAxis)
	{
		for (int i = 0; i < 4; i++)
		{
			minX = min(minX, wallVertices[i].x);
			maxX = max(maxX, wallVertices[i].x);

			minZ = min(minZ, wallVertices[i].z);
			maxZ = max(maxZ, wallVertices[i].z);
		}

		if (minX < point.x && point.x < maxX && minZ < point.z && point.z < maxZ) return true;
		else return false;
	}
	if (normal == zAxis || normal==-zAxis)
	{
		for (int i = 0; i < 4; i++)
		{
			minX = min(minX, wallVertices[i].x);
			maxX = max(maxX, wallVertices[i].x);

			minY = min(minY, wallVertices[i].y);
			maxY = max(maxY, wallVertices[i].y);
		}

		if (minX < point.x && point.x < maxX && minY < point.y && point.y < maxY) return true;
		else return false;
	}
	if (normal == xAxis || normal==-xAxis)
	{
		for (int i = 0; i < 4; i++)
		{
			minY = min(minY, wallVertices[i].y);
			maxY = max(maxY, wallVertices[i].y);

			minZ = min(minZ, wallVertices[i].z);
			maxZ = max(maxZ, wallVertices[i].z);
		}

		if (minY < point.y && point.y < maxY && minZ < point.z && point.z < maxZ) return true;
		else return false;
	}

	/*/
	//APOTIXIMENI PROSPATHEIA NA "GENIKOPOIHSW" TON KWDIKA ME ALGORITHMO GIA OPOIONDHPOTE TOIXO ME SCALAR PRODUCTS KAI PROVOLES
	vec3 A = wallVertices[0];
	vec3 B = wallVertices[1];
	vec3 C = wallVertices[2];
	vec3 D = wallVertices[3];
	//edges
	vec3 AB = B - A;
	vec3 AC = C - A;
	vec3 AD = D - A;
	vec3 AP = point - A;
	

	if (0.0f < dot(AP, AB) && dot(AP, AB) < dot(AB, AB) && 0.0f < dot(AP, AC) && dot(AP, AC) < dot(AC, AC)) return true;

	else return false;
	//*/

}


vec3 Scene::projectPointToWall(vec3 point, Wall* wall)
{
	vec3 randomWallPoint = wall->wallModel->vertices[0];
	vec3 normal = wall->wallModel->normals[0];

	vec3 projectedPoint = point + dot(point - randomWallPoint, normal) * normal;

	return projectedPoint;
}