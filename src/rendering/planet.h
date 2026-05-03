#pragma once

#include <glm/glm.hpp>
#include <string>

struct Planet
{
    std::string name;

    float radius;          // visible scale
    float distance;        // distance from the sun
    float orbitSpeed;      // orbit speed
    float rotationSpeed;   // rotation on its own axis

    unsigned int textureID;

    glm::vec3 position;
};