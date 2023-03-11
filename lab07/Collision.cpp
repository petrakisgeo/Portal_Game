#include "Collision.h"
#include "Box.h"
#include "Sphere.h"
#include "Portal.h"
#include "Scene.h"
#include "Player.h"
#include <iostream>
#include <iostream>


using namespace glm;

void handleBoxSphereCollision(Box& box, Sphere& sphere);
bool checkForBoxSphereCollision(glm::vec3 &pos, const float& r,
                                const float& size, glm::vec3& n);
void handleSphereWithSphereCollision(Sphere& sphere1, Sphere& sphere2);
bool checkSphereWithSphereCollision(vec3& sphere1center, vec3& sphere2center, vec3& n, float sphere1radius, float sphere2radius);

void handleSphereWithPortalCollision(Portal* portal,Sphere& original,Sphere& duplicate); 
bool checkSphereWithPortalFrontAABBCollision(Sphere& sphere, Portal* portal);
bool checkSphereWithPortalBackAABBCollision(Sphere& sphere, Portal* portal);
bool checkSphereWithPortalPlaneCollision(Sphere& sphere, Portal* portal);

void handleSphereWithSceneCollision(Sphere& sphere, Scene& scene,Portal** portals);
bool checkSphereWithSceneCollision(Sphere& sphere, Scene& scene,Portal** portals, vec3& collisionVector);

void handlePlayerWithSceneCollision(Player& player, Scene& scene);
bool checkPlayerWithSceneWallsCollision(Player& player, Scene& scene, Portal** portals, glm::vec3& collisionVector);
bool checkPlayerWithSceneFloorsCollision(Player& player, Scene& scene, Portal** portals, glm::vec3& collisionVector);
//voithitikes
vec3 projectPointToWall(vec3 point, Wall* wall);
bool checkPointInsideWall(vec3 point, Wall* wall);
bool checkAABBs(AABB* a, AABB* b);

float max(float x, float y);
float min(float x, float y);

void handleBoxSphereCollision(Box& box, Sphere& sphere) {
    vec3 n; //normal epipedou sigrousis (monadiaio epistrefetai apo checkCollision()
    if (checkForBoxSphereCollision(sphere.x, sphere.r, box.width, n)) {
        // Task 2b: define the velocity of the sphere after the collision
        sphere.v = sphere.v - n * glm::dot(sphere.v, n) * 1.99f; //eswteriko ginomeno h sinistwsa ths taxititas katheti me to epipedo sigrousis
        sphere.P = sphere.m * sphere.v;
    } //stin taxitita to 2.0f einai elastiki sigrusi to 1.0f einai plastiki sigrousi ta endiamesa einai san "aporrofisi"
} //afairw ligo taxitita me 1.99f gia na diorthwsw to sfalma taxititas

bool checkForBoxSphereCollision(vec3 &pos, const float& r,
                                const float& size, vec3& n) {
    if (pos.x - r <= 0) {
        //correction
        float dis = -(pos.x - r);
        pos = pos + vec3(dis, 0, 0);

        n = vec3(-1, 0, 0);
    } else if (pos.x + r >= size) {
        //correction
        float dis = size - (pos.x + r);
        pos = pos + vec3(dis, 0, 0);

        n = vec3(1, 0, 0);
    } else if (pos.y - r <= 0) {
        //correction
        float dis = -(pos.y - r);
        pos = pos + vec3(0, dis, 0);

        n = vec3(0, -1, 0);
    } else if (pos.y + r >= size) {
        //correction
        float dis = size - (pos.y + r);
        pos = pos + vec3(0, dis, 0);

        n = vec3(0, 1, 0);
    } else if (pos.z - r <= 0) {
        //correction
        float dis = -(pos.z - r);
        pos = pos + vec3(0, 0, dis);

        n = vec3(0, 0, -1);
    } else if (pos.z + r >= size) {
        //correction
        float dis = size - (pos.z + r);
        pos = pos + vec3(0, 0, dis);

        n = vec3(0, 0, 1);
    } else {
        return false;
    }

    return true;
}

void handleSphereWithSphereCollision(Sphere& sphere1, Sphere& sphere2) {
    vec3 n; //normal epipedou sigrousis (monadiaio epistrefetai apo checkCollision())
    if (checkSphereWithSphereCollision(sphere1.x, sphere2.x,n,sphere1.r,sphere2.r)) {
        
        sphere1.v = sphere1.v + n * glm::dot(sphere1.v, n) * 2.00f;
        sphere1.P = sphere1.m * sphere1.v;
        sphere2.v = sphere2.v - n * glm::dot(sphere2.v, n) * 2.00f;
        sphere2.P = sphere2.m * sphere2.v;
    } 
} 

bool checkSphereWithSphereCollision(vec3&  sphere1center, vec3& sphere2center, vec3& n, float sphere1radius, float sphere2radius)
{
    float distanceOfCenters = distance(sphere1center, sphere2center); //apostasi kentrwn
    float radiusSum = sphere1radius + sphere2radius; //athroisma aktinwn
    vec3 collisionVector = normalize(sphere1center - sphere2center); //dianisma epipedou sigrousis 

    if (distanceOfCenters < radiusSum)
    {
        sphere1center = sphere1center + (radiusSum - distanceOfCenters) * collisionVector; //diorthwsi thesis kata tin dieuthinsi tou collision vector ( pou ehei kateuthinsi pros thn sfera 1)
        sphere2center = sphere2center - (radiusSum - distanceOfCenters) * collisionVector;
        return true;
    }
    else return false;
}

float max(float x, float y) {
    if (x > y) return x;
    else return y;
}

float min(float x, float y) {
    if (x < y) return x;
    else return y;
}

bool checkSphereWithPortalFrontAABBCollision(Sphere& sphere, Portal* portal)
{
    float sqDist = 0.0f;
    float b_min[3] = { portal->frontBoundingBox->min_x,
        portal->frontBoundingBox->min_y,
        portal->frontBoundingBox->min_z
    };
    float b_max[3] = { portal->frontBoundingBox->max_x,
        portal->frontBoundingBox->max_y,
        portal->frontBoundingBox->max_z
    };
    
    for (int i = 0; i < 3; i++) {
        // for each axis count any excess distance outside box extents
        float v = sphere.x[i];
        if (v < b_min[i]) sqDist += (b_min[i] - v) * (b_min[i] - v);
        if (v > b_max[i]) sqDist += (v - b_max[i]) * (v - b_max[i]);
    }
    return sqDist < sphere.r* sphere.r;

}

bool checkSphereWithPortalBackAABBCollision(Sphere& sphere, Portal* portal) {
    float sqDist = 0.0f;
    float b_min[3] = { portal->backBoundingBox->min_x,
        portal->backBoundingBox->min_y,
        portal->backBoundingBox->min_z
    };
    float b_max[3] = { portal->backBoundingBox->max_x,
        portal->backBoundingBox->max_y,
        portal->backBoundingBox->max_z
    };

    for (int i = 0; i < 3; i++) {
        // for each axis count any excess distance outside box extents
        float v = sphere.x[i];
        if (v < b_min[i]) sqDist += (b_min[i] - v) * (b_min[i] - v);
        if (v > b_max[i]) sqDist += (v - b_max[i]) * (v - b_max[i]);
    }
    return sqDist < sphere.r* sphere.r;
}

bool checkSphereWithPortalPlaneCollision(Sphere& sphere, Portal* portal)
{
    vec3 planePoint = vec3(portal->position.x, portal->position.y, portal->position.z);
    vec3 planeNormal = vec3(portal->portalNormal.x, portal->portalNormal.y, portal->portalNormal.z);

    float dist = dot(sphere.x, planeNormal);

    if (abs(dist) < sphere.r) return true;
    else return false;
}



void handleSphereWithSceneCollision(Sphere& sphere, Scene& scene, Portal** portals) {

    vec3 n;
    if (checkSphereWithSceneCollision(sphere, scene,portals, n))
    {
        sphere.v = sphere.v -  1.6f * dot(sphere.v, n) * n; 
        sphere.P = sphere.m * sphere.v;
    }
}

bool checkSphereWithSceneCollision(Sphere& sphere, Scene& scene,Portal** portals, vec3& collisionVector)
{
    for (int i = 0; i<7 ; i++)
    { 
        bool inBounds = false;
        bool ignoreBecauseOfPortal = false;
        std::vector<vec3> vertices = scene.walls[i]->wallModel->vertices; //4 shmeia pou orizoun to wall (oxi indexedVertices)
        vec3 normal = scene.walls[i]->wallModel->normals[0]; //opoiodhpote afou ola ta vertices twn walls ehoun idia normals

        for (int j = 0; j < 2; j++)
        {
            //an h sfera einai mesa sto mprostino h sto pisw kouti enos portal
            //tote o toixos pou ehei idio prosanatolismo me to portal den ehei collisions
            //wste na mporei na thlemetaferthei h sfaira
            vec3 portalNormal = vec3(portals[j]->portalNormal.x, portals[j]->portalNormal.y, portals[j]->portalNormal.z);
            if ((checkSphereWithPortalFrontAABBCollision(sphere, portals[j]) ||
                checkSphereWithPortalBackAABBCollision(sphere, portals[j])) &&
                portalNormal == normal)
            {
                ignoreBecauseOfPortal = true;
                break;
            }
        }
        if (ignoreBecauseOfPortal) continue; 


        vec3 A = vertices[0];
        vec3 B = vertices[1];
        vec3 C = vertices[2];
        vec3 D = vertices[3];
        
        float dist = dot(sphere.x-A, normal); //provoli tou dianismatos pou enwnei to kentro ths sfairas me tixaio shmeio tou epipedou
        //panw sto kanoniko dianisma tou epipedou tou wall

        vec3 projectedSphereCenter = sphere.x-dist*normal; //provalw to kentro ths sfairas afairwntas thn sinistwsa parallhlh me to normal
        inBounds = scene.checkPointInsideWall(projectedSphereCenter, scene.walls[i]);

        /*/
        //den douleuei kala
        vec3 AB = B - A;
        vec3 AC = C - A;
        vec3 AP = projectedSphereCenter - A;

        if (0.0f < dot(AP, AB) && dot(AP, AB) < dot(AB, AB) && 0.0f < dot(AP, AC) && dot(AP, AC) < dot(AC, AC)) inBounds = true;
        //if (0.0f <= dot(AB, AP) && dot(AB, AP) <= dot(AB, AB)) inBounds1 = true;
        //if (0.0f <= dot(AC, AP) && dot(AC, AP) <= dot(AC, AC)) inBounds2 = true;
        //*/

        if (inBounds && abs(dist) <= sphere.r && !ignoreBecauseOfPortal) {
            //an to kentro provlithei sto wall kai h provoli einai mesa sto rectangle 
            //kai to kentro apexei apostasi apo to plane mikroteri ths aktinas to
            collisionVector = normal; 
            sphere.x = sphere.x + collisionVector * (sphere.r-(dist+0.001f)); //diorthwsi thesis
            return true;
        }
    }
    return false;
}

void handlePlayerWithSceneCollision(Player& player, Scene& scene, Portal** portals)
{
    vec3 n;
    if (checkPlayerWithSceneWallsCollision(player, scene, portals, n) && n == vec3(0.0f,1.0f,0.0f))
    {
        player.v.y = 0;
        player.disableGravity();
    }
    else if (checkPlayerWithSceneWallsCollision(player, scene, portals, n) && n != vec3(0.0f, 1.0f, 0.0f))
    {

    }

}
bool checkPlayerWithSceneWallsCollision(Player& player, Scene& scene, Portal** portals, glm::vec3& collisionVector)
{
    //an aabb center ine inbounds me to checkinsidewall
    //an !ignorebecauseofportal
    for (int i = 0; i < 7; i++)
    {
        bool inBounds =false;
        bool ignoreBecauseOfPortal = false;
        std::vector<vec3> vertices = scene.walls[i]->wallModel->vertices; //4 shmeia pou orizoun to wall (oxi indexedVertices
        vec3 randomWallVertex = vertices[0];
        vec3 normal = scene.walls[i]->wallModel->normals[0]; //opoiodhpote afou ola ta vertices twn walls ehoun idia normals
        //*/
        vec3 center = player.playerAABB->center;
        vec3 e = player.playerAABB->extents;

        for (int j = 0; j < 2; j++)
        {
            vec3 portalNormal = vec3(portals[j]->portalNormal.x, portals[j]->portalNormal.y, portals[j]->portalNormal.z);
            AABB* frontAABB = portals[j]->frontBoundingBox;
            AABB* backAABB = portals[j]->backBoundingBox;
            if ((checkAABBs(frontAABB,player.playerAABB) ||
                checkAABBs(backAABB,player.playerAABB)) &&
                portalNormal == normal)
            {
                ignoreBecauseOfPortal = true;
                break;
            }
        }
        if (ignoreBecauseOfPortal) continue;

        float dist = dot(randomWallVertex-center, normal); //apostasi kentrou AABB apo wallPlane

        vec3 projectedCenter = center - dist * normal; //akrivws idia logiki me sphere-wall collision provalw to kentro tou box ston toixo alliws kanw aplws plane-aabb collision
        inBounds = scene.checkPointInsideWall(projectedCenter, scene.walls[i]);

        float r = abs(dot(e, normal)); //provoli tou extents panw sto portalNormal gia na paroume kapoio apo ta x,y,z

        if (inBounds && !ignoreBecauseOfPortal && abs(dist) <= r)
        {
            //cout << "collision";
            collisionVector = normal;
            if (abs(dist) <= r / 1.1f) player.x = player.x + collisionVector * (r - dist); //manual debugging se periptwsi pou kollhseis mesa se wall me teleport
            return true;
        }
    }
    return false;
}

bool checkPlayerWithSceneFloorsCollision(Player& player, Scene& scene, Portal** portals, glm::vec3& collisionVector)
{
    for (int i = 0; i < 2; i++)
    {
        bool inBounds = false;
        bool ignoreBecauseOfPortal = false;
        std::vector<vec3> vertices = scene.floors[i]->wallModel->vertices; //4 shmeia pou orizoun to wall (oxi indexedVertices
        vec3 randomWallVertex = vertices[0];
        vec3 normal = scene.floors[i]->wallModel->normals[0]; //opoiodhpote afou ola ta vertices twn walls ehoun idia normals
        //*/
        vec3 center = player.playerAABB->center;
        vec3 e = player.playerAABB->extents;

        for (int j = 0; j < 2; j++)
        {
            vec3 portalNormal = vec3(portals[j]->portalNormal.x, portals[j]->portalNormal.y, portals[j]->portalNormal.z);
            AABB* frontAABB = portals[j]->frontBoundingBox;
            AABB* backAABB = portals[j]->backBoundingBox;
            if ((checkAABBs(frontAABB, player.playerAABB) ||
                checkAABBs(backAABB, player.playerAABB)) &&
                portalNormal == normal)
            {
                ignoreBecauseOfPortal = true;
                break;
            }
        }
        if (ignoreBecauseOfPortal) continue;

        float dist = dot(randomWallVertex - center, normal); //apostasi kentrou AABB apo wallPlane

        vec3 projectedCenter = center - dist * normal; //akrivws idia logiki me sphere-wall collision provalw to kentro tou box ston toixo alliws kanw aplws plane-aabb collision
        inBounds = scene.checkPointInsideWall(projectedCenter, scene.floors[i]);

        float r = abs(dot(e, normal)); //provoli tou extents panw sto portalNormal gia na paroume kapoio apo ta x,y,z

        if (inBounds && !ignoreBecauseOfPortal && abs(dist) <= r)
        {
            //cout << "collision";
            collisionVector = normal;
            if(abs(dist)<=r/1.2f) player.x = player.x + collisionVector * (r-dist); //diorthwsi thesis se periptwsi pou xwthei o paixths sto patwma (debugging bia orismena portal positions h ipsiles taxitites)
            return true;
        }
    }
    return false;
}

    bool checkAABBs(AABB * a, AABB * b)
    {
        return (a->min_x <= b->max_x && a->max_x >= b->min_x) &&
            (a->min_y <= b->max_y && a->max_y >= b->min_y) &&
            (a->min_z <= b->max_z && a->max_z >= b->min_z);
    }