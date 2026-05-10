#pragma once

#include <glad/glad.h>

class Orbit
{
public:
    Orbit(float radius, int segments = 100); //segments are used to approximate the circle with a polygon
    //indeed a circle is approximated using many connected line segments
    void Draw();

private:
    unsigned int VAO, VBO;
    int vertexCount;
};