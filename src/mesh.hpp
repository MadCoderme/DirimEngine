#pragma once
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "shaderProgram.h"
#include "physics.hpp"
#include <math.h>
#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

using namespace std;

struct Vertex {
    glm::vec3 Position;
    glm::vec3 Normal;
    glm::vec2 TexCoord;
};

struct Texture {
    unsigned int id;
    string type;
    string path;
};

class Mesh {
private:
    unsigned int VAO, VBO, EBO;
    void setupMesh()
    {
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        glGenBuffers(1, &EBO);
    
        glBindVertexArray(VAO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);

        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0], GL_STATIC_DRAW);  

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), 
                    &indices[0], GL_STATIC_DRAW);

        // vertex positions
        glEnableVertexAttribArray(0);	
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
        // vertex normals
        glEnableVertexAttribArray(1);	
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Normal));
        // vertex texture coords
        glEnableVertexAttribArray(2);	
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, TexCoord));

        glBindVertexArray(0);
    }
public:
    vector<Vertex> vertices;
    vector<unsigned int> indices;
    vector<Texture> textures;
    int maxUnits;
    AABB boundingBox;

    Mesh(vector<Vertex> vertices, vector<unsigned int> indices, vector<Texture> textures)
    {
        this->vertices = vertices;
        this->indices = indices;
        this->textures = textures;


        glGetIntegerv(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS, &maxUnits);
        setupMesh();
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

    void activateAABB() {
        if (vertices.empty()) return;
        boundingBox = { glm::vec3(FLT_MAX), glm::vec3(-FLT_MAX) };

        for (Vertex vertex : vertices) {
            boundingBox.boundsMin.x = min(boundingBox.boundsMin.x, vertex.Position.x);
            boundingBox.boundsMin.y = min(boundingBox.boundsMin.y, vertex.Position.y);
            boundingBox.boundsMin.z = min(boundingBox.boundsMin.z, vertex.Position.z);
            boundingBox.boundsMax.x = max(boundingBox.boundsMax.x, vertex.Position.x);
            boundingBox.boundsMax.y = max(boundingBox.boundsMax.y, vertex.Position.y);
            boundingBox.boundsMax.z = max(boundingBox.boundsMax.z, vertex.Position.z);
        }

        const float EPSILON = 0.05f; 
        if (abs(boundingBox.boundsMax.x - boundingBox.boundsMin.x) < EPSILON) {
            boundingBox.boundsMax.x += EPSILON; 
            boundingBox.boundsMin.x -= EPSILON;
        }
        if (abs(boundingBox.boundsMax.y - boundingBox.boundsMin.y) < EPSILON) {
            boundingBox.boundsMax.y += EPSILON; 
            boundingBox.boundsMin.y -= EPSILON;
        }
        if (abs(boundingBox.boundsMax.z - boundingBox.boundsMin.z) < EPSILON) {
            boundingBox.boundsMax.z += EPSILON; 
            boundingBox.boundsMin.z -= EPSILON;
        }
    }

    void Draw(Shader &shader) {
        unsigned int diffuseN = 1;
        unsigned int specularN = 1;

        for (int i = 0; i < textures.size() && i < maxUnits; i++) {
            glActiveTexture(GL_TEXTURE0 + i);
            string name = textures[i].type, number;
            if (name == "texture_diffuse") 
                number = to_string(diffuseN++);
            else if (name == "texture_specular")
                number = to_string(specularN++);

            shader.setInt(("material." + name + number).c_str(), i);
            glBindTexture(GL_TEXTURE_2D, textures[i].id); 
        }
        if (specularN == 1) {
            glActiveTexture(GL_TEXTURE0 + textures.size());
            glBindTexture(GL_TEXTURE_2D, 0);
            shader.setInt("material.texture_specular1", textures.size());
        }
        glActiveTexture(GL_TEXTURE0);

        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);
    }
};