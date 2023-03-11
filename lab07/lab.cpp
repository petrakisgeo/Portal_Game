 // Include C++ headers
#include <iostream>
#include <string>

// Include GLEW
#include <GL/glew.h>
#include <GL/gl.h>
// Include GLFW
#include <glfw3.h>

// Include GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

// Shader loading utilities and other
#include <common/shader.h>
#include <common/util.h>
#include <common/camera.h>
#include <common/model.h>
#include <common/texture.h>

#include "Cube.h"
#include "Sphere.h"
#include "Box.h"
#include "MassSpringDamper.h"
#include "Collision.h"
#include "Portal.h"
#include "Player.h"
#include "Scene.h"

#include "lighting/light.h"

using namespace std;
using namespace glm;

// Function prototypes
void initialize();
void createContext();
void mainLoop();
void free();
void switchTextureMode(int method, GLuint textureSelectorLocation);
void renderPortalImage(Portal* portal, Portal* portal2, Camera* virtualCamera1, Camera* virtualCamera2); //1st try
void createDepthTextureBuffer();
void createDepthTexture();

void renderRecursivePortals(mat4 currentViewMatrix, mat4 currentProjMatrix, int max_recursion, int current_recursion);

mat4 computePortalCameraViewMatrix(mat4 originalViewMatrix, Portal* sourcePortal, Portal* destinationPortal);
mat4 computePortalCameraProjectionMatrix(mat4 currentViewMatrix, mat4 originalProjMatrix,Portal* motherPortal);
float sgn(float x);

bool checkCameraPortalCollision(Portal* portal, vec3 prevCameraPosition, vec3 newCameraPosition); //line-plane intersection check.
//kanoume camera->update(). Protou kanoume render tin skini prepei na tsekaroume an h camera perase mesa apo to portal
//https://en.wikipedia.org/wiki/Line%E2%80%93plane_intersection
void renderDuplicatePlayer(Player* player, Portal* sourcePortal, GLuint modelMatrixLocation);
void teleportPlayerCamera(Portal* sourcePortal,Player& player);

void renderDuplicateSphere(Sphere* sphere, Portal* portal,GLuint modelMatrixLocation);
void teleportOriginalSphere(Sphere& sphere, Portal* sourcePortal);

#define W_WIDTH 1024
#define W_HEIGHT 768
#define SHADOW_WIDTH 2048*2
#define SHADOW_HEIGHT 2048*2
#define TITLE "PORTALS 1"


// Global variables
GLFWwindow* window;
Camera* camera, *testCamera;
GLuint shaderProgram,depthProgram;
//for standardShading shaders
GLuint projectionMatrixLocation, viewMatrixLocation, modelMatrixLocation,lightVPLocation;
GLuint textureSelectorLocation, textureSamplerLocation,shadowMapSamplerLocation;
GLuint timeLocation;

//for depth texture shaders
GLuint depthFrameBuffer,depthTexture;
GLuint shadowViewProjectionLocation, shadowModelLocation;

//for light parameters
GLuint lightAmbientLocation, lightDiffuseLocation, lightSpecularLocation;
GLuint lightPositionLocation, lightPowerLocation;

Sphere* sphere;
Portal* portal1,*portal2;
Portal** portals;

Player* player;
Scene* scene;
Light* directionalLight;
Box* box;


Drawable* shadowscene;

bool ballThrown = false;

void createContext() {
    
    //standard rendering shaders
    shaderProgram = loadShaders(
        "StandardShading.vertexshader",
        "StandardShading.fragmentshader");

    
    projectionMatrixLocation = glGetUniformLocation(shaderProgram, "P");
    viewMatrixLocation = glGetUniformLocation(shaderProgram, "V");
    modelMatrixLocation = glGetUniformLocation(shaderProgram, "M");
    lightVPLocation = glGetUniformLocation(shaderProgram, "lightVP");

    textureSelectorLocation= glGetUniformLocation(shaderProgram, "useTexture");
    textureSamplerLocation= glGetUniformLocation(shaderProgram, "textureSampler");
    shadowMapSamplerLocation = glGetUniformLocation(shaderProgram, "shadowMapSampler");
    timeLocation = glGetUniformLocation(shaderProgram, "time");

    //light parameters (mas endiaferei mono h thesi kai h isxis afou pada tha einai white light)
    lightPositionLocation = glGetUniformLocation(shaderProgram, "light.lightPosition_worldspace");
    lightPowerLocation = glGetUniformLocation(shaderProgram, "light.power");

    //depth texture shaders
    depthProgram = loadShaders("Depth.vertexshader", "Depth.fragmentshader");
    shadowViewProjectionLocation = glGetUniformLocation(depthProgram, "VP");
    shadowModelLocation = glGetUniformLocation(depthProgram, "M");

    
    scene = new Scene();
    player = new Player(window, vec3(0, 1, 3), 1.0f, 1.0f, 10.0f, 2.0f);

    directionalLight = new Light(window,
        vec3{ 16.0f, 20.0f, 0.0f }, //peripou sto kentro tou scene?
        400.0f); 
    


    portal1 = new Portal(window);
    portal2 = new Portal(window);

    portal1->dst = portal2;
    portal2->dst = portal1;

    portals = (Portal**) malloc(2 * sizeof(Portal*));

    portals[0] = portal1;
    portals[1] = portal2;

    //idia skhnh me ta teixh na exoun paxos gia na mhn iparxei peterpanning
}

void createDepthTextureBuffer()
{
    glGenFramebuffers(1, &depthFrameBuffer);

    glBindFramebuffer(GL_FRAMEBUFFER, depthFrameBuffer);

    glGenTextures(1, &depthTexture);
    glBindTexture(GL_TEXTURE_2D, depthTexture);


    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOW_WIDTH, SHADOW_HEIGHT, 0,
        GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    float bordercolor[] = { 1.0, 1.0, 1.0,1.0 };
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, bordercolor);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthTexture, 0);

    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        glfwTerminate();
        throw runtime_error("Frame buffer not initialized correctly");
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0); //ksanabind ton default framebuffer gia asfaleia
}

//fragmentshader texturing method
void switchTextureMode(int mode, GLuint textureSelectorLocation)
{
    glUniform1i(textureSelectorLocation, mode);
    //mode==1 material properties and lighting
    //mode==2 portal outline rendering, rotating texture
    //mode==3 for scene rendering, static textures 
    //mode==4 for skybox rendering, without shadow texture
} 

//portal image rendering
//1st try
void renderPortalImage(Portal* portal1, Portal* portal2, Camera* virtualCamera1, Camera* virtualCamera2){
    
	//h sinartisi auti ine outdated kai leitourgouse se arxiki ekdosi tou programmatos omws paratithetai giati panw se autin vasizetai h recursivePortalImage

    glDepthMask(GL_FALSE); //den grafoume ston depth buffer
    glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE); //den grafoume ston color buffer
    glStencilMask(0xFF); //grafoume ston stencil buffer

    glClear(GL_STENCIL_BUFFER_BIT); //katharizoume ton buffer prin kathe render
    glDisable(GL_DEPTH_TEST); 
    glEnable(GL_STENCIL_TEST);
    
    glStencilFunc(GL_NEVER,1,0xFF);
    glStencilOp(GL_INCR,GL_KEEP,GL_KEEP); //auksanoume to stencil buffer bit an GL_STENCIL_TEST fails gia auto to pixel othonis, dhladh pantote logw GL_NEVER
    

    //kanume render ston stencil buffer to portal apto player perspective
    mat4 viewMatrix = player->playerView;
    mat4 projectionMatrix = player->playerProjection;
    //portal->modelMatrix = translate(mat4(), vec3(0.0, 2.0, 0.0));
    glUniformMatrix4fv(viewMatrixLocation, 1, GL_FALSE, &viewMatrix[0][0]);
    glUniformMatrix4fv(projectionMatrixLocation, 1, GL_FALSE, &projectionMatrix[0][0]);
    glUniformMatrix4fv(modelMatrixLocation, 1, GL_FALSE, &portal1->modelMatrix[0][0]);
    portal1->draw(modelMatrixLocation);

    glUniformMatrix4fv(modelMatrixLocation, 1, GL_FALSE, &portal2->modelMatrix[0][0]);
    portal2->draw(modelMatrixLocation);
    

    //twra pou ehume grapsei ston stencil buffer sxediazoume tin skini apo thn virtual camera (portal2) alla mono sta pixels (fragments) pou ehun timi > h == tou 1 
    //grafoume stous depth kai color buffers
    

    glDepthMask(GL_TRUE); //grafoume ston depth buffer
    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE); //grafoume ston color buffer
    glStencilMask(0x00); //den grafoume ston stencil

    glEnable(GL_DEPTH_TEST);
    glStencilFunc(GL_LEQUAL, 1, 0xFF); //to STENCIL_TEST einai epitixes mono an stencil_value>=1
    

    //viewMatrix = virtualCamera1->viewMatrix;
    //projectionMatrix = virtualCamera1->projectionMatrix;
    mat4 modelMatrix = mat4(1.0f);
    glUniformMatrix4fv(viewMatrixLocation, 1, GL_FALSE, &viewMatrix[0][0]);
    glUniformMatrix4fv(projectionMatrixLocation, 1, GL_FALSE, &projectionMatrix[0][0]);

    scene->draw(modelMatrixLocation,textureSamplerLocation);
    //viewMatrix = virtualCamera2->viewMatrix;
    //projectionMatrix = virtualCamera2->projectionMatrix;
    modelMatrix = mat4(1.0f);
    glUniformMatrix4fv(viewMatrixLocation, 1, GL_FALSE, &viewMatrix[0][0]);
    glUniformMatrix4fv(projectionMatrixLocation, 1, GL_FALSE, &projectionMatrix[0][0]);

    scene->draw(modelMatrixLocation, textureSamplerLocation);
    modelMatrix = mat4(1.0f);

    //*/
    //grafoume to portal sto depth buffer giati theloume ta fragments tou portal na min sxediazontai an iparxei antikeimeno mprosta aptin kamera tou parathrhth
    //auto ginetai giati sto mainloop() den tha ksanasxediasoume to portal wste na graftei sto depthbuffer
    glDisable(GL_STENCIL_TEST);
    glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE); //den grafoume ston color buffer
    glClear(GL_DEPTH_BUFFER_BIT);
    glDepthFunc(GL_ALWAYS);

    viewMatrix = player->playerView;
    projectionMatrix = player->playerProjection;
    glUniformMatrix4fv(viewMatrixLocation, 1, GL_FALSE, &viewMatrix[0][0]);
    glUniformMatrix4fv(projectionMatrixLocation, 1, GL_FALSE, &projectionMatrix[0][0]);
    portal1->draw(modelMatrixLocation);
   
    portal2->draw(modelMatrixLocation);

    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE); //ksanaenergopoiw ton color buffer prin paw sto mainloop
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS); //xrhsimopoiw thn default depthfunction
    glDepthMask(GL_TRUE); //grafw ston depthbuffer


    return;
} //1st try

//teliki sinartisi portal image
void renderRecursivePortals(mat4 currentViewMatrix, mat4 currentProjMatrix, int max_recursion, int current_recursion)
{
    for (int i = 0; i < 2; i++)
    {
        glDepthMask(GL_FALSE); //den grafoume ston depth buffer
        glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE); //den grafoume ston color buffer
        glStencilMask(0xFF); //grafoume ston stencil buffer

        glDisable(GL_DEPTH_TEST); //theloume na perasoume to portal ston stencil 
        glEnable(GL_STENCIL_TEST);

        glStencilFunc(GL_NOTEQUAL, current_recursion, 0xFF); //ekteloume stencil tests sto fail gia na graftoun ta portals ston stencil
        glStencilOp(GL_INCR, GL_KEEP, GL_KEEP);

        glUniformMatrix4fv(viewMatrixLocation, 1, GL_FALSE, &currentViewMatrix[0][0]);
        glUniformMatrix4fv(projectionMatrixLocation, 1, GL_FALSE, &currentProjMatrix[0][0]);
        
        portals[i]->draw(modelMatrixLocation);
        //sxediasame to portal pou vlepei o currentViewMatrix
        //o currentViewMatrix sthn prwth klhsh ths sinartisis einai o playerCamera->viewMatrix 
        //stis epomenes klhseis einai h virtual camera tou destination portal. 
        mat4 nextViewMatrix = computePortalCameraViewMatrix(currentViewMatrix, portals[i], portals[i]->dst);
        //1h klhsh: metavainoume apo playerCamera se portalcamera
        //epomenes klhseis: metavainoume apo portalCamera se portalCamera epomenou epipedou recursion

        mat4 nextProjMatrix = perspective(radians(40.0f), 4.0f / 3.0f, 0.1f, 100.0f);
        if (current_recursion != max_recursion)
        {
            renderRecursivePortals(nextViewMatrix, nextProjMatrix, max_recursion, current_recursion + 1);
        }
        else //zwgrafizoume tin skini apo thn prooptiki tou eswterikou portal sto max_recursion level (mono sthn teleutaia klhsh)
        {
            glDepthMask(GL_TRUE);
            glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);

            glClear(GL_DEPTH_BUFFER_BIT);
            glEnable(GL_STENCIL_TEST);
            glEnable(GL_DEPTH_TEST);

            glStencilMask(0x00); //den grafoume ston stencil buffer alla pragmatopoioume stencil test

            glStencilFunc(GL_EQUAL, current_recursion + 1, 0xFF);
            //antikathistw me sinartiseis

            glUniformMatrix4fv(viewMatrixLocation, 1, GL_FALSE, &nextViewMatrix[0][0]);
            glUniformMatrix4fv(projectionMatrixLocation, 1, GL_FALSE, &nextProjMatrix[0][0]);
            mat4 modelMatrix = mat4(1.0f);

            switchTextureMode(2, textureSelectorLocation);
            scene->draw(modelMatrixLocation, textureSamplerLocation);
            switchTextureMode(3, textureSelectorLocation);
            scene->drawSkybox(modelMatrixLocation, textureSamplerLocation);
            switchTextureMode(0, textureSelectorLocation);
            scene->drawTrophy(modelMatrixLocation, textureSamplerLocation);
            switchTextureMode(3, textureSelectorLocation);
            if(current_recursion!=max_recursion) portals[i]->drawOutline(modelMatrixLocation, textureSamplerLocation); //sto teleutaio recursion level den zwgrafizw to outline giati tha fainetai olokliro to orthogwnio xwris portal image
            switchTextureMode(0, textureSelectorLocation);
            player->draw(modelMatrixLocation);
            for(int j=0; j<2; j++) if(checkAABBs(player->playerAABB,portals[j]->frontBoundingBox)) renderDuplicatePlayer(player,portals[j],modelMatrixLocation);
            if (ballThrown) {
                glUniformMatrix4fv(modelMatrixLocation, 1, GL_FALSE, &sphere->modelMatrix[0][0]);
                switchTextureMode(4, textureSelectorLocation); //den efarmozw skies panw stin sfaira otan exw portals giati kati buggarei
                sphere->draw();
                for (int j = 0; j < 2; j++) {
                    if (checkSphereWithPortalFrontAABBCollision(*sphere, portals[j]) ||
                        (checkSphereWithPortalBackAABBCollision(*sphere, portals[j])
                            && checkSphereWithPortalPlaneCollision(*sphere, portals[j])))
                        renderDuplicateSphere(sphere, portals[j], modelMatrixLocation);
                }
            }
        } 
        glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
        glDepthMask(GL_FALSE);

        glEnable(GL_STENCIL_TEST);
        glStencilMask(0xFF);

        glStencilFunc(GL_NOTEQUAL, current_recursion + 1, 0xFF);
        glStencilOp(GL_DECR, GL_KEEP, GL_KEEP);

        glUniformMatrix4fv(viewMatrixLocation, 1, GL_FALSE, &currentViewMatrix[0][0]);
        glUniformMatrix4fv(projectionMatrixLocation, 1, GL_FALSE, &currentProjMatrix[0][0]);
        portals[i]->draw(modelMatrixLocation);
        }

    //grafoume kai ta 2 portals ston depth buffer epeidi theloume na ginoun render swsta mazi me tin skini
    glDisable(GL_STENCIL_TEST);
    glStencilMask(0x00);
    glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE);
    glDepthFunc(GL_ALWAYS);
    glClear(GL_DEPTH_BUFFER_BIT);

    glUniformMatrix4fv(viewMatrixLocation, 1, GL_FALSE, &currentViewMatrix[0][0]);
    glUniformMatrix4fv(projectionMatrixLocation, 1, GL_FALSE, &currentProjMatrix[0][0]);
    for (int i = 0; i < 2; i++) {
        portals[i]->draw(modelMatrixLocation);
    }
    //kanume render thn skhnh sto current_recursion level me tous currentProjMatrix kai currentViewMatrix
    glDepthFunc(GL_LESS);

    glDepthMask(GL_TRUE);
    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
    glStencilMask(0x00); //den grafoume ston stencil buffer omws ekteloume stencil tests gia na elegxoume poio kommati tis othonis sxediazetai

    glEnable(GL_STENCIL_TEST);
    glEnable(GL_DEPTH_TEST);

    glStencilFunc(GL_LEQUAL, current_recursion, 0xFF);

    switchTextureMode(2, textureSelectorLocation);
    
    scene->draw(modelMatrixLocation, textureSamplerLocation);
    
    switchTextureMode(3, textureSelectorLocation);
    scene->drawSkybox(modelMatrixLocation, textureSamplerLocation);
    switchTextureMode(0, textureSelectorLocation);
    scene->drawTrophy(modelMatrixLocation, textureSamplerLocation);
    switchTextureMode(0, textureSelectorLocation);
    player->draw(modelMatrixLocation);
    for (int j = 0; j < 2; j++) if (checkAABBs(player->playerAABB, portals[j]->frontBoundingBox)) renderDuplicatePlayer(player, portals[j], modelMatrixLocation);
    switchTextureMode(3, textureSelectorLocation);
    glUniform1f(timeLocation, glfwGetTime());
    for (int i = 0; i < 2; i++) portals[i]->drawOutline(modelMatrixLocation, textureSamplerLocation);
    if (ballThrown) {
        glUniformMatrix4fv(modelMatrixLocation, 1, GL_FALSE, &sphere->modelMatrix[0][0]);
        switchTextureMode(4, textureSelectorLocation);
        sphere->draw();
        for (int i = 0; i < 2; i++) {
            if (checkSphereWithPortalFrontAABBCollision(*sphere, portals[i]) ||
                (checkSphereWithPortalBackAABBCollision(*sphere, portals[i])
                    && checkSphereWithPortalPlaneCollision(*sphere, portals[i])))
                renderDuplicateSphere(sphere, portals[i], modelMatrixLocation);
        }
    }
}

mat4 computePortalCameraViewMatrix(mat4 originalViewMatrix, Portal* sourcePortal, Portal* destinationPortal)
{
    mat4 destinationViewMatrix = originalViewMatrix * sourcePortal->modelMatrix * rotate(mat4(), 3.14f, vec3(0, 1, 0)) *
        inverse(destinationPortal->modelMatrix); //o gnwstos metasxhmatismos pou ekshgoume sthn anafora omws pollaplasiazoume apo deksia me ton antistrofo giati eimaste se sintetagmenes kameras
    return destinationViewMatrix;
}

//player camera teleportation
bool checkCameraPortalCollision(Portal* portal, vec3 prevCameraPosition, vec3 newCameraPosition) {
    //mporoume na to valoume mesa se if(distance(camera,portal)<...) mesa stin mainloop gia na veltiwsoume performance
    //elegxoume gia ta 2 trigwna tou portal
    for (int i = 0; i < 2; i++) {

        vec4 _p0 = portal->modelMatrix * vec4(portal->vertices[3 * i + 0], 1.0);
        vec4 _p1 = portal->modelMatrix * vec4(portal->vertices[3 * i + 1], 1.0);
        vec4 _p2 = portal->modelMatrix * vec4(portal->vertices[3 * i + 2], 1.0);

        vec3 p0 = vec3(_p0.x, _p0.y, _p0.z);
        vec3 p1 = vec3(_p1.x, _p1.y, _p1.z);
        vec3 p2 = vec3(_p2.x, _p2.y, _p2.z);
        
        //dianismata
        vec3 p01 = p1 - p0;
        vec3 p02 = p2 - p0;

        vec3 lineVector = newCameraPosition - prevCameraPosition;

        float denominator = dot(-lineVector, cross(p01, p02));
        if (denominator == 0.0f) continue; //parallhla
        if (denominator < 0.0f) continue; //simainei oti h fora ths kinisis einai apo to pisw meros tou portal pros to mprosta
                                          //theloume na apokleisoume auti ti periptwsi alliws tha exoume loop thlemetaforwn
        //liseis
        // prosthetoume/afairoume 1e-6 gia tixon floating point error
        float t = dot(cross(p01, p02), prevCameraPosition - p0) / denominator;
        if (t > 1.0f + 1e-6 || t < 0.0f - 1e-6) continue;

        float u = dot(cross(p02 ,-lineVector), prevCameraPosition - p0) / denominator;
        if (u > 1.0f + 1e-6 || u < 0.0f - 1e-6) continue;

        float v = dot(cross(-lineVector, p01), prevCameraPosition - p0) / denominator;
        if (v > 1.0f + 1e-6 || v < 0.0f - 1e-6) continue;
        if (u + v > 1.0f + 1e-6) continue;

        cout << "portalcollision";
        return true; //ama perasoun oloi oi elegxoi tote ehume intersection me kapoio apo ta 2 trigwna
    }
    return false; //ama den kanei return true mesa sto for loop den ehume intersection me kanena apo ta 2 trigwna
}

void teleportPlayerCamera(Portal* sourcePortal, Player& player)
{
    Portal* destination = sourcePortal->dst;
 
    vec4 newPosition = destination->modelMatrix* rotate(mat4(), 3.14f, vec3(0, 1, 0))*inverse(sourcePortal->modelMatrix)*vec4(player.x, 1.0f) ;
    vec4 newDirection = destination->modelMatrix * rotate(mat4(), 3.14f, vec3(0, 1, 0)) * inverse(sourcePortal->modelMatrix) * vec4(player.viewDirection, 0.0f);
    vec4 newVelocity= destination->modelMatrix * rotate(mat4(), 3.14f, vec3(0, 1, 0)) * inverse(sourcePortal->modelMatrix) * vec4(player.v, 0.0f);
    player.x = vec3(newPosition.x,newPosition.y,newPosition.z);
    player.viewDirection = vec3(newDirection.x, newDirection.y, newDirection.z);
    player.v = vec3(newVelocity.x, newVelocity.y, newVelocity.z);
    player.P = player.m * player.v;
    //player.playerView = computePortalCameraViewMatrix(player.playerView, sourcePortal, destination);
    //prepei na ananewsoume tis gwnies ths kameras wste na epanelthei h kamera
    player.verticalAngle = asin(player.viewDirection.y);
    vec3 n1 = vec3(sourcePortal->portalNormal.x, sourcePortal->portalNormal.y, sourcePortal->portalNormal.z);
    vec3 n2 = vec3(destination->portalNormal.x, destination->portalNormal.y, destination->portalNormal.z);
    if (n1 == n2) player.horizontalAngle += 3.14f;
    else if (n1 == -n2) player.horizontalAngle = player.horizontalAngle;
    else if (n1 == vec3(0, 0, -1) && n2 == vec3(-1, 0, 0)) player.horizontalAngle -= 3.14f / 2.0f;
    else if (n1 == vec3(0, 0, 1) && n2 == vec3(-1, 0, 0)) player.horizontalAngle += 3.14f / 2.0f;
    else  player.horizontalAngle = acos(player.viewDirection.z / cos(player.verticalAngle));

}
void renderDuplicatePlayer(Player* player, Portal* sourcePortal, GLuint modelMatrixLocation)
{
    Portal* destination = sourcePortal->dst;
    mat4 bodyRotations = rotate(mat4(), 3.14f, vec3(1, 0, 0));
    mat4 duplicateModelMatrix = destination->modelMatrix * rotate(mat4(), 3.14f, vec3(0, 1, 0)) * inverse(sourcePortal->modelMatrix) * player->modelMatrix* bodyRotations;
    glUniformMatrix4fv(modelMatrixLocation, 1, GL_FALSE, &duplicateModelMatrix[0][0]);
    player->playerBody->bind();
    player->playerBody->draw();
    
}
void renderDuplicateSphere(Sphere* sphere, Portal* sourcePortal,GLuint modelMatrixLocation)
{
    Portal* destination = sourcePortal->dst;
    mat4 duplicateModelMatrix = destination->modelMatrix * rotate(mat4(), 3.14f, vec3(0, 1, 0))* inverse(sourcePortal->modelMatrix)* sphere->modelMatrix;
    glUniformMatrix4fv(modelMatrixLocation, 1, GL_FALSE, &duplicateModelMatrix[0][0]);
    sphere->draw();
}
void teleportOriginalSphere(Sphere& sphere, Portal* sourcePortal)
{
    /*/
    Portal* destination = sourcePortal->dst;
    vec3 sourcePortalPosition = vec3(sourcePortal->position.x, sourcePortal->position.y, sourcePortal->position.z);
    vec3 n1 = vec3(sourcePortal->portalNormal.x, sourcePortal->portalNormal.y, sourcePortal->portalNormal.z);
    vec3 sourceUp = vec3(sourcePortal->portalUpVector.x, sourcePortal->portalUpVector.y, sourcePortal->portalUpVector.z);
    vec3 n2 = vec3(destination->portalNormal.x, destination->portalNormal.y, destination->portalNormal.z);
    //vec3 sourceUp = vec3(sourcePortal->portalUpVector.x, sourcePortal->portalUpVector.y, sourcePortal->portalUpVector.z);
    vec3 relativePositionVector = normalize(sourcePortalPosition -sphere.x);
    float distanceSphereSource = distance(sourcePortalPosition, sphere.x);
    vec3 rotationAxis = normalize(cross(-n1, n2));
    float rotationAngle = acos(dot(-n1, n2)); //edw paizei na einai to lathos?
    if (n1 == n2 || n1 == -n2)
    {
        vec3 up_axis = normalize(cross(n1, sourceUp));
        vec4 rotatedAxis = rotate(mat4(), 3.14f / 2.0f, up_axis) * sourcePortal->portalNormal;
        rotationAxis = vec3(rotatedAxis.x, rotatedAxis.y, rotatedAxis.z);
    }

    vec4 relativePositionDst = rotate(mat4(), rotationAngle, rotationAxis) * vec4(relativePositionVector, 1.0f);
    vec4 newPosition = destination->position - distanceSphereSource*relativePositionDst;
    vec4 newVelocity = rotate(mat4(), rotationAngle, rotationAxis) * vec4(sphere.v, 1.0f);

    sphere.x = vec3(newPosition.x, newPosition.y, newPosition.z);
    sphere.v = vec3(newVelocity.x, newVelocity.y, newVelocity.z);
    sphere.P = sphere.m * sphere.v;
    //*/
    //*/
    Portal* destination = sourcePortal->dst;

    vec4 newPosition;
    vec4 newVelocity;

    newPosition = destination->modelMatrix * rotate(mat4(), 3.14f, vec3(0, 1, 0)) * inverse(sourcePortal->modelMatrix) *vec4(sphere.x, 1.0f);
    newVelocity = destination->modelMatrix * rotate(mat4(), 3.14f, vec3(0, 1, 0)) *inverse(sourcePortal->modelMatrix) * vec4(sphere.v, 0.0f);

    sphere.x = vec3(newPosition.x, newPosition.y, newPosition.z);
    sphere.v = vec3(newVelocity.x, newVelocity.y, newVelocity.z);
    sphere.P = sphere.m * sphere.v;
    //*/

}


void free() {
    delete sphere;
    delete portal1;
    delete portal2;
    delete camera;
    delete[] portals;

    glDeleteProgram(shaderProgram);
    glDeleteProgram(depthProgram);

    glfwTerminate();
}

void createDepthTexture(mat4 viewMatrix, mat4 projectionMatrix, GLuint depthFrameBuffer) 
{ //kaleitai mesa sti mainloop kai stin renderRecursivePortals (?) sinexeia

    glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);

    glBindFramebuffer(GL_FRAMEBUFFER, depthFrameBuffer); //bind ton buffer pou ehume orisei stin createcontext

    glClear(GL_DEPTH_BUFFER_BIT);

    glUseProgram(depthProgram);

    mat4 view_projection = projectionMatrix * viewMatrix; //dinw ta light view kai projection matrix
    glUniformMatrix4fv(shadowViewProjectionLocation, 1, GL_FALSE, &view_projection[0][0]);
    mat4 sceneModelMatrix = mat4(1.0);
    //glUniformMatrix4fv(shadowModelLocation, 1, GL_FALSE, &sceneModelMatrix[0][0]);

    // ---- rendering the scene ---- //
    //edw zwgrafizw tin skini kai ola ta antikeimena ston framebuffer

    glDisable(GL_CULL_FACE);

    scene->draw(shadowModelLocation);
    glEnable(GL_CULL_FACE);
    player->draw(shadowModelLocation);
    for (int j = 0; j < 2; j++) if (checkAABBs(player->playerAABB, portals[j]->frontBoundingBox)) renderDuplicatePlayer(player, portals[j], modelMatrixLocation);
    if (ballThrown) {
        glUniformMatrix4fv(shadowModelLocation, 1, GL_FALSE, &sphere->modelMatrix[0][0]);

        sphere->draw();
        for (int j = 0; j < 2; j++) {
            if (checkSphereWithPortalFrontAABBCollision(*sphere, portals[j]) ||
                (checkSphereWithPortalBackAABBCollision(*sphere, portals[j])
                    && checkSphereWithPortalPlaneCollision(*sphere, portals[j])))
                renderDuplicateSphere(sphere, portals[j], shadowModelLocation);
        }
    }

    // binding the default framebuffer again
    
    glViewport(0, 0, W_WIDTH, W_HEIGHT);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    //*/
}

void mainLoop() {
    float t = glfwGetTime();
    vec3 intersectionPoint = vec3(0.0, 0.0, 0.0);
    static bool portal1spawned=false, portal2spawned=false;

    directionalLight->update(); //gia dimiourgia ton matrices
    //create shadow texture
    mat4 light_projection = directionalLight->projectionMatrix;
    mat4 light_view = directionalLight->viewMatrix;
    mat4 lightVP = directionalLight->lightVP();


    do {
        // calculate dt
        float currentTime = glfwGetTime();
        float dt = (currentTime - t)/2.0f;
     
        createDepthTexture(light_view, light_projection, depthFrameBuffer);

        //render rest of scene
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glUseProgram(shaderProgram);

        glUniformMatrix4fv(lightVPLocation, 1, GL_FALSE, &lightVP[0][0]); //gia shadowcalculations
        
        // arxika stelnw to shadow texture sth thesi 5 afou opote kai na sxediazetai h skhnh theloume to shadow texture ths 
        //(eite sxediazetai mesa sthn recursivePortals eite oxi)
        glActiveTexture(GL_TEXTURE5);								// Activate texture position
        glBindTexture(GL_TEXTURE_2D, depthTexture);			// Assign texture to position 
        glUniform1i(shadowMapSamplerLocation, 5);

        //kratame previous position kai current position gia na kanume line-plane intersection gia playercamera portal collision

        vec3 previousCameraPosition = player->cameraPosition; //gia teleportPlayerCamera 
        vec3 previousPlayerPosition = player->x; //gia player-wall collision detection
        vec3 n;
        bool jumptrigger = false; //an o paixths pataei sto patwma mporei na kanei jump


        player->applyGravity();
        if (checkPlayerWithSceneFloorsCollision(*player, *scene, portals, n)) { //player-floor collision==true den askeitai varitita
            player->disableGravity();
            player->v = vec3(0, 0, 0);
            player->P = player->m * player->v;
            jumptrigger = true;
        }

        player->move(jumptrigger, dt); //ananewnoume thesi

        player->update(t, dt); //ananewnoume katastasi

        //player->update(t, dt);
        
        if (portal1spawned && portal2spawned)
        {
            for (int i = 0; i < 2; i++)
            {
                if (checkCameraPortalCollision(portals[i], previousCameraPosition, player->x)) {
                    //player->disableGravity();
                    teleportPlayerCamera(portals[i], *player);
                    break; //simantiko wste an tilemetaferthei apto ena portal na min tilemetaferthei amesws kai apo to allo
                }
            }
        }
        if (checkPlayerWithSceneWallsCollision(*player, *scene, portals, n) && n != vec3(0.0f, 1.0f, 0.0f)) //an h epomenh thesi ehei wall collision epistrepse stin prohgoumenh
        {
            player->x = previousPlayerPosition;
        }
        if (player->x.y < -10.0f) player->x = vec3(0, 1, 3); //respawn

        mat4 viewMatrix = player->playerView;
        mat4 projectionMatrix = player->playerProjection;

        //*/
        if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS) {
            for (int i = 0; i < 7; i++)
            {
                vec3 wallPosition = scene->walls[i]->wallModel->vertices[0];
                vec3 wallNormal = scene->walls[i]->wallModel->normals[0];

                if (player->checkRayPlaneIntersection(wallPosition, wallNormal)) 
                {
                    intersectionPoint = player->rayCastToPlane(wallPosition, wallNormal);
                    if (scene->checkPointInsideWall(intersectionPoint, scene->walls[i])) //den douleuei kala autos o elegxos?
                    {
                        Portal::spawnPortal(*portals[0], intersectionPoint + 0.05f * wallNormal, wallNormal, window);
                        portal1spawned = true;
                    }//spawn ligo pio mprosta apo to wall
                }
            }
        }
        if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS) {
            for (int i = 0; i < 7; i++)
            {
                vec3 wallPosition = scene->walls[i]->wallModel->vertices[0];
                vec3 wallNormal = scene->walls[i]->wallModel->normals[0];

                if (player->checkRayPlaneIntersection(wallPosition, wallNormal))
                {
                    intersectionPoint = player->rayCastToPlane(wallPosition, wallNormal);
                    if (scene->checkPointInsideWall(intersectionPoint, scene->walls[i]))
                    {
                        Portal::spawnPortal(*portals[1], intersectionPoint + 0.05f * wallNormal, wallNormal, window);
                        portal2spawned = true;
                    }
                }
            }
        }

        if (glfwGetKey(window, GLFW_KEY_T) == GLFW_PRESS && !ballThrown)
        {
            sphere = new Sphere(player->cameraPosition, 4.0f * player->viewDirection, 0.4, 10);
            sphere->update(t, dt); //update gia na dimiourgithei o modelMatrix
            ballThrown = true;
        }

        if (ballThrown == true) {
            //apply gravity
            sphere->forcing = [&](float t, const vector<float>& y)->vector<float> {
                vector<float> f(6, 0.0f);
                f[1] = -9.8f * sphere->m;
                return f;
            };
            

            if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS || sphere->x.y < -20.0f)
            {
                ballThrown = false; //san reload alla katastrefei thn prohgoumenh
            }

            //check for collisions
            for (int i = 0; i < 2; i++)
            {
                
                if (checkSphereWithPortalBackAABBCollision(*sphere, portals[i])
                    && !checkSphereWithPortalPlaneCollision(*sphere, portals[i]))
                    teleportOriginalSphere(*sphere, portals[i]);
            }
            handleSphereWithSceneCollision(*sphere, *scene, portals);

            sphere->update(t, dt); //update gia na simperilavoume ta collisions
        }
        
        if (portal1spawned && portal2spawned)
        {
            renderRecursivePortals(player->playerView, player->playerProjection, 4, 0);
        }
        else //an den ehoume zeugos portals tote prepei na kanoume ksexwrista render thn skhnh 
        {

            glUniformMatrix4fv(viewMatrixLocation, 1, GL_FALSE, &viewMatrix[0][0]);
            glUniformMatrix4fv(projectionMatrixLocation, 1, GL_FALSE, &projectionMatrix[0][0]);

            switchTextureMode(1, textureSelectorLocation);
            glUniform1f(timeLocation, t);

            if (portal1spawned) portals[0]->drawOutline(modelMatrixLocation, textureSamplerLocation);
            else if (portal2spawned) portals[1]->drawOutline(modelMatrixLocation, textureSamplerLocation);

            switchTextureMode(2, textureSelectorLocation);
            scene->draw(modelMatrixLocation, textureSamplerLocation);
            switchTextureMode(3, textureSelectorLocation);
            scene->drawSkybox(modelMatrixLocation,textureSamplerLocation);

            if (ballThrown)
            {
                glUniformMatrix4fv(modelMatrixLocation, 1, GL_FALSE, &sphere->modelMatrix[0][0]);
                switchTextureMode(0, textureSelectorLocation);
                sphere->draw();
            }
        }

        t += dt;

        glfwSwapBuffers(window);
        glfwPollEvents();
    } while (glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS &&
             glfwWindowShouldClose(window) == 0);
}

void initialize() {
    // Initialize GLFW
    if (!glfwInit()) {
        throw runtime_error("Failed to initialize GLFW\n");
    }

    glfwWindowHint(GLFW_SAMPLES, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // To make MacOS happy
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // Open a window and create its OpenGL context
    window = glfwCreateWindow(W_WIDTH, W_HEIGHT, TITLE, NULL, NULL);
    if (window == NULL) {
        glfwTerminate();
        throw runtime_error(string(string("Failed to open GLFW window.") +
                            " If you have an Intel GPU, they are not 3.3 compatible." +
                            "Try the 2.1 version.\n"));
    }
    glfwMakeContextCurrent(window);

    // Start GLEW extension handler
    glewExperimental = GL_TRUE;

    // Initialize GLEW
    if (glewInit() != GLEW_OK) {
        glfwTerminate();
        throw runtime_error("Failed to initialize GLEW\n");
    }

    // Ensure we can capture the escape key being pressed below
    glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);

    // Hide the mouse and enable unlimited movement
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // Set the mouse at the center of the screen
    glfwPollEvents();
    glfwSetCursorPos(window, W_WIDTH / 2, W_HEIGHT / 2);

    // Gray background color
    glClearColor(0.5f, 0.5f, 0.5f, 0.0f);

    // Enable depth test
    glEnable(GL_DEPTH_TEST);
    // Accept fragment if it closer to the camera than the former one
    glDepthFunc(GL_LESS);

    // Cull triangles which normal is not towards the camera
    glEnable(GL_CULL_FACE);
    // glFrontFace(GL_CW);
    // glFrontFace(GL_CCW);

    // enable point size when drawing points
    glEnable(GL_PROGRAM_POINT_SIZE);

    // Log
    logGLParameters();



    // Create camera
    //camera = new Camera(window,vec3(0,0,5));
    //testCamera = new Camera(window, vec3(0, 0, 5));
}

int main(void) {
    try {
        initialize();
        createContext();
        createDepthTextureBuffer();
        mainLoop();
        free();
    } catch (exception& ex) {
        cout << ex.what() << endl;
        getchar();
        free();
        return -1;
    }

    return 0;
}