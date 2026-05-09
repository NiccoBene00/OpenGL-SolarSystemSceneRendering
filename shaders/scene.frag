#version 330 core


//this shader outputs to TWO framebuffer textures simultaneously
//indeed since the project uses HDR bloom, we need to render either normal rendered scene
//or only bright fragments 
layout (location = 0) out vec4 FragColor;
layout (location = 1) out vec4 BrightColor;

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoords;

//textures stored in GPU memory
uniform sampler2D diffuseMap;
uniform sampler2D nightMap;
uniform samplerCube skybox;

uniform int useReflection; //Enables/disables cubemap reflections

//light and camera position
uniform vec3 lightPos;
uniform vec3 viewPos;

//emissive and night toggle variables
uniform int isEmissive;
uniform int hasNightMap;

void main() //run once per pixel
{
    vec3 color = texture(diffuseMap, TexCoords).rgb;

    vec3 result;

    if (isEmissive == 1)
    {
        result = color * 10.0; //bloom activation
    }
    else //all non-emissive objects use Phong lighting
    {
        vec3 norm = normalize(Normal);
        vec3 lightDir = normalize(lightPos - FragPos);

        //diffuse meaning (dot product): if surface faces light -> bright otherwise -> dark
        float diff = max(dot(norm, lightDir), 0.0);

        vec3 ambient = 0.1 * color;
        //scales texture brightness based on light angle
        vec3 diffuse = diff * color;

        vec3 viewDir = normalize(viewPos - FragPos);
        vec3 reflectDir = reflect(-lightDir, norm);

        float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
        vec3 specular = vec3(0.3) * spec;

        result = ambient + diffuse + specular;

        // NIGHT LIGHTS
        if (hasNightMap == 1)
        {
            // darkside: 1 -> daylight, 0-> full night 
            float nightFactor = 1.0 - diff;

            //loads illuminated city texture
            vec3 nightColor = texture(nightMap, TexCoords).rgb;

            // add emission only for the night
            result += nightColor * nightFactor * 2.0;
        }
    }


    // =====================================================
    // REFLECTION
    // =====================================================
    if (useReflection == 1)
    {
        vec3 I = normalize(FragPos - viewPos); //camera direction

        vec3 R = reflect(I, normalize(Normal)); //compute reflect vector using surdace nornal

        vec3 reflection = texture(skybox, R).rgb; //sample cubemap

        // mix between planet color and reflection
        result = mix(result, reflection, 0.13);
    }


    FragColor = vec4(result, 1.0);

    // BRIGHT PASS
    float brightness = dot(result, vec3(0.2126, 0.7152, 0.0722));

    if (brightness > 1.0) //Only very bright fragments enter bloom pipeline
        BrightColor = vec4(result, 1.0);
    else
        BrightColor = vec4(0.0, 0.0, 0.0, 1.0); //Non-bright fragments are discarded from bloom
}