//This shader renders planet orbital lines

#version 330 core

//each orbit vertex is a point along a circular path
layout (location = 0) in vec3 aPos; 

uniform mat4 view; //folow camera movement
uniform mat4 projection; //apply perspective

void main()
{
    gl_Position = projection * view * vec4(aPos, 1.0);
}