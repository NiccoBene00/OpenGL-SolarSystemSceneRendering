#include "model.h"

#define TINYOBJLOADER_IMPLEMENTATION
#include "tinyobjloader/tiny_obj_loader.h"
#include "utils/texture.h"

#include <iostream>

struct Vertex
{
    float position[3];
    float normal[3];
    float texcoord[2];
};

Model::Model(const char* path)
{
    tinyobj::attrib_t attrib;

    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;

    std::string warn, err;

    // directory file OBJ
    std::string objPath(path);

    std::string baseDir =
        objPath.substr(0, objPath.find_last_of("/\\") + 1);
    
    std::cout << "==============================\n";
    std::cout << "OBJ files: " << objPath << std::endl;

    std::cout << "Base dir: "
          << baseDir << std::endl;

    bool ret = tinyobj::LoadObj(
        &attrib,
        &shapes,
        &materials,
        &warn,
        &err,
        path,
        baseDir.c_str(),
        true
    );

    if (!warn.empty())
        std::cout << warn << std::endl;

    if (!err.empty())
        std::cerr << err << std::endl;

    if (!ret)
    {
        std::cerr << "Failed to load OBJ\n";
        return;
    }

    std::vector<Vertex> vertices;

    // ==========================================
    // LOAD VERTICES
    // ==========================================
    for (const auto& shape : shapes)
    {
        for (const auto& index : shape.mesh.indices)
        {
            Vertex vertex{};

            // POSITION
            vertex.position[0] =
                attrib.vertices[3 * index.vertex_index + 0];

            vertex.position[1] =
                attrib.vertices[3 * index.vertex_index + 1];

            vertex.position[2] =
                attrib.vertices[3 * index.vertex_index + 2];

            // NORMAL
            if (index.normal_index >= 0)
            {
                vertex.normal[0] =
                    attrib.normals[3 * index.normal_index + 0];

                vertex.normal[1] =
                    attrib.normals[3 * index.normal_index + 1];

                vertex.normal[2] =
                    attrib.normals[3 * index.normal_index + 2];
            }

            // TEXCOORD
            if (index.texcoord_index >= 0)
            {
                vertex.texcoord[0] =
                    attrib.texcoords[2 * index.texcoord_index + 0];

                vertex.texcoord[1] =
                    attrib.texcoords[2 * index.texcoord_index + 1];
            }

            vertices.push_back(vertex);
        }
    }

    vertexCount = vertices.size();

    // ==========================================
    // LOAD DIFFUSE TEXTURE
    // ==========================================
    diffuseTexture = 0;

    if (!materials.empty())
    {
        std::string texName =
            materials[0].diffuse_texname;

        if (!texName.empty())
        {
            std::string texPath = baseDir + texName;

            diffuseTexture = loadTexture(texPath.c_str());

            std::cout << "Loaded diffuse texture obj: "
                      << texPath << std::endl;
        }
    }

    // ==========================================
    // OPENGL BUFFERS
    // ==========================================
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);

    glBufferData(GL_ARRAY_BUFFER,
                 vertices.size() * sizeof(Vertex),
                 vertices.data(),
                 GL_STATIC_DRAW);

    // POSITION
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(
        0,
        3,
        GL_FLOAT,
        GL_FALSE,
        sizeof(Vertex),
        (void*)0
    );

    // NORMAL
    glEnableVertexAttribArray(1);

    glVertexAttribPointer(
        1,
        3,
        GL_FLOAT,
        GL_FALSE,
        sizeof(Vertex),
        (void*)(3 * sizeof(float))
    );

    // TEXCOORD
    glEnableVertexAttribArray(2);

    glVertexAttribPointer(
        2,
        2,
        GL_FLOAT,
        GL_FALSE,
        sizeof(Vertex),
        (void*)(6 * sizeof(float))
    );

    std::cout << "===============================\n";
}

void Model::Draw()
{
    if (diffuseTexture != 0)
    {
        glActiveTexture(GL_TEXTURE0);

        glBindTexture(GL_TEXTURE_2D,
                      diffuseTexture);
    }

    glBindVertexArray(VAO);

    glDrawArrays(GL_TRIANGLES,
                 0,
                 vertexCount);
}