#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <math.h>

#ifndef DIRIMENGINE_ASSET_ROOT
#define DIRIMENGINE_ASSET_ROOT "game/assets"
#endif

#include "shaderProgram.h"
#include "camera.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "objects.hpp"
#include "model.hpp"
#include "gameobject.hpp"
#include "ui.hpp"
#include "player.hpp"
#include "physics.hpp"
#include "scene.hpp"
#include "uiscene.hpp"
#include "levelloader.hpp"
#include "pathfinding.hpp"
#include "inputs.hpp"
#include "enemy.hpp"

using namespace std;


float deltaTime = 0.0f;
float lastFrame = 0.0f;

glm::vec3 lightPos(1.2f, 1.0f, 2.0f);

void framebuffer_size_callback(GLFWwindow* window, int width, int height);  
void leftClickHandler();
void pickAmmo(CollisionInfo col, GameObject* obj);

Scene* scene = nullptr;
enum Screen { MENU, GAME, DEAD };
Screen screen;
bool playerShot = false;

// Game State
int playerHealth = 100;
int playerAmmo = 20; 
int enemyCount = 0;
const int MAX_AMMO = 100;

int main() {
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(800, 600, "DirimEngine", NULL, NULL);
    if (window == NULL) {
        cout << "Failed to create GLFW window" << endl;
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        cout << "Failed to initialize GLAD" << endl;
        return -1;
    }

    glViewport(0, 0, 800, 600);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_FRAMEBUFFER_SRGB);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);  

    cout << "GLAD loaded :D" << endl;


    Plane ground = Plane();
    ground.setTexture("floor.jpg", DIRIMENGINE_ASSET_ROOT "/textures", "texture_diffuse");
    ground.activateAABB();
    Plane crosshair = Plane();
    crosshair.setTexture("crosshair.png", DIRIMENGINE_ASSET_ROOT "/textures", "texture_diffuse");

    Model cubeModel = Model(DIRIMENGINE_ASSET_ROOT "/models/brickcube/brick.obj");
    Model ammoModel = Model(DIRIMENGINE_ASSET_ROOT "/models/ammobox/ammobox.obj");
    Model gunModel = Model(DIRIMENGINE_ASSET_ROOT "/models/gun/Gun.obj");
    Model enemyModel = Model(DIRIMENGINE_ASSET_ROOT "/models/shambler/Shambler.obj");

    scene = new Scene(window);


    GameObject* wallTemplate = new GameObject(&cubeModel);
    wallTemplate->tag = "wall";
    GameObject* floorTemplate = new GameObject(&cubeModel);
    floorTemplate->tag = "floor";
    floorTemplate->scale = glm::vec3(150.0f, 1.0f, 150.0f);
    floorTemplate->position.y = -1.0f;
    floorTemplate->textureTiling = glm::vec2(150.0f, 150.0f);
    floorTemplate->type = TEXTURE_GEOMETRY;
    floorTemplate->isCollisionEnabled = true;
    GameObject* ceilingTemplate = floorTemplate->Clone();
    ceilingTemplate->tag = "ceiling";
    ceilingTemplate->position.y = 3.0f;
    GameObject* fireWallTemplate = new GameObject(&cubeModel);
    fireWallTemplate->tag = "firewall";

    wallTemplate->scale.y = 3.0f;
    wallTemplate->position.y = 1.0f;
    fireWallTemplate->scale.y = 3.0f;
    fireWallTemplate->position.y = 1.0f;
    
    Enemy* enemyTemplate = new Enemy(&enemyModel, scene); 
    enemyTemplate->scale = glm::vec3(0.015f);
    enemyTemplate->position.y = 0.0f;
    enemyTemplate->tag = "enemy";
    enemyTemplate->isCollisionEnabled = true;

    GameObject* ammoTemplate = new GameObject(&ammoModel);
    ammoTemplate->scale = glm::vec3(0.005f);
    ammoTemplate->position.y = -0.5f;
    ammoTemplate->tag = "ammo";
    ammoTemplate->isCollisionEnabled = true;

    LevelLoader loader;
    loader.ObjectMap['W'] = wallTemplate;
    loader.ObjectMap['F'] = fireWallTemplate;
    loader.ObjectMap['E'] = enemyTemplate;
    loader.ObjectMap['A'] = ammoTemplate;

    loader.LoadLevel(DIRIMENGINE_ASSET_ROOT "/levels/level1.txt", &cubeModel, &cubeModel);
    scene->pathfinder.Init(loader.width, loader.height);


    for (GameObject* obj : loader.walls) {
        if (obj->tag == "wall" || obj->tag == "firewall") {
             scene->Walls.push_back(obj);
        }
        Enemy* e = dynamic_cast<Enemy*>(obj);
        if (e) {
            e->SetupAI(&scene->camera.cameraPos, &playerHealth, &loader, &scene->Walls, &scene->pathfinder);
        }
        scene->GameObjects.push_back(obj);
        if (obj->tag == "firewall") {
            glm::vec3 lightPos = obj->position + glm::vec3(0.0f, 0.5f, 0.0f); 

            scene->lightSystem.AddPointLight(
                lightPos,
                glm::vec3(0.2f, 0.05f, 0.0f),  
                glm::vec3(1.5f, 0.8f, 0.2f),   
                glm::vec3(1.0f, 0.9f, 0.5f),   
                1.0f,
                0.35f, 
                0.07f
            );
        }
        if (obj->tag == "enemy") enemyCount++;
    }
    
    scene->camera.cameraPos = loader.playerStartPos;

    GameObject* gun = new GameObject(&gunModel, glm::vec3(0.3f, -0.25f, -0.7f), glm::vec3(1.0f), -90.0f, glm::vec3(0.0f, 1.0f, 0.0f));
    gun->id = "gun";
    gun->type = TEXTURE_GEOMETRY;
    gun->isCollisionEnabled = false;

    GameObject* crosshairObj = new GameObject(&crosshair, glm::vec3(400.0f, 300.0f, 0.0f), glm::vec3(50.0f, 50.0f, 50.0f), 0.0f, glm::vec3(1.0f, 0.0f, 0.0f));
    crosshairObj->type = UI;

    scene->GameObjects.push_back(gun);
    scene->GameObjects.push_back(crosshairObj);
    scene->GameObjects.push_back(floorTemplate);
    scene->GameObjects.push_back(ceilingTemplate);
    
    scene->Texts = {
        { "TARGET: NONE", 20.0f, 570.0f, 0.5f, glm::vec3(1.0f) }, 
        { "HEALTH 100", 20.0f, 20.0f, 0.8f, glm::vec3(0.0f, 1.0f, 0.0f) },
        { "AMMO 20", 600.0f, 20.0f, 0.8f, glm::vec3(1.0f, 0.8f, 0.0f) },
        { "ENEMY LEFT: " + to_string(enemyCount), 20.0f, 540.0f, 0.5f, glm::vec3(1.0f) },
    };

    scene->generalLitShader.use(); 
     scene->lightSystem.SetDirectionalLight(
            glm::vec3(-0.3f, -1.0f, -0.2f),
            glm::vec3(0.02f, 0.02f, 0.05f), 
            glm::vec3(0.05f, 0.05f, 0.05f), 
            glm::vec3(0.1f)                 
    );
    scene->lightSystem.UpdateToGPU();
    scene->generalLitShader.setFloat("material.shininess", 2.0f);

    scene->onLeftClick = leftClickHandler;
    scene->SetPlayerCollisionHandler("ammo", pickAmmo);

    scene->audioSystem.Preload(DIRIMENGINE_ASSET_ROOT "/audio/01-Quake.mp3", 0.2f);
    scene->audioSystem.Play2D(DIRIMENGINE_ASSET_ROOT "/audio/01-Quake.mp3", true);
    scene->audioSystem.Preload(DIRIMENGINE_ASSET_ROOT "/audio/Qtestlock4.wav");
    scene->audioSystem.Preload(DIRIMENGINE_ASSET_ROOT "/audio/Qtestguncock.wav");
    scene->audioSystem.Preload(DIRIMENGINE_ASSET_ROOT "/audio/Sdeath.wav");
    scene->audioSystem.Preload(DIRIMENGINE_ASSET_ROOT "/audio/Melee1.wav");
    scene->audioSystem.Preload(DIRIMENGINE_ASSET_ROOT "/audio/Noammo.wav");

    glBindVertexArray(0);
    
    screen = MENU;
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    int MenuOption = 1;
    UIScene MenuScene = UIScene(window);
    MenuScene.ClearColor = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
    UIText titleText = { "DirimEngine", 270, 400, 1.0f, glm::vec3(1.0f) };
    UIText startText = { "Start", 360, 340, 0.6f, glm::vec3(1.0f, 1.0f, 0.0f) };
    UIText settingsText = { "Settings", 350, 290, 0.5f, glm::vec3(1.0f) };
    MenuScene.elements.push_back({ "title", TEXT, nullptr, &titleText });
    MenuScene.elements.push_back({ "start", TEXT, nullptr, &startText });
    MenuScene.elements.push_back({ "settings", TEXT, nullptr, &settingsText });

    UIScene DeadScene = UIScene(window);
    DeadScene.ClearColor = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
    UIText deadText = { "You ded", 300, 400, 1.0f, glm::vec3(0.9f, 0.0f, 0.0f) };
    UIText reloadText = { "Press R to restart", 280, 350, 0.6f, glm::vec3(1.0f, 1.0f, 0.0f) };
    DeadScene.elements.push_back({ "dead", TEXT, nullptr, &deadText });
    DeadScene.elements.push_back({ "reload", TEXT, nullptr, &reloadText });

    Input input = Input(window);

    // Main Loop
    while (!glfwWindowShouldClose(window)) {
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

        input.Update();

        glfwPollEvents();

        if (screen == MENU) {
            MenuScene.Draw();
            if(input.GetKeyDown(GLFW_KEY_UP)) {
                MenuScene.elements[MenuOption].label->scale = 0.5f;
                MenuScene.elements[MenuOption].label->color = glm::vec3(1.0f);
                MenuOption = MenuOption == 1 ? 2 : MenuOption - 1;
                MenuScene.elements[MenuOption].label->scale = 0.6f;
                MenuScene.elements[MenuOption].label->color = glm::vec3(1.0f, 1.0f, 0.0f);
            }
            if(input.GetKeyDown(GLFW_KEY_DOWN)) {
                MenuScene.elements[MenuOption].label->scale = 0.5f;
                MenuScene.elements[MenuOption].label->color = glm::vec3(1.0f);
                MenuOption = MenuOption == 2 ? 1 : MenuOption + 1;
                MenuScene.elements[MenuOption].label->scale = 0.6f;
                MenuScene.elements[MenuOption].label->color = glm::vec3(1.0f, 1.0f, 0.0f);
            }
            if (input.GetKeyDown(GLFW_KEY_ENTER)) {
                if (MenuOption == 1) screen = GAME;
            }
        }
        else if (screen == GAME)
        {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
            glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            if (playerHealth <= 0) {
                screen = DEAD;
            }

            gun->setInitModelMat(glm::inverse(scene->camera.view));
            scene->audioSystem.UpdateListener(scene->camera.cameraPos, scene->camera.cameraFront, scene->camera.cameraUp);
            scene->Draw(deltaTime);


            scene->Texts[2].text = "AMMO " + to_string(playerAmmo);
            scene->Texts[2].color = (playerAmmo == 0) ? glm::vec3(1.0f, 0.0f, 0.0f) : glm::vec3(1.0f, 0.8f, 0.0f);

            scene->Texts[1].text = "HEALTH " + to_string(playerHealth);
            if (playerHealth > 50) scene->Texts[1].color = glm::vec3(0.2f, 1.0f, 0.2f);
            else if (playerHealth > 25) scene->Texts[1].color = glm::vec3(1.0f, 0.5f, 0.0f);
            else scene->Texts[1].color = glm::vec3(1.0f, 0.0f, 0.0f);

            if (playerShot) {
                scene->audioSystem.Play2D(DIRIMENGINE_ASSET_ROOT "/audio/Qtestguncock.wav", false);
                scene->AddGunRecoil();
                Ray r = Ray(scene->camera.cameraPos, scene->camera.cameraFront);
                
                bool hitSomething = false;
                for (GameObject* obj : scene->GameObjects) {
                    if (obj->tag == "enemy" && obj->isAlive) {
                        if (ray_intersect(r, obj->getWorldAABB())) {
                            obj->updateHealth(-15);
                            if (!obj->isAlive) {
                                scene->audioSystem.Play3D(DIRIMENGINE_ASSET_ROOT "/audio/Sdeath.wav", obj->position, false);
                                enemyCount--;
                                scene->Texts[3].text = "ENEMY LEFT: " + to_string(enemyCount);
                            }
                            scene->Texts[0].text = "TARGET: HIT";
                            hitSomething = true;
                            break; // Only hit one enemy
                        }
                    }
                }
                if (!hitSomething) scene->Texts[0].text = "TARGET: MISS";
                
                playerShot = false;
            }
        } else {
            DeadScene.Draw();
            
            if (input.GetKeyDown(GLFW_KEY_R)) {
                screen = GAME;
                playerHealth = 100;
                playerAmmo = 20;
                scene->camera.cameraPos = loader.playerStartPos;
                
                for(GameObject* obj : scene->GameObjects) {
                    if(obj->tag == "enemy") {
                        Enemy* e = dynamic_cast<Enemy*>(obj);
                        if(e) {
                            e->health = 100;
                            e->isAlive = true;
                        }
                    }
                }
            }
        }
        
        glfwSwapBuffers(window);
    }
    
    delete scene;
    glfwTerminate();
    return 0;
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}

void leftClickHandler() {
    if (playerAmmo > 0) {
        playerShot = true;
        playerAmmo--;
    } else {
        scene->audioSystem.Play2D(DIRIMENGINE_ASSET_ROOT "/audio/Noammo.wav", false);
    }
}

void pickAmmo(CollisionInfo col, GameObject* obj) {
    playerAmmo += 20;
    if (playerAmmo > MAX_AMMO) playerAmmo = MAX_AMMO;
    
    obj->isAlive = false;
    obj->isCollisionEnabled = false;
    scene->audioSystem.Play2D(DIRIMENGINE_ASSET_ROOT "/audio/Qtestlock4.wav", false);
}