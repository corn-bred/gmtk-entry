#pragma once
#include <AL/al.h>
#include <AL/alc.h>

#include <iostream>
#include <vector>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <cornbreadlib/audiodata.h>

#define _SOURCESIZE 32

class AudioManager {
    
    public:
    ALCcontext *AudioContext = nullptr;
    ALCdevice *AudioDevice = nullptr;

    std::vector<ALuint> SourceAvailPool;
    std::vector<ALuint> SourceUsedPool;

    AudioManager();

    ~AudioManager();

    AudioManager(AudioManager &&other);
    AudioManager &operator=(AudioManager &&other);

    AudioManager(const AudioManager&) = delete;
    AudioManager& operator=(const AudioManager&) = delete;

    void PlaySound(AudioData &buffer);

    void Update();

    void SetListenerPosition(glm::vec3 Position);
    void SetListenerVelocity(glm::vec3 Velocity);
    void SetListenerOrientation(glm::vec3 forward, glm::vec3 up);

    void SetListenerGain(float Gain);
};