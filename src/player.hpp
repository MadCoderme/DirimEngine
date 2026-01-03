#pragma once
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "shaderProgram.h"
#include "camera.h"
#include "model.hpp"

class Player {

bool isFps = true;
float recoilVelocity = 0.0f; 
float accumulatedRecoil = 0.0f; 
public:
    const float PLAYER_WIDTH = 0.8f;
    const float PLAYER_HEIGHT = 1.8f;
    const float EYE_LEVEL = 1.6f; 
    const float GRAVITY = 9.8f;
    const float TERMINAL_VELOCITY = -20.0f;
    const float WALKING_SPEED = 2.5f;

    Camera* cam;
    GLFWwindow* window;
    AABB boundingBox;

    glm::vec3 Position;
    unsigned int Mass;
    glm::vec3 Velocity = glm::vec3(2.5f, 0.0f, 2.5f);
    bool IsFreeFall = false;

    Player(Camera* camera, GLFWwindow* w) {
        isFps = true;
        cam = camera;
        window = w;
        setPlayerAABB();
        // for later
    }
    void AddRecoil(float amount) {
        recoilVelocity += amount;
    }
    void Update(float deltaTime) {
        glm::vec3 v = Velocity * deltaTime;
        glm::vec3 moveDir = glm::vec3(0.0f);

        glm::vec3 forward = glm::normalize(glm::vec3(cam->cameraFront.x, 0.0f, cam->cameraFront.z));
        glm::vec3 right = glm::normalize(glm::cross(forward, glm::vec3(0.0f, 1.0f, 0.0f)));

        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) moveDir += forward;
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) moveDir -= forward;
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) moveDir -= right;
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) moveDir += right;

        if (glm::length(moveDir) > 0.0f) {
            v.x = WALKING_SPEED * deltaTime;
            v.z = WALKING_SPEED * deltaTime;
            cam->cameraPos += glm::normalize(moveDir) * v;
        }

        if (IsFreeFall) {
            Velocity.y -= GRAVITY * deltaTime;
        } 

        if (Velocity.y < TERMINAL_VELOCITY) {
            Velocity.y = TERMINAL_VELOCITY;
        }

        cam->cameraPos.y += Velocity.y * deltaTime;

        if (!IsFreeFall && glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
            Velocity.y = 3.0f;
            IsFreeFall = true;
            cam->cameraPos.y += 0.1f;
        }

        if (recoilVelocity > 0.01f) {
            float oldPitch = cam->pitch;
            float kickAmount = recoilVelocity * deltaTime;
            cam->handleCameraDir(0.0f, kickAmount);
            float pitchDelta = cam->pitch - oldPitch;
            accumulatedRecoil += pitchDelta;
            recoilVelocity -= recoilVelocity * 15.0f * deltaTime;
        }
        else if (accumulatedRecoil > 0.001f) {
            float oldPitch = cam->pitch;
            float recoverAmount = accumulatedRecoil * 5.0f * deltaTime;

            cam->handleCameraDir(0.0f, -recoverAmount);

            float pitchDelta = oldPitch - cam->pitch; 
            accumulatedRecoil -= pitchDelta;
        }
        else {
            accumulatedRecoil = 0.0f;
        }

        setPlayerAABB();
        cam->updateViewMatrix();
    }
    void setPlayerAABB() {
        glm::vec3 feetPos = cam->cameraPos;
        feetPos.y -= EYE_LEVEL;

        boundingBox.boundsMax = {
            feetPos.x + (PLAYER_WIDTH/2),
            feetPos.y + PLAYER_HEIGHT,
            feetPos.z + (PLAYER_WIDTH/2)
        };
        boundingBox.boundsMin = {
            feetPos.x - (PLAYER_WIDTH/2),
            feetPos.y,
            feetPos.z - (PLAYER_WIDTH/2)
        };
    }

};