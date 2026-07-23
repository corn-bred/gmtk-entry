#include "game.h"
#include "scenes.h"
#include <misc/globals.h>
#include <cornjam/inputmanager.h>
#include <cornbreadlib/audiomanager.h>

void GameState::ChangeScene(Scene *newScene) {
    PendingScene.reset(newScene); //new ownershippp
}

void GameState::ApplyPendingScene() {
    if (PendingScene) {

        if (CurrentScene) CurrentScene->Exit();

        CurrentScene = std::move(PendingScene);

        if (CurrentScene) CurrentScene->Init();

    }
}

void GameState::Init() {
    input.BindAction(Action::MoveDown, GLFW_KEY_S);
    input.BindAction(Action::MoveUp, GLFW_KEY_W);
    input.BindAction(Action::MoveLeft, GLFW_KEY_A);
    input.BindAction(Action::MoveRight, GLFW_KEY_D);
    input.BindAction(Action::Dash, GLFW_KEY_SPACE);
    ChangeScene(new PlayingScene());
}

void GameState::Play(GLFWwindow *window) {
    glfwPollEvents();
    
    audio.Update();
    input.Update(window);
    
    ApplyPendingScene();

    if (CurrentScene) {
        CurrentScene->Update();
    }

    if (CurrentScene) {
        CurrentScene->Render();
    }

    glfwSwapBuffers(window);
}