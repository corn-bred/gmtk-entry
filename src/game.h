#pragma once
#include "scenes.h"
#include <misc/globals.h>
#include <memory>
#include <cornbreadlib/audiodata.h>

class GameState {
    private:
    std::unique_ptr<Scene> CurrentScene;
    std::unique_ptr<Scene> PendingScene;

    bool isRunning = true;

    public:

    void ChangeScene(Scene *newScene);

    void ApplyPendingScene();

    void Init();

    void Play(GLFWwindow *window);
};