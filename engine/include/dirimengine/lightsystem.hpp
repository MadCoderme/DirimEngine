#pragma once
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "shaderProgram.h"
#include "physics.hpp"
#include "stb_image.h"
#include <math.h>
#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#define MAX_POINT_LIGHTS 64

struct DirLight {
    glm::vec4 direction;
    glm::vec4 ambient;
    glm::vec4 diffuse;
    glm::vec4 specular;
};

struct PointLight {
    glm::vec4 position;
    glm::vec4 ambient;
    glm::vec4 diffuse;
    glm::vec4 specular;
    float constant;
    float linear;
    float quadratic;
	float padding;    
};

struct LightUBO {
    int numPointLights;
    float padding[3];
    DirLight dirLight;
    PointLight pointLights[MAX_POINT_LIGHTS];
};

class LightSystem {
    unsigned int UBO;
    LightUBO lightData;
public:
    LightSystem() {
        memset(&lightData, 0, sizeof(lightData));
        glGenBuffers(1, &UBO);
        glBindBuffer(GL_UNIFORM_BUFFER, UBO);
        glBufferData(GL_UNIFORM_BUFFER, sizeof(lightData), NULL, GL_STATIC_DRAW);
        glBindBufferBase(GL_UNIFORM_BUFFER, 0, UBO);
        glBindBuffer(GL_UNIFORM_BUFFER, 0);
        lightData.numPointLights = 0;
    }

    void SetDirectionalLight(glm::vec3 direction, glm::vec3 ambient, glm::vec3 diffuse, glm::vec3 specular) {
        lightData.dirLight.direction = glm::vec4(direction, 0.0f);
        lightData.dirLight.ambient = glm::vec4(ambient, 1.0f);
        lightData.dirLight.diffuse = glm::vec4(diffuse, 1.0f);
        lightData.dirLight.specular = glm::vec4(specular, 1.0f);
    }

    void AddPointLight(glm::vec3 position, glm::vec3 ambient, glm::vec3 diffuse, glm::vec3 specular, float constant, float linear, float quadratic) {
        if (lightData.numPointLights >= MAX_POINT_LIGHTS) return;

        lightData.pointLights[lightData.numPointLights].position = glm::vec4(position, 1.0f);
        lightData.pointLights[lightData.numPointLights].ambient = glm::vec4(ambient, 1.0f);
        lightData.pointLights[lightData.numPointLights].diffuse = glm::vec4(diffuse, 1.0f);
        lightData.pointLights[lightData.numPointLights].specular = glm::vec4(specular, 1.0f);
        lightData.pointLights[lightData.numPointLights].constant = constant;
        lightData.pointLights[lightData.numPointLights].linear = linear;
        lightData.pointLights[lightData.numPointLights].quadratic = quadratic;
        lightData.pointLights[lightData.numPointLights].padding = 0.0f;
        lightData.numPointLights++;
    }

    void UpdateToGPU() {
        glBindBuffer(GL_UNIFORM_BUFFER, UBO);
        glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(lightData), &lightData);
        glBindBuffer(GL_UNIFORM_BUFFER, 0);
    }
};