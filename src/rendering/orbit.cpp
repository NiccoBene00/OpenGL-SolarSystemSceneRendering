#include "orbit.h"
#include <vector>
#include <cmath>

Orbit::Orbit(float radius, int segments)
{
    std::vector<float> vertices;

    for (int i = 0; i < segments; ++i)
    {
        float angle = 2.0f * 3.1415926f * i / segments;

        float x = cos(angle) * radius;
        float z = sin(angle) * radius;

        vertices.push_back(x);
        vertices.push_back(0.0f);
        vertices.push_back(z);
    }

    vertexCount = segments;

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER,
        vertices.size() * sizeof(float),
        vertices.data(),
        GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE,
        3 * sizeof(float), (void*)0);
}

void Orbit::Draw()
{
    glBindVertexArray(VAO);
    glDrawArrays(GL_LINE_LOOP, 0, vertexCount);
}