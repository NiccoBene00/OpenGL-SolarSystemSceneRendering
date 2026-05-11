#pragma once

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
    int vertexCount;
};