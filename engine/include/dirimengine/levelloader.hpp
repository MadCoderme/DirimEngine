#pragma once
#include <vector>
#include <string>
#include <fstream>
#include <iostream>
#include "gameobject.hpp"
#include "model.hpp"

using namespace std;

class LevelLoader {
public:
    unordered_map<char, GameObject*> ObjectMap;
    GameObject* BaseObject = nullptr;
    vector<GameObject*> walls;
    vector<string> mapLayout;

    int width = 0;
    int height = 0;

    glm::vec3 playerStartPos = glm::vec3(0.0f, 5.0f, 0.0f);
    glm::vec3 enemyStartPos = glm::vec3(0.0f, -0.5f, 0.0f);
    
    float TILE_SIZE = 2.0f;

    void LoadLevel(string path, Model* wallModel, Model* floorModel) {
        ifstream file(path);
        if (!file.is_open()) {
            cout << "Failed to load level: " << path << endl;
            return;
        }

        string line;
        int row = 0;
        mapLayout.clear();

        while (getline(file, line)) {
            mapLayout.push_back(line);
            if (line.length() > width) width = line.length();

            for (int col = 0; col < line.length(); col++) {
                char tile = line[col];
                
                float posX = col * TILE_SIZE;
                float posZ = row * TILE_SIZE;
                float posY = 0.0f;

                if (tile == '.') {
                    PutBaseObject(posX, -1.0f, posZ);
                } else if (tile == 'P') {
                    playerStartPos = glm::vec3(posX, 1.0f, posZ); 
                    PutBaseObject(posX, -TILE_SIZE/2.0f, posZ);
                } else {
                    if (ObjectMap.count(tile) > 0) {
                        GameObject *obj = ObjectMap[tile]->Clone();

                        glm::vec3 finalScale = glm::vec3(TILE_SIZE);
                        glm::vec3 templateScale = ObjectMap[tile]->scale;

                        if (templateScale.x != 1.0f) finalScale.x = templateScale.x;
                        if (templateScale.y != 1.0f) finalScale.y = templateScale.y;
                        if (templateScale.z != 1.0f) finalScale.z = templateScale.z; 

                        obj->position = glm::vec3(posX, ObjectMap[tile]->position.y != 0.0f ? ObjectMap[tile]->position.y : posY, posZ);
                        obj->scale = finalScale;
                        obj->type = TEXTURE_GEOMETRY;
                        obj->id = to_string(row) + "_" + to_string(col);
                        obj->tag = ObjectMap[tile]->tag;
                        walls.push_back(obj);

                        PutBaseObject(posX, -TILE_SIZE/2.0f, posZ);
                    }
                }
            }
            row++;
        }
        height = row;
        file.close();
    }

    void PutBaseObject(float posX, float posY, float posZ) {
        if (!BaseObject) return;
        GameObject* base = new GameObject(
            BaseObject->model, 
            glm::vec3(posX, posY, posZ), 
            glm::vec3(TILE_SIZE, 1.0f, TILE_SIZE), 
            0.0f, 
            glm::vec3(1.0f, 0.0f, 0.0f)
        );
        base->tag = BaseObject->tag;
        base->type = TEXTURE_GEOMETRY;
        base->isCollisionEnabled = true;
        walls.push_back(base);
    }
};