#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D scene; //HDR color of the scene
uniform sampler2D bloomBlur; //blurred version of bright parts

uniform bool bloom; //switch to turn on/off bloom
uniform float exposure; //control how bright is the final image

void main()
{
    //load original hdrColor
    vec3 hdrColor = texture(scene, TexCoords).rgb; 

    //load blurred glow texture
    vec3 bloomColor = texture(bloomBlur, TexCoords).rgb;

    if(bloom)
        hdrColor += bloomColor;

    // tone mapping: convert HDR values into display-readable range
    vec3 result = vec3(1.0) - exp(-hdrColor * exposure);

    // gamma correction since monitors are not non-linear devices so 
    //without gamma correction colors look too dark and lights become incorrect
    result = pow(result, vec3(1.0 / 2.2));

    //final image displayed on screen
    FragColor = vec4(result, 1.0);
}