#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include "utils/texture.h"

#include <iostream>

unsigned int loadTexture(const char* path, bool gammaCorrection)
{
    unsigned int textureID;
    glGenTextures(1, &textureID);

    int width, height, nrComponents;
    stbi_set_flip_vertically_on_load(true); 

    unsigned char* data = stbi_load(path, &width, &height, &nrComponents, 0);
    std::cout << "Loading texture: " << path << std::endl;
    if (data)
    {

        std::cout << "Loaded OK: " << width << "x" << height << " components: " << nrComponents << std::endl;
        GLenum internalFormat;
        GLenum dataFormat;

        if (nrComponents == 1)
        {
            internalFormat = dataFormat = GL_RED;
        }
        else if (nrComponents == 3)
        {
            internalFormat = gammaCorrection ? GL_SRGB : GL_RGB;
            dataFormat = GL_RGB;
        }
        else if (nrComponents == 4)
        {
            internalFormat = gammaCorrection ? GL_SRGB_ALPHA : GL_RGBA;
            dataFormat = GL_RGBA;
        }

        glBindTexture(GL_TEXTURE_2D, textureID);

        glTexImage2D(GL_TEXTURE_2D, 0, internalFormat,
                     width, height, 0, dataFormat, GL_UNSIGNED_BYTE, data);

        glGenerateMipmap(GL_TEXTURE_2D);

        // wrapping
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

        // filtering
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        stbi_image_free(data);
    }
    else
    {
        std::cout << "FAILED to load texture at path: " << path << std::endl;
        stbi_image_free(data);
    }

    return textureID;
}

// ----------------------------
// HDR TEXTURE 
// ----------------------------
unsigned int loadHDRTexture(const char* path)
{
    stbi_set_flip_vertically_on_load(true);

    int width, height, nrComponents;
    float* data = stbi_loadf(path, &width, &height, &nrComponents, 0);

    unsigned int hdrTexture;

    if (data)
    {
        glGenTextures(1, &hdrTexture);
        glBindTexture(GL_TEXTURE_2D, hdrTexture);

        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F,
                     width, height, 0, GL_RGB, GL_FLOAT, data);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        stbi_image_free(data);
    }
    else
    {
        std::cout << "Failed to load HDR texture: " << path << std::endl;
    }

    return hdrTexture;
}