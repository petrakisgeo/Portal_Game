#include "Player.h"
#include <common/model.h>
#include <glm/gtc/matrix_transform.hpp>

#include <iostream>


using namespace glm;


bool spawned = false;



Player::Player(GLFWwindow* win,vec3 position, float w, float l, float mass,float movspeed)
{
    playerBody = new Drawable("models/stickman.obj");
    playerBox = new Drawable("models/cube.obj"); //gia to fast AABB 
    modelMatrix = mat4(1.0f);
    window = win;
    x = position;
    m = mass;
    movementspeed = movspeed; //movement speed gia to move()
    v = vec3(0.0, 0.0, 0.0); //default. Pairnei movement apto move h to update
    P = m * v;
    //"camera" parameters
    horizontalAngle = 3.14f;
    verticalAngle = 0.0f;
    mouseSpeed = 0.0015f;
    //arxiko spawn kameras sto panw meros tou model
    

    calculatePlayerAABB();
    cameraPosition = x + vec3(0.0f, 1.5f, 0.0f);
}

void Player::draw(GLuint location)
{
    playerBody->bind();
    mat4 bodyRotations= rotate(mat4(), 3.14f, vec3(1, 0, 0)); //gia to stickman.obj
    mat4 correctMatrix = modelMatrix*bodyRotations;
    glUniformMatrix4fv(location, 1, GL_FALSE, &correctMatrix[0][0]);
    playerBody->draw();

}

void Player::applyGravity() 
{
    forcing = [&](float t, const std::vector<float>& y)->std::vector<float> {
        std::vector<float> f(6, 0.0f);
        f[1] = -9.8f * m;
        return f;
    };
}
void Player::disableGravity()
{
    forcing = [&](float t, const std::vector<float>& y)->std::vector<float> {
        std::vector<float> f(6, 0.0f);
        f[1] = 0;
        return f;
    };
}

void Player::move(bool onFloor,float dt) //an onfloor ine true mono tote tha mporei na ginei jump
{
    static bool gravity=true;
    double xPos, yPos;
    glfwGetCursorPos(window, &xPos, &yPos);

    int width, height;
    glfwGetWindowSize(window, &width, &height);

    glfwSetCursorPos(window, width / 2, height / 2);

    //*/
     float newVAngle= verticalAngle + mouseSpeed * float(height / 2 - yPos);
     horizontalAngle+= mouseSpeed * float(width / 2 - xPos);
     if (abs(newVAngle) < 3.14f / 2.5f) verticalAngle = newVAngle; //elegxos gia na mhn ehoume revert
     
    vec3 direction = vec3(
        cos(verticalAngle) * sin(horizontalAngle),
        sin(verticalAngle),
        cos(verticalAngle) * cos(horizontalAngle)
    );

    viewDirection = direction; 

    vec3 directionProjXZ= direction - dot(direction, vec3(0, 1, 0)) * vec3(0, 1, 0); //gia na min petaei metakinoume kata tin provoli
    //den mporei na doulepsei to player movement me diorthwsi thesis mono me prosthiki taxititas kata to dianisma directionProjXZ
    vec3 right(
        sin(horizontalAngle - 3.14f / 2.0f),
        0,
        cos(horizontalAngle - 3.14f / 2.0f)
    );
    vec3 up = cross(right, direction);

    if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) {
        movementspeed = 6.0f; //sprint
    }
    else movementspeed = 3.0f; //walk
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
        x += movementspeed * dt* directionProjXZ;
    }
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
        x -= movementspeed * dt* directionProjXZ;
    }
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
        if (dot(v, right) < 0.5f*movementspeed)
        x += 0.5f*movementspeed*dt*right;
    }
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
        if (dot(v, right) > -0.5f * movementspeed)
        x -= 0.5f*movementspeed *dt* right;
    }
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
    {
        if(onFloor)
        v += 3.0f * vec3(0.0f,1.0f,0.0f);
    }
    if (glfwGetKey(window, GLFW_KEY_G) == GLFW_PRESS) {
        gravity = !gravity;
    }
    if (!gravity) disableGravity();
    //projection matrix stin mainloop perspective
    //kathe fora pou allazoume manually thn taxitita tou rigidbody prepei na enhmerwnoume kai thn ormh tou
    P = m * v; //ksanaipologizoume tin ormi giati tha kalestei h update

    playerProjection= perspective(radians(40.0f), 4.0f / 3.0f, 0.1f, 100.0f);
    playerView = lookAt(x+vec3(0.0f,1.5f,0.0f), //offset gia na fainetai san na koitaei apto kefali
        viewDirection + x+vec3(0.0f,1.5f,0.0f),
        up);
    //to x sto viewMatrix allazei sinexws sto update
}

void Player::update(float t, float dt) //pithanon na xreiazetai orisma portals gia na thlemetaferthei
{
    //integration
    advanceState(t, dt);

    cameraPosition = x + vec3(0.0f, 1.5f, 0.0f);
    // compute model matrix
    mat4 scale = glm::scale(mat4(), vec3(1.0, 1.0, 1.0));
    mat4 tranlation = translate(mat4(), vec3(x.x, x.y, x.z));
#ifdef USE_QUATERNIONS
    mat4 rotation = rotate(mat4(), horizontalAngle, vec3(0, 1, 0))*mat4_cast(q); //gia na akolouthei tis peristrofes tis kameras
#else
    mat4 rotation = mat4(R);
#endif
    modelMatrix = tranlation * rotation * scale;

    //calculate neo AABB me ton neo modelMatrix
    calculatePlayerAABB();
}



bool Player::checkRayPlaneIntersection(vec3 planePosition, vec3 planeNormal)
{
    float distanceFromPlane;
    double xPos, yPos;
    glfwGetCursorPos(window, &xPos, &yPos); //viewport coordinates

    //kanonikopoioume sto diastima -1,1
    float x = (2.0f * xPos) / 1024.0f - 1.0f;
    float y = 1.0f - (2.0f * yPos) / 768.0f;

    vec3 ray_nds = vec3(x, y, 1.0f);

    vec4 ray_clip = vec4(ray_nds.x, ray_nds.y, -1.0, 1.0);

    vec4 ray_eye = inverse(playerProjection) * ray_clip;

    ray_eye = vec4(ray_eye.x, ray_eye.y, -1.0, 0.0);

    vec4 ray_wor = inverse(playerView) * ray_eye;
    vec3 ray_world = vec3(ray_wor.x, ray_wor.y, ray_wor.z); //dianisma

    ray_world = normalize(ray_world);

    float denominator = dot(ray_world,planeNormal);

    if (abs(denominator) > 1e-6f)
    {

        distanceFromPlane = -dot(cameraPosition - planePosition , planeNormal) / denominator;

        //std::cout << distanceFromPlane;
    }
    else return false;

    if (distanceFromPlane == 0.0f || distanceFromPlane < 0.0f) return false;
    return true;

}


vec3 Player::rayCastToPlane(vec3 planePosition, vec3 planeNormal)
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

    vec4 ray_eye = inverse(playerProjection) * ray_clip;

    ray_eye = vec4(ray_eye.x, ray_eye.y, -1.0, 0.0);

    vec4 ray_wor = inverse(playerView) * ray_eye;
    vec3 ray_world = vec3(ray_wor.x, ray_wor.y, ray_wor.z); //dianisma

    ray_world = normalize(ray_world);

    distanceFromPlane = -dot(cameraPosition-planePosition,planeNormal) / dot(ray_world, planeNormal); //eksiswnoume dianismatikes plane kai ray equations
    intersectionPoint = cameraPosition + distanceFromPlane * ray_world;

    return intersectionPoint;
}

void Player::calculatePlayerAABB()
{
    playerAABB = new AABB; //arxikopoiw (pithano memory leak)

    std::vector<vec3> modelVertices = playerBox->indexedVertices;
    for (int i = 0; i < modelVertices.size(); i++)
    {
        //metavainw se world coordinates
        //kanw ena anomoiomorfo scale gia na einai pio lepto to box kai na efarmozei kalitera sto portal
        //metakinw mprosta ston z efoson to portal einai strameno pros to +z prin pollaplasiasw me to portal modelmatrix
        //fortwsa ta models stickman kai cube sto blender kai tsekara ta transformations pou xreiazontai wste na "efarmosei" swsta to box ston stickman
        //auto to ekana epeidi to laptop mou den mporouse na sikwsei na ipologizei to AABB apo ta 4000+ vertices tou stickman.obj kai olo to programma disleitourgouse
        vec4 worldVertex = modelMatrix * translate(mat4(), vec3(0.0f, -0.8f, 0.0f)) * scale(mat4(), vec3(1.0f, 1.85f, 1.0f)) * rotate(mat4(), 3.14f, vec3(1, 0, 0)) *
            vec4(modelVertices[i].x, modelVertices[i].y, modelVertices[i].z, 1.0f);

        float x = worldVertex.x;
        float y = worldVertex.y;
        float z = worldVertex.z;

        playerAABB->min_x = min(playerAABB->min_x, x);
        playerAABB->max_x = max(playerAABB->max_x, x);

        playerAABB->min_y = min(playerAABB->min_y, y);
        playerAABB->max_y = max(playerAABB->max_y, y);

        playerAABB->min_z = min(playerAABB->min_z, z);
        playerAABB->max_z = max(playerAABB->max_z, z);
    }

    playerAABB->center = 0.5f * vec3(playerAABB->min_x + playerAABB->max_x,
        playerAABB->min_y + playerAABB->max_y,
        playerAABB->min_z + playerAABB->max_z);

    playerAABB->extents= 0.5f * vec3(abs(playerAABB->max_x - playerAABB->min_x),
        abs(playerAABB->max_y - playerAABB->min_y),
        abs(playerAABB->max_z - playerAABB->min_z));
    /*/
    //poli argos ipologismos
    std::vector<vec3> modelVertices = playerBody->vertices;
    mat4 bodyRotations = rotate(mat4(), 3.14f, vec3(1, 0, 0)); //gia to stickman.obj
    for (int i = 0; i < modelVertices.size(); i++)
    {
        vec4 worldVertex4d = modelMatrix *bodyRotations* vec4(modelVertices[i], 1.0f);
        vec3 worldVertex = vec3(worldVertex4d.x, worldVertex4d.y, worldVertex4d.z);

        //oria tou bounding box
        playerAABB->min_x = min(playerAABB->min_x, worldVertex.x);
        playerAABB->max_x = max(playerAABB->max_x, worldVertex.x);

        playerAABB->min_y = min(playerAABB->min_y, worldVertex.y);
        playerAABB->max_y = max(playerAABB->max_y, worldVertex.y);

        playerAABB->min_z = min(playerAABB->min_z, worldVertex.z);
        playerAABB->max_z = max(playerAABB->max_z, worldVertex.z);
    }
    //*/
}





/*/
Cube::Cube(vec3 pos, vec3 vel, vec3 omega, float length, float mass)
    : RigidBody() {
    cube = new Drawable("models/cube.obj");

    l = length;
    m = mass;
    x = pos;
    v = vel;
    P = m * v;
    w = omega;

    if (l == 0) throw std::logic_error("Cube: length != 0");
    mat3 I = mat3(
        1.0f / 6 * mass * l * l, 0, 0,
        0, 1.0f / 6 * mass * l * l, 0,
        0, 0, 1.0f / 6 * mass * l * l);

    L = I * w;
    I_inv = inverse(I);
}

Cube::~Cube() {
    delete cube;
}

void Cube::draw(unsigned int drawable) {
    cube->bind();
    cube->draw();
}

void Cube::update(float t, float dt) {
    // numerical integration
    advanceState(t, dt);

    // compute model matrix
    mat4 scale = glm::scale(mat4(), vec3(l, l, l));
    mat4 tranlation = translate(mat4(), vec3(x.x, x.y, x.z));
#ifdef USE_QUATERNIONS
    mat4 rotation = mat4_cast(q);
#else
    mat4 rotation = mat4(R);
#endif
    modelMatrix = tranlation * rotation * scale;
}
//*/