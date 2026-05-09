#version 330 core

//inputs from VAO/VBO 
layout (location = 0) in vec3 aPos; //3D position
layout (location = 1) in vec3 aNormal; //surface normal (for the texture lights)
layout (location = 2) in vec2 aTexCoords; //UV coordinates (for the texture position)

//output to the fragment shader
out vec3 Normal;
out vec3 FragPos;
out vec2 TexCoords;

uniform mat4 model; //model matrix (transform vertices  from Local Space -> World Space)
uniform mat4 view; //camera matrix (World Space -> View Space)
uniform mat4 projection; //perspective matrix (View Space -> Clip Space)

void main() //run one per vertice
{   
    //bring the vertex from local space to world space
    //it used for the computation of the lights
    FragPos = vec3(model * vec4(aPos, 1.0));
    
    //correctly transform normals under scaling/rotation
    Normal = mat3(transpose(inverse(model))) * aNormal;

    TexCoords = aTexCoords;

    //final vertex position
    gl_Position = projection * view * model * vec4(aPos, 1.0);
}