//This shader converts HDR colors into displayable monitor colors

#version 330 core
out vec4 FragColor; //final display color

in vec2 TexCoords;

uniform sampler2D scene; //contains HDR framebuffer image

void main()
{
    //laoad HDR color from framebuffer
    vec3 color = texture(scene, TexCoords).rgb;

    // tone mapping base
    color = color / (color + vec3(1.0));

    FragColor = vec4(color, 1.0);
}