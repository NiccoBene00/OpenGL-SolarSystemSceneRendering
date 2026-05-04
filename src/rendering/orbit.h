#pragma once

#include <glad/glad.h>

class Orbit
{
public:
    Orbit(float radius, int segments = 100);
    void Draw();

private:
    unsigned int VAO, VBO;
    int vertexCount;
};