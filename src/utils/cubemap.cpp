
#include "cubemap.h"
#include "stb_image.h"

#include <glad/glad.h>
#include <iostream>

unsigned int loadCubemap(std::vector<std::string> faces)
{
    unsigned int textureID;

    //allocate GPU texture object
    glGenTextures(1, &textureID);


    glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

    int width, height, nrChannels;

    for (unsigned int i = 0; i < faces.size(); i++)
    {
        unsigned char* data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
    
        std::cout << "Loading cubemap texture: " << faces[i] << std::endl;
        

        if (data)
        {
            //upload face to GPU, note that the target is GL_TEXTURE_CUBE_MAP_POSITIVE_X + i
            //since the faces are ordered in the vector as right, left, top, bottom, front, back
            //so auto-incrementing the target with i allows us to upload each face to the correct cubemap face
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
                0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);

            stbi_image_free(data);
        }
        else
        {
            std::cout << "Failed to load cubemap: " << faces[i] << std::endl;
            stbi_image_free(data);
        }
    }

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    return textureID;
}