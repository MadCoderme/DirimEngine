#pragma once
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <cstring>
#define MAX_KEYS 512

class Input {
GLFWwindow* window;
public:
    inline static bool keys[MAX_KEYS];
    inline static bool keysLast[MAX_KEYS];

    Input(GLFWwindow* w) {
        window = w;
        memset(keys, 0, sizeof(bool) * MAX_KEYS);
        memset(keysLast, 0, sizeof(bool) * MAX_KEYS);
        glfwSetKeyCallback(window, KeyCallback);
    }
    void Update() {
        for (int i = 0; i < MAX_KEYS; i++) {
            keysLast[i] = keys[i];
        }
    }

    bool GetKey(int key) {
        if (key < 0 || key > MAX_KEYS) return false;
        return keys[key];
    }
    bool GetKeyDown(int key) {
        if (key < 0 || key > MAX_KEYS) return false;
        return keys[key] && !keysLast[key];
    }
    bool GetKeyUp(int key) {
        if (key < 0 || key > MAX_KEYS) return false;
        return !keys[key] && keysLast[key];
    }

    static void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
        if (action == GLFW_PRESS) {
            keys[key] = true;
        } else if (action == GLFW_RELEASE) {
            keys[key] = false;
        }
    }
};