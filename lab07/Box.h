#ifndef BOX_H
#define BOX_H

#include <glm/glm.hpp>

class Drawable;

/**
 * Represents the bounding box
 */
class Box {
public:
    Drawable* cube;
    float width,height;
    glm::vec3 position;
    glm::mat4 modelMatrix;


    Box(glm::vec3 p,float w,float h);
    ~Box();

    void draw(unsigned int drawable = 0);
    void update(float t = 0, float dt = 0);
};

#endif
