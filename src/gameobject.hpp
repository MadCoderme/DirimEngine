#pragma once
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <vector>
#include <iterator>

#include "shaderProgram.h"
#include "mesh.hpp"
#include "model.hpp"

enum ObjectTypeEnum {
    GEOMETRY,
    TEXTURE_GEOMETRY,
    ENTITY,
    UI
};

class GameObject {
private:
    glm::mat4 initialModelMat = glm::mat4(1.0f);
    bool useMatrixOverride = false;
    glm::mat4 overrideMatrix = glm::mat4(1.0f);
    glm::mat4 cachedMatrix;
    bool isDirty = true;
public:
    string id;
    string tag;
    
    ObjectTypeEnum type;
    Model* model = nullptr;
    Mesh* mesh = nullptr;

    glm::vec3 position = glm::vec3(0.0f);
    glm::vec3 scale = glm::vec3(1.0f);
    float rotationAngle = 0.0f;
    glm::vec3 rotationVector = glm::vec3(1.0f, 0.0f, 0.0f);
    glm::vec2 textureTiling = glm::vec2(1.0f, 1.0f);

    int health = 100;
    bool isAlive = true;
    bool isCollisionEnabled = true;

    GameObject (Model* m) : model(m) {}
    GameObject(Model* m, glm::vec3 pos, glm::vec3 sc, float rot, glm::vec3 rotVect) 
        : model(m), position(pos), scale(sc), rotationAngle(rot), rotationVector(rotVect) {}
    GameObject (Mesh* m) : mesh(m) {}
    GameObject(Mesh* m, glm::vec3 pos, glm::vec3 sc, float rot, glm::vec3 rotVect) 
        : mesh(m), position(pos), scale(sc), rotationAngle(rot), rotationVector(rotVect) {}

    virtual ~GameObject() {}
    virtual void Update(float deltaTime) {}

    void setInitModelMat(glm::mat4 mat) {
        initialModelMat = mat;
    }
    void setModelMatrix(glm::mat4 mat) {
        useMatrixOverride = true;
        overrideMatrix = mat;
    }
    glm::mat4 getModelMatrix() {
        if (useMatrixOverride) return overrideMatrix;
        if (!isDirty) return cachedMatrix;
        cachedMatrix = glm::mat4(1.0f);
        cachedMatrix = glm::translate(cachedMatrix, position);
        cachedMatrix = glm::rotate(cachedMatrix, glm::radians(rotationAngle), rotationVector);
        cachedMatrix = glm::scale(cachedMatrix, scale);
        isDirty = false;
        return cachedMatrix;
    }
    void SetPosition(glm::vec3 pos) {
        position = pos;
        isDirty = true;
    }
    void SetScale(glm::vec3 sc) {
        scale = sc;
        isDirty = true;
    }
    void SetRotation(float rotAngle, glm::vec3 rotVec) {
        rotationAngle = rotAngle;
        rotationVector = rotVec;
        isDirty = true;
    }

    AABB getWorldAABB() {
        if (mesh) return mesh->getTransformedAABB(getModelMatrix());
        else return model->getTransformedAABB(getModelMatrix());
    }

    void updateHealth(int delta) {
        health += delta;
        if (health <= 0) {
            isAlive = false;
            isCollisionEnabled = false;
        }
    }
    void Draw(Shader &shader) {
        shader.use();
        shader.setVec2("tiling", textureTiling);

        if (type == UI) {
            glDisable(GL_DEPTH_TEST);
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            glm::mat4 projection = glm::ortho(0.0f, 800.0f, 0.0f, 600.0f, -1.0f, 1.0f);
            shader.setMat4("projection", projection);
            shader.setMat4("model", getModelMatrix());

            if (model) model->Draw(shader);
            else if (mesh) mesh->Draw(shader);

            glEnable(GL_DEPTH_TEST);
            glDisable(GL_BLEND);
            return;
        }
        if (isAlive) {
            shader.setMat4("model", getModelMatrix());
            if (model) model->Draw(shader);
            else if (mesh) mesh->Draw(shader);
        }
    }

    virtual GameObject* Clone() {
        GameObject* clone = new GameObject(this->model);
        if (this->mesh) clone->mesh = this->mesh;
        
        clone->type = this->type;
        clone->tag = this->tag;
        clone->position = this->position;
        clone->scale = this->scale;
        clone->rotationAngle = this->rotationAngle;
        clone->rotationVector = this->rotationVector;
        clone->isCollisionEnabled = this->isCollisionEnabled;
        clone->textureTiling = this->textureTiling;
        return clone;
    }

};