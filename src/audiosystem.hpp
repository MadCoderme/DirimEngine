#pragma once
#include <iostream>
#include <irrklang/irrKlang.h>
#include <glm/glm.hpp>

using namespace std;

class AudioSystem {
public:
    irrklang::ISoundEngine *SoundEngine = nullptr;
    AudioSystem() {
        SoundEngine = irrklang::createIrrKlangDevice();
        if (!SoundEngine) {
            cout << "Error starting sound engine" << endl;
        }
    }
    ~AudioSystem() {
        if (SoundEngine) {
            SoundEngine->drop();
        }
    }
    void Preload(const char* filePath) {
        if (SoundEngine) {
            irrklang::ISoundSource* source = SoundEngine->addSoundSourceFromFile(filePath);
            if (source) {
                source->setDefaultVolume(0.8f);
            }
        }
    }
    void Preload(const char* filePath, float volume) {
        if (SoundEngine) {
            irrklang::ISoundSource* source = SoundEngine->addSoundSourceFromFile(filePath);
            if (source) {
                source->setDefaultVolume(volume);
            }
        }
    }
    void Play2D(const char* filePath, bool loop) {
        if (SoundEngine) {
            SoundEngine->setSoundVolume(0.8f);
            SoundEngine->play2D(filePath, loop);
        }
    }
    void Play3D(const char* filePath, glm::vec3 pos, bool loop) {
        if (SoundEngine) {
            SoundEngine->setSoundVolume(1.0f);
            irrklang::vec3df position(pos.x, pos.y, pos.z);
            SoundEngine->play3D(filePath, position, loop);
        }
    }
    void UpdateListener(glm::vec3 pos, glm::vec3 lookDir, glm::vec3 upDir) {
        if (SoundEngine) {
            irrklang::vec3df position(pos.x, pos.y, pos.z);
            irrklang::vec3df look(lookDir.x, lookDir.y, lookDir.z);
            irrklang::vec3df up(upDir.x, upDir.y, upDir.z);
            SoundEngine->setListenerPosition(position, look, irrklang::vec3df(0.0f, 0.0f, 0.0f), up);
        }
    }
};