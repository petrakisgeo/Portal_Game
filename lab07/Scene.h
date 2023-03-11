#ifndef SCENE_H
#define SCENE_H

#include "common/model.h"
#include "common/texture.h"




struct Wall {
    Drawable* wallModel;
    GLuint texture;
};


class Scene {
public:

    Wall* walls[7];
    static int numOfWalls;
    Wall* floors[10];
    Wall* ceilings[2];
    Drawable* skybox,*trophy;
    GLuint skyboxTexture;

    glm::mat4 modelMatrix=glm::mat4(1.0f); //ta antikeimena ths skhnhs den metakinountai

    Scene::Scene();

    void Scene::draw(GLuint modelMatrixLocation, GLuint textureSamplerLocation);
    void Scene::draw(GLuint modelMatrixLocation);
    void Scene::drawSkybox(GLuint modelMatrixLocation, GLuint textureSamplerLocation);
    void Scene::drawTrophy(GLuint modelMatrixLocation, GLuint textureSamplerLocation);
    glm::vec3 projectPointToWall(glm::vec3 point, Wall* wall);
    bool checkPointInsideWall(glm::vec3 point, Wall* wall);
};

#endif