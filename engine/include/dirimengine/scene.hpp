#pragma once
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "shaderProgram.h"
#include "camera.h"
#include "model.hpp"
#include "gameobject.hpp"
#include "ui.hpp"
#include "player.hpp"
#include "physics.hpp"
#include "lightsystem.hpp"
#include "audiosystem.hpp"
#include "skybox.hpp"
#include "pathfinding.hpp"

class Scene {

public:
    Shader generalLitShader;
    Shader uiShader;
    Shader textShader;
    TextRenderer textRenderer;

    vector<GameObject*> GameObjects;
    vector<UIText> Texts;
    GLFWwindow* window;
    Camera camera;
    Player player;
    LightSystem lightSystem;
    SkyBox* skybox = nullptr;
    AudioSystem audioSystem;

    Pathfinder pathfinder;
    vector<GameObject*> Walls; 
    int pathfindRequestsThisFrame = 0; 
    const int MAX_PATH_REQUESTS = 2; 

    float gunRecoilPitch = 0.0f;
    float gunRecoilZ = 0.0f;

    unordered_map<string, void(*)(CollisionInfo, GameObject*)> playerCollisionHandlers;

    void (*onLeftClick)();
    float lastX = 400, lastY = 300, firstMouse = true;
    float xOffset, yOffset;

    Scene(GLFWwindow* w) : window(w),
          // Initialize Shaders here so they load AFTER context is created
          generalLitShader("/home/abrar/Projects/gamedev/src/shaders/shader.vs", "/home/abrar/Projects/gamedev/src/shaders/shader.fs"), 
          uiShader("/home/abrar/Projects/gamedev/src/shaders/uishader.vs", "/home/abrar/Projects/gamedev/src/shaders/uishader.fs"),
          textShader("/home/abrar/Projects/gamedev/src/shaders/textshader.vs", "/home/abrar/Projects/gamedev/src/shaders/textshader.fs"),
          textRenderer("/home/abrar/Projects/gamedev/src/fonts/Arial.ttf"),
          camera(),
          player(&camera, w),
          audioSystem(),
          lightSystem()
    {
        glfwSetWindowUserPointer(window, this);
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        glfwSetCursorPosCallback(window, dispatchMouseMovement);
        glfwSetMouseButtonCallback(window, dispatchMouseClick);
        glfwSetScrollCallback(window, dispatchScroll);

        movementSwayX = 0.0f;
        movementSwayY = 0.0f;

        unsigned int lightSystemIndex = glGetUniformBlockIndex(generalLitShader.ID, "lightUBO");
        glUniformBlockBinding(generalLitShader.ID, lightSystemIndex, 0);

        string path = "/home/abrar/Projects/gamedev/src/textures/skybox/";
        vector<string> faces = {
            path + "right.png",
            path + "left.png",
            path + "top.png",
            path + "bottom.png",
            path + "front.png",
            path + "back.png"
        };
        skybox = new SkyBox(faces);
    }

    void AddGunRecoil() {
        gunRecoilZ = 0.1f; 
        
        gunRecoilPitch = 10.0f;
    }

    float movementSwayX, movementSwayY, bobMagnitude, walkTimer;
    void Draw(float deltaTime) {
        pathfindRequestsThisFrame = 0;
        player.Update(deltaTime);
        player.setPlayerAABB();

        for (GameObject* obj : GameObjects) {
            if (obj->isAlive) {
                obj->Update(deltaTime);
            }
        }
        
        bool shouldFall = true;
        for (GameObject* obj : GameObjects) {
            if (obj->type == TEXTURE_GEOMETRY && obj->isCollisionEnabled) {
                CollisionInfo col = checkCollision(player.boundingBox, obj->getWorldAABB());
                if (col.IsColliding) {
                    camera.cameraPos += col.Correction;
                    player.setPlayerAABB();
                    camera.updateViewMatrix();
                    
                    if (col.MinOverlap == 0) player.Velocity.x = 0.0f;
                    else if (col.MinOverlap == 1) {
                        player.Velocity.y = 0.0f;
                        if (col.Correction.y > 0.0f) {
                            shouldFall = false;
                            player.IsFreeFall = false;
                            float groundHeight = obj->getWorldAABB().boundsMax.y;
                            camera.cameraPos.y = groundHeight + player.EYE_LEVEL;
                            player.setPlayerAABB(); 
                        }
                    }
                    else if (col.MinOverlap == 2) player.Velocity.z = 0.0f;

                    if (playerCollisionHandlers.count(obj->tag) > 0) playerCollisionHandlers[obj->tag](col, obj);
                }
            }
        }

        if (shouldFall) player.IsFreeFall = true;
        camera.updateViewMatrix();
        GameObject* gun = GetObjectById("gun"); 

        if (gun) {
            gunRecoilPitch = glm::mix(gunRecoilPitch, 0.0f, 10.0f * deltaTime);
            gunRecoilZ = glm::mix(gunRecoilZ, 0.0f, 10.0f * deltaTime);

            float smoothSpeed = 10.0f; 
            float swayAmount = 0.02f; 
            float maxSway = 0.06f;

            float targetX = -xOffset * swayAmount; 
            float targetY = -yOffset * swayAmount; 

            targetX = std::max(-maxSway, std::min(maxSway, targetX));
            targetY = std::max(-maxSway, std::min(maxSway, targetY));

            movementSwayX = movementSwayX + (targetX - movementSwayX) * smoothSpeed * deltaTime;
            movementSwayY = movementSwayY + (targetY - movementSwayY) * smoothSpeed * deltaTime;

            bool isMoving = (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS || 
                     glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS ||
                     glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS ||
                     glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS);
            
            float targetBob = (isMoving && !player.IsFreeFall) ? 1.0f : 0.0f;

            bobMagnitude += (targetBob - bobMagnitude) * 10.0f * deltaTime;

            if (isMoving) {
                walkTimer += deltaTime * 8.0f;
            } else {
                walkTimer = 0.0f; 
            }

            float bobX = sin(walkTimer) * 0.01f * bobMagnitude; 
            float bobY = cos(walkTimer * 2.0f) * 0.005f * bobMagnitude;

            glm::mat4 gunMat = glm::inverse(camera.view);
            gunMat = glm::translate(gunMat, glm::vec3(movementSwayX + bobX, movementSwayY + bobY, 0.0f));
            gunMat = glm::translate(gunMat, glm::vec3(0.3f, -0.25f, -0.7f));
            gunMat = glm::translate(gunMat, glm::vec3(0.0f, 0.0f, gunRecoilZ));
            gunMat = glm::rotate(gunMat, glm::radians(gunRecoilPitch), glm::vec3(1.0f, 0.0f, 0.0f));
            gunMat = glm::rotate(gunMat, glm::radians(-90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
            gun->setInitModelMat(glm::mat4(1.0f)); 
            gun->setModelMatrix(gunMat);
        }

        xOffset = 0.0f;
        yOffset = 0.0f;
        
        generalLitShader.use();
        generalLitShader.setVec3("viewPos", camera.cameraPos);
        generalLitShader.setMat4("view", camera.view);
        generalLitShader.setMat4("projection", camera.projection);

        for (GameObject *obj : GameObjects) {
            if (obj->type == TEXTURE_GEOMETRY) {
                obj->Draw(generalLitShader);
            }
        }
        
        for (GameObject *obj : GameObjects) {
            if (obj->type == UI) {
                obj->Draw(uiShader);
            }
        }
        for (UIText &txt : Texts) {
            textRenderer.RenderText(textShader, txt);
        }
    }

    GameObject* GetObjectById(string id) {
        for (int i = 0; i < GameObjects.size(); i++) {
            if (GameObjects[i]->id == id) return GameObjects[i];
        }
        return nullptr;
    }
    vector<GameObject*> GetObjectByTag(string tag) {
        vector<GameObject*> objs;
        for (int i = 0; i < GameObjects.size(); i++) {
            if (GameObjects[i]->tag == tag) objs.push_back(GameObjects[i]);
        }
        return objs;
    }
    void RemoveObject(string id) {
        for (int i = 0; i < GameObjects.size(); i++) {
            if (GameObjects[i]->id == id) {
                swap(GameObjects[i], GameObjects.back());
                GameObjects.pop_back();
            }
        }
    }
    void SetPlayerCollisionHandler(string tag, void (*handler)(CollisionInfo, GameObject*)) {
        playerCollisionHandlers[tag] = handler;
    }

    void onMouseMove(double xpos, double ypos) {
        if (firstMouse)
        {
            lastX = xpos;
            lastY = ypos;
            firstMouse = false;
        }
        xOffset = xpos - lastX;
        yOffset = lastY - ypos;;
        lastX = xpos;
        lastY = ypos;

        const float sensitivity = 0.08f;
        xOffset *= sensitivity;
        yOffset *= sensitivity;

        camera.handleCameraDir(xOffset, yOffset);
    }

    void onMouseScroll(double xoffset, double yoffset)
    {
        camera.handleCameraZoom(yoffset);
    }

    static void dispatchMouseMovement(GLFWwindow* window, double xpos, double ypos) {
        Scene* scene = static_cast<Scene*>(glfwGetWindowUserPointer(window));
        if (scene) {
            scene->onMouseMove(xpos, ypos);
        }
    }

    static void dispatchScroll(GLFWwindow* window, double xoffset, double yoffset) {
        Scene* scene = static_cast<Scene*>(glfwGetWindowUserPointer(window));
        if (scene) {
            scene->onMouseScroll(xoffset, yoffset);
        }
    }

    static void dispatchMouseClick(GLFWwindow* window, int button, int action, int mods) {
        Scene* scene = static_cast<Scene*>(glfwGetWindowUserPointer(window));

        if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS)
            scene->onLeftClick();
    }

    ~Scene() {}
};