#pragma once
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <vector>
#include <iterator>
#include <algorithm>
#include <cfloat>

#include "mesh.hpp"
#include "model.hpp"

const float Cube[] = {
    // Back face (z = -0.5)
    -0.5f, -0.5f, -0.5f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f,
    0.5f, -0.5f, -0.5f, 1.0f, 0.0f, 0.0f, 0.0f, -1.0f,
    0.5f, 0.5f, -0.5f, 1.0f, 1.0f, 0.0f, 0.0f, -1.0f,
    0.5f, 0.5f, -0.5f, 1.0f, 1.0f, 0.0f, 0.0f, -1.0f,
    -0.5f, 0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 0.0f, -1.0f,
    -0.5f, -0.5f, -0.5f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f,

    // Front face (z = 0.5)
    -0.5f, -0.5f, 0.5f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f,
    0.5f, -0.5f, 0.5f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f,
    0.5f, 0.5f, 0.5f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f,
    0.5f, 0.5f, 0.5f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f,
    -0.5f, 0.5f, 0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f,
    -0.5f, -0.5f, 0.5f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f,

    // Left face (x = -0.5)
    -0.5f, 0.5f, 0.5f, 1.0f, 0.0f, -1.0f, 0.0f, 0.0f,
    -0.5f, 0.5f, -0.5f, 1.0f, 1.0f, -1.0f, 0.0f, 0.0f,
    -0.5f, -0.5f, -0.5f, 0.0f, 1.0f, -1.0f, 0.0f, 0.0f,
    -0.5f, -0.5f, -0.5f, 0.0f, 1.0f, -1.0f, 0.0f, 0.0f,
    -0.5f, -0.5f, 0.5f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f,
    -0.5f, 0.5f, 0.5f, 1.0f, 0.0f, -1.0f, 0.0f, 0.0f,

    // Right face (x = 0.5)
    0.5f, 0.5f, 0.5f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f,
    0.5f, 0.5f, -0.5f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f,
    0.5f, -0.5f, -0.5f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f,
    0.5f, -0.5f, -0.5f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f,
    0.5f, -0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
    0.5f, 0.5f, 0.5f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f,

    // Bottom face (y = -0.5)
    -0.5f, -0.5f, -0.5f, 0.0f, 1.0f, 0.0f, -1.0f, 0.0f,
    0.5f, -0.5f, -0.5f, 1.0f, 1.0f, 0.0f, -1.0f, 0.0f,
    0.5f, -0.5f, 0.5f, 1.0f, 0.0f, 0.0f, -1.0f, 0.0f,
    0.5f, -0.5f, 0.5f, 1.0f, 0.0f, 0.0f, -1.0f, 0.0f,
    -0.5f, -0.5f, 0.5f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f,
    -0.5f, -0.5f, -0.5f, 0.0f, 1.0f, 0.0f, -1.0f, 0.0f,

    // Top face (y = 0.5)
    -0.5f, 0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f,
    0.5f, 0.5f, -0.5f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f,
    0.5f, 0.5f, 0.5f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f,
    0.5f, 0.5f, 0.5f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f,
    -0.5f, 0.5f, 0.5f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f,
    -0.5f, 0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f
};

const float PlaneVertices[] = {
    -0.5f, -0.5f, 0.0f,   0.0f, 0.0f, 1.0f,   0.0f, 0.0f,
     0.5f, -0.5f, 0.0f,   0.0f, 0.0f, 1.0f,   1.0f, 0.0f,
     0.5f,  0.5f, 0.0f,   0.0f, 0.0f, 1.0f,   1.0f, 1.0f,
    -0.5f,  0.5f, 0.0f,   0.0f, 0.0f, 1.0f,   0.0f, 1.0f
};

const int PlaneIndices[] = {
    0, 1, 2,
    2, 3, 0
};

class Plane : public Mesh {
private:
    AABB boundingBox;
    static vector<Vertex> getVertices() {
        vector<Vertex> vertices;
        for (int i = 0; i < sizeof(PlaneVertices)/sizeof(float); i+=8) {
            Vertex vertex = { 
                glm::vec3(PlaneVertices[i], PlaneVertices[i+1], PlaneVertices[i+2]),
                glm::vec3(PlaneVertices[i+3], PlaneVertices[i+4], PlaneVertices[i+5]),
                glm::vec2(PlaneVertices[i+6], PlaneVertices[i+7])
            };
            vertices.push_back(vertex);
        }
        return vertices;
    }
    static vector<unsigned int> getIndices() {
        return vector<unsigned int>(begin(PlaneIndices), end(PlaneIndices));
    }
public:
    Plane () : Mesh(getVertices(), getIndices(), {}) {
        // do nothing
        boundingBox.boundsMin = glm::vec3(-0.5f, -0.5f, -0.01f);
        boundingBox.boundsMax = glm::vec3(0.5f,  0.5f,  0.01f);
    }
    void setTexture(const char* path, string directory, string typeName) {
        Texture texture;
        texture.id = TextureFromFile(path, directory);
        texture.type = typeName;
        texture.path = path;
        textures.push_back(texture);
    }
    void setTexture(unsigned int id, string typeName) {
        Texture texture;
        texture.id = id;
        texture.type = typeName;
        texture.path = '/' + to_string(id);
        textures.push_back(texture);
    }
    AABB getTransformedAABB(glm::mat4 modelMatrix) {
        glm::vec3 min = boundingBox.boundsMin;
        glm::vec3 max = boundingBox.boundsMax;

        glm::vec3 corners[8] = {
            glm::vec3(min.x, min.y, min.z),
            glm::vec3(min.x, min.y, max.z),
            glm::vec3(min.x, max.y, min.z),
            glm::vec3(min.x, max.y, max.z),
            glm::vec3(max.x, min.y, min.z),
            glm::vec3(max.x, min.y, max.z),
            glm::vec3(max.x, max.y, min.z),
            glm::vec3(max.x, max.y, max.z)
        };

        AABB worldBox;
        worldBox.boundsMin = glm::vec3(FLT_MAX);
        worldBox.boundsMax = glm::vec3(-FLT_MAX);

        for (int i = 0; i < 8; i++) {
            glm::vec4 transformed = modelMatrix * glm::vec4(corners[i], 1.0f);

            worldBox.boundsMin.x = std::min(worldBox.boundsMin.x, transformed.x);
            worldBox.boundsMin.y = std::min(worldBox.boundsMin.y, transformed.y);
            worldBox.boundsMin.z = std::min(worldBox.boundsMin.z, transformed.z);

            worldBox.boundsMax.x = std::max(worldBox.boundsMax.x, transformed.x);
            worldBox.boundsMax.y = std::max(worldBox.boundsMax.y, transformed.y);
            worldBox.boundsMax.z = std::max(worldBox.boundsMax.z, transformed.z);
        }

        return worldBox;
    }
};

// class Cube : public Mesh {
// private:
//     static vector<Vertex> getVertices() {
//         vector<Vertex> vertices;
//         for (int i = 0; i < sizeof(PlaneVertices)/sizeof(float); i+=8) {
//             vertices.push_back({ 
//                 glm::vec3(PlaneVertices[i], PlaneVertices[i+1], PlaneVertices[i+2]),
//                 glm::vec3(PlaneVertices[i+3], PlaneVertices[i+4], PlaneVertices[i+5]),
//                 glm::vec2(PlaneVertices[i+6], PlaneVertices[i+7])
//             });
//         }
//         return vertices;
//     }
//     static vector<unsigned int> getIndices() {
//         return vector<unsigned int>(begin(PlaneIndices), end(PlaneIndices));
//     }
// public:
//     Cube () : Mesh(getVertices(), getIndices(), {}) {
//         // do nothing
//     }
//     void setTexture(const char* path, string directory, string typeName) {
//         Texture texture;
//         texture.id = TextureFromFile(path, directory);
//         texture.type = typeName;
//         texture.path = path;
//         textures.push_back(texture);
//     }
//     void setTexture(unsigned int id, string typeName) {
//         Texture texture;
//         texture.id = id;
//         texture.type = typeName;
//         texture.path = '/' + to_string(id);
//         textures.push_back(texture);
//     }
// };