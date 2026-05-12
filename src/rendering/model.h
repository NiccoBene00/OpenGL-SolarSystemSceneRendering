#pragma once
#include <string>
#include <vector>
#include <glad/glad.h>
#include <glm/glm.hpp>

class Model
{
public:
    Model(const char* path);
    void Draw();

private:
    unsigned int VAO, VBO;
    unsigned int diffuseTexture;
    int vertexCount;
};