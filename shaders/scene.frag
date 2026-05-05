#version 330 core

layout (location = 0) out vec4 FragColor;
layout (location = 1) out vec4 BrightColor;

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoords;

uniform sampler2D diffuseMap;
uniform sampler2D nightMap;

uniform vec3 lightPos;
uniform vec3 viewPos;

uniform int isEmissive;
uniform int hasNightMap;

void main()
{
    vec3 color = texture(diffuseMap, TexCoords).rgb;

    vec3 result;

    if (isEmissive == 1)
    {
        result = color * 10.0;
    }
    else
    {
        vec3 norm = normalize(Normal);
        vec3 lightDir = normalize(lightPos - FragPos);

        float diff = max(dot(norm, lightDir), 0.0);

        vec3 ambient = 0.1 * color;
        vec3 diffuse = diff * color;

        vec3 viewDir = normalize(viewPos - FragPos);
        vec3 reflectDir = reflect(-lightDir, norm);

        float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
        vec3 specular = vec3(0.3) * spec;

        result = ambient + diffuse + specular;

        // NIGHT LIGHTS
        if (hasNightMap == 1)
        {
            // lato in ombra
            float nightFactor = 1.0 - diff;

            vec3 nightColor = texture(nightMap, TexCoords).rgb;

            // aggiungi emission solo nella notte
            result += nightColor * nightFactor * 2.0;
        }
    }

    FragColor = vec4(result, 1.0);

    // BRIGHT PASS
    float brightness = dot(result, vec3(0.2126, 0.7152, 0.0722));

    if (brightness > 1.0)
        BrightColor = vec4(result, 1.0);
    else
        BrightColor = vec4(0.0, 0.0, 0.0, 1.0);
}