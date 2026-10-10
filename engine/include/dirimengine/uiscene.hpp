#pragma once
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "shaderProgram.h"
#include "ui.hpp"

enum UIElementType { TEXT, BUTTON, IMAGE };
struct UIElement {
    string name;
    UIElementType type;
    GameObject* bgObj = nullptr;
    UIText* label = nullptr;
    void (*onClick)();
};

class UIScene {
GLFWwindow* window;
public:
    vector<UIElement> elements;
    Shader uiShader;
    Shader textShader;
    TextRenderer textRenderer;

    glm::vec4 ClearColor;

    UIScene(GLFWwindow* w) : 
        uiShader("/home/abrar/Projects/gamedev/src/shaders/uishader.vs", "/home/abrar/Projects/gamedev/src/shaders/uishader.fs"),
        textShader("/home/abrar/Projects/gamedev/src/shaders/textshader.vs", "/home/abrar/Projects/gamedev/src/shaders/textshader.fs"),
        textRenderer("/home/abrar/Projects/gamedev/src/fonts/Arial.ttf")
    {
        window = w;
    }

    bool Contains(GameObject* bgObject, double mouseX, double mouseY) {
        if (!bgObject) return false;
        float halfW = bgObject->scale.x; 
        float halfH = bgObject->scale.y;
        

        return (mouseX >= bgObject->position.x - halfW && mouseX <= bgObject->position.x + halfW &&
                mouseY >= bgObject->position.y - halfH && mouseY <= bgObject->position.y + halfH);
    }

    void Update() {
        double mx, my;
        glfwGetCursorPos(window, &mx, &my);
        bool clicked = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS;

        for (UIElement &el : elements) {
            if (Contains(el.bgObj, mx, my)) {
                if (el.type == BUTTON) {
                    if (clicked && el.onClick) {
                         el.onClick();
                    }
                }
            }
        }
    }

    void Draw() {
        Update();
        glClearColor(ClearColor.x, ClearColor.y, ClearColor.z, ClearColor.w);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glDisable(GL_DEPTH_TEST);
        uiShader.use();
        for (UIElement &el : elements) {
            if (el.bgObj) el.bgObj->Draw(uiShader);
        }
        for (UIElement &el : elements) {
            if (el.label) textRenderer.RenderText(textShader, *el.label);
        }
        glEnable(GL_DEPTH_TEST);
    }
};