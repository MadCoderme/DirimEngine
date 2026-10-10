#pragma once
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "shaderProgram.h"
#include "mesh.hpp"
#include "physics.hpp"
#include "stb_image.h"

#include <math.h>
#include <vector>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

using namespace std;

unsigned int TextureFromFile(const char* path, string directory);

class Model
{
private:
    vector<Mesh> meshes;
    vector<Texture> textures_loaded;
    string directory;


    Assimp::Importer importer;
    const aiScene *scene;
    void loadModel(string path)
    {
        scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_GenNormals);

        if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
        {
            cout << "ERROR::ASSIMP::" << importer.GetErrorString() << endl;
            return;
        }
        directory = path.substr(0, path.find_last_of('/'));

        boundingBox.boundsMin = glm::vec3(FLT_MAX);
        boundingBox.boundsMax = glm::vec3(-FLT_MAX);

        processNode(scene->mRootNode, scene);
    }

    void processNode(aiNode *node, const aiScene *scene)
    {
        for (int i = 0; i < node->mNumMeshes; i++)
        {
            aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
            meshes.push_back(processMesh(mesh, scene));
        }
        for (int i = 0; i < node->mNumChildren; i++)
        {
            processNode(node->mChildren[i], scene);
        }
    }

    Mesh processMesh(aiMesh *mesh, const aiScene *scene)
    {
        vector<Vertex> vertices;
        vector<unsigned int> indices;
        vector<Texture> textures;

        for (int i = 0; i < mesh->mNumVertices; i++)
        {
            Vertex vertex;

            vertex.Position = glm::vec3(mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z);
            vertex.Normal = glm::vec3(mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z);
            vertex.TexCoord = glm::vec2(0.0f, 0.0f);
            if (mesh->mTextureCoords[0])
            {
                vertex.TexCoord = glm::vec2(mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y);
            }

            boundingBox.boundsMin.x = min(boundingBox.boundsMin.x, vertex.Position.x);
            boundingBox.boundsMin.y = min(boundingBox.boundsMin.y, vertex.Position.y);
            boundingBox.boundsMin.z = min(boundingBox.boundsMin.z, vertex.Position.z);
            boundingBox.boundsMax.x = max(boundingBox.boundsMax.x, vertex.Position.x);
            boundingBox.boundsMax.y = max(boundingBox.boundsMax.y, vertex.Position.y);
            boundingBox.boundsMax.z = max(boundingBox.boundsMax.z, vertex.Position.z);

            vertices.push_back(vertex);
        }

        for (int i = 0; i < mesh->mNumFaces; i++)
        {
            for (int j = 0; j < mesh->mFaces[i].mNumIndices; j++)
            {
                indices.push_back(mesh->mFaces[i].mIndices[j]);
            }
        }

        if (mesh->mMaterialIndex >= 0)
        {
            aiMaterial *material = scene->mMaterials[mesh->mMaterialIndex];
            vector<Texture> diffuseMaps = loadMaterialTextures(material, aiTextureType_DIFFUSE, "texture_diffuse");
            textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());
            vector<Texture> specularMaps = loadMaterialTextures(material, aiTextureType_SPECULAR, "texture_specular");
            textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());
        }

        return Mesh(vertices, indices, textures);
    }

    vector<Texture> loadMaterialTextures(aiMaterial *material, aiTextureType type, string typeName)
    {
        vector<Texture> textures;
        for (unsigned int i = 0; i < material->GetTextureCount(type); i++)
        {
            aiString str;
            material->GetTexture(type, i, &str);
            bool skip = false;
            for (unsigned int j = 0; j < textures_loaded.size(); j++)
            {
                if (std::strcmp(textures_loaded[j].path.data(), str.C_Str()) == 0)
                {
                    textures.push_back(textures_loaded[j]);
                    skip = true;
                    break;
                }
            }
            if (!skip)
            {
                Texture texture;
                texture.id = TextureFromFile(str.C_Str(), directory);
                texture.type = typeName;
                texture.path = str.C_Str();
                textures.push_back(texture);
                textures_loaded.push_back(texture);
            }
        }
        return textures;
    }

public:
    AABB boundingBox;
    AABB getTransformedAABB(glm::mat4 modelMatrix) {
        glm::vec3 position = glm::vec3(modelMatrix[3]);

        glm::vec3 scale;
        scale.x = glm::length(glm::vec3(modelMatrix[0]));
        scale.y = glm::length(glm::vec3(modelMatrix[1]));
        scale.z = glm::length(glm::vec3(modelMatrix[2]));

        AABB worldBox;
        worldBox.boundsMin = (boundingBox.boundsMin * scale) + position;
        worldBox.boundsMax = (boundingBox.boundsMax * scale) + position;

        return worldBox;
    }
    Model(const char *path)
    {
        loadModel(path);
    }
    void Draw(Shader &shader)
    {
        for (int i = 0; i < meshes.size(); i++)
        {
            meshes[i].Draw(shader);
        }
    }
};

unsigned int TextureFromFile(const char *path, string directory)
{
    string filename = string(path);
    filename = directory + '/' + filename;

    unsigned int texture;
    glGenTextures(1, &texture);

    int width, height, nChannels;
    // stbi_set_flip_vertically_on_load(true);
    unsigned char *data = stbi_load(filename.c_str(), &width, &height, &nChannels, 0);

    if (data)
    {
        GLenum format;
        if (nChannels == 1)
            format = GL_RED;
        else if (nChannels == 3)
            format = GL_RGB;
        else if (nChannels == 4)
            format = GL_RGBA;

        glBindTexture(GL_TEXTURE_2D, texture);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    }
    else
    {
        cout << "Failed to load texture " << filename << endl;
    }
    stbi_image_free(data);
    return texture;
}