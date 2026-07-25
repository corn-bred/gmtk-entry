#include "scenes.h"
#include "game.h"          
#include <cornjam/inputmanager.h>  
#include <misc/globals.h>

//PlayingScene

void PlayingScene::Init() {
    Time = 0.0f;   
    LastTime = 0.0f;
    TimeLeft = 10;
    CurrentState = Screen::Playing;

    MainPlayer = new Player(glm::vec3(0.0, -500.0, 0.0), 0.0, glm::vec2(200.0), glm::vec2(0.0), glm::vec2(10000.0), glm::vec2(0.8), glm::vec2(50), glm::vec2(1.0), glm::vec2(0.0), glm::vec2(50), glm::vec2(-10, -10), glm::vec2(30), 4000.0, 1.5, 0.5, 0.75, 1.0);
    
    PlayerAnimation = new Animation(0, 39, 0.02, true, "res/img/player.png", 40, 2);

    for (int i = 0; i < 30; i++) {
        Enemies.push_back(new Enemy(glm::vec3(rand() % 100 - 50, rand() % 100 - 50, 0.0), 0.0, 125.0, glm::vec2(0.0), glm::vec2(10000.0), glm::vec2(0.8), glm::vec2(50), glm::vec2(1.0), glm::vec2(0.0), glm::vec2(50), 1.5, 2250.0, 1.5f, 3.5f));
    }

    Generators[0] = new Generator(glm::vec2(1000, 0.0));
    Generators[1] = new Generator(glm::vec2(-1000, 0.0));
    Generators[2] = new Generator(glm::vec2(0.0, 1000));
    Generators[3] = new Generator(glm::vec2(0.0, -1000));

    WorldGrid = new GridSpace(glm::vec2(50), glm::vec3(0.0));
    for (int i = -22; i <= 22; i++) {
        WorldGrid->AddSquare(i, 22);
    }
    for (int i = -22; i <= 22; i++) {
        WorldGrid->AddSquare(i, -22);
    }
    for (int i = -21; i <= 21; i++) {
        WorldGrid->AddSquare(22, i);
    }
    for (int i = -21; i <= 21; i++) {
        WorldGrid->AddSquare(-22, i);
    }

    ParticleManager = new Particles("src/shaders/particles.comp", 5000);
    ParticleManager->RenderSolidColourState(glm::vec3(1.0));

    mainVBO = new VertexBuffer(quadData, sizeof(quadData), GL_STATIC_DRAW);
    mainVBO->addAttribute(0, 2, GL_FLOAT, 4, 0);
    mainVBO->addAttribute(1, 2, GL_FLOAT, 4, 2);

    mainShader = new Shader("src/shaders/main.vert", "src/shaders/main.frag");

    mainCamera = new Camera2D(glm::vec3(0.0, 0.0, 0.0), glm::vec2(1.0), 0.0);

    timeText = new TextRenderer("res/img/sans-serif.png", 15, 15, 72, 90, true);

    PlayerAnimation->Update();
}   

void PlayingScene::Update() {
    if (TimeLeft <= 0 || CurrentState == Screen::Win) {
        CurrentState = Screen::Win;
        TimeSinceFinished += DeltaTime;
    } else if (MainPlayer->Health <= 0 || CurrentState == Screen::Lose) {
        CurrentState = Screen::Lose;
        TimeSinceFinished += DeltaTime;
    } else {
        CurrentState = Screen::Playing;
    }

    if (CurrentState == Screen::Lose) {
        if (TimeSinceFinished <= float(DeltaTime)) {
            ParticleManager->Emit(MainPlayer->Position, 13, 1.5, 15, 10, 10, false);
            mainCamera->TriggerShake(15.0, 2.0);
        }
        mainCamera->Position = mainCamera->CameraToEntity(*MainPlayer, WIDTH, HEIGHT, lerpToTime(0.5, DeltaTime));
        mainCamera->UpdateShake();
        ParticleManager->Update(glm::vec2(0.0));
    }

    if (CurrentState == Screen::Playing) {
        Time += DeltaTime;
        if (floor(Time) != floor(LastTime)) {
            TimeLeft--;
            Enemies[TimeLeft % 29]->Respawn();
        }
    
        MainPlayer->KeyboardUpdate(input);
    
        MainPlayer->VeloUpdate(*WorldGrid, 200, *Generators[0], *Generators[1], *Generators[2], *Generators[3], Enemies);
    
        if (!MainPlayer->isDashing) {
            glm::vec2 PCMousePos = glm::vec2(MousePos.x - WIDTH / 2, (MousePos.y - HEIGHT / 2) * -1);
            //std::cout << PCMousePos.x << ", " << PCMousePos.y << std::endl;
            glm::vec2 PCPlayerScreenPos = glm::vec2((MainPlayer->Position - mainCamera->Position).x - WIDTH / 2, (MainPlayer->Position - mainCamera->Position).y - HEIGHT / 2);
            //std::cout << PCPlayerScreenPos.x << ", " << PCPlayerScreenPos.y << std::endl;
            MainPlayer->SetDirection(glm::normalize( glm::vec2(PCMousePos - PCPlayerScreenPos) ));
        }

        mainCamera->Position = mainCamera->CameraToEntity(*MainPlayer, WIDTH, HEIGHT, lerpToTime(0.5, DeltaTime));
        mainCamera->UpdateShake();
        ParticleManager->Update(glm::vec2(0.0));
    
        for (int i = 0; i < Enemies.size(); i++) {
            Enemies[i]->Update(*MainPlayer, *WorldGrid, 200, *Generators[0], *Generators[1], *Generators[2], *Generators[3], TimeLeft, *mainCamera, *ParticleManager);
        }
    
        LastTime = Time;
    }
    
}

void PlayingScene::Render() {
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    mainShader->use();

    glm::mat4 View = mainCamera->GetViewMatrix(WIDTH, HEIGHT);
    mainShader->setMat4("view", View);

    mainShader->setMat4("projection", Projection);

    mainShader->setFloat("Alpha", 1.0);
    
    //Player
    
    glm::mat4 PlayerModel = MainPlayer->GetTransformMatrix();
    mainShader->setMat4("model", PlayerModel);
    
        if (CurrentState != Screen::Lose) {
            
        mainShader->setBool("isSolidColour", false);
        mainShader->setBool("isAnimation", false);

        mainShader->setInt("Texture", 0);
        PlayerAnimation->RenderSprite(*mainShader, *mainVBO, PlayerModel, View, Projection);

        glDrawArrays(GL_TRIANGLES, 0, 6);
    }

    mainVBO->bind();
    
    //Enemy
    mainShader->use();
    mainShader->setBool("isSolidColour", true);
    mainShader->setBool("isAnimation", false);
    for (int i = 0; i < Enemies.size(); i++) {
        if (!Enemies[i]->isActive) continue;
        mainShader->use();
    
        glm::mat4 EnemyModel = Enemies[i]->GetTransformMatrix();
        mainShader->setMat4("model", EnemyModel);

        mainShader->setVec3("Colour", glm::vec3(1.0, 0.2, 0.3));
    
        glDrawArrays(GL_TRIANGLES, 0, 6);
    }
    //Generators

    mainShader->use();
    
    glm::mat4 Gen1Model = Generators[0]->GetTransformMatrix();
    mainShader->setMat4("model", Gen1Model);

    mainShader->setVec3("Colour", glm::vec3(1.0));
    
    glDrawArrays(GL_TRIANGLES, 0, 6);

    mainShader->use();
    
    glm::mat4 Gen2Model = Generators[1]->GetTransformMatrix();
    mainShader->setMat4("model", Gen2Model);

    mainShader->setVec3("Colour", glm::vec3(1.0));
    
    glDrawArrays(GL_TRIANGLES, 0, 6);

    mainShader->use();
    
    glm::mat4 Gen3Model = Generators[2]->GetTransformMatrix();
    mainShader->setMat4("model", Gen3Model);

    mainShader->setVec3("Colour", glm::vec3(1.0));
    
    glDrawArrays(GL_TRIANGLES, 0, 6);

    mainShader->use();
    
    glm::mat4 Gen4Model = Generators[3]->GetTransformMatrix();
    mainShader->setMat4("model", Gen4Model);

    mainShader->setVec3("Colour", glm::vec3(1.0));
    
    glDrawArrays(GL_TRIANGLES, 0, 6);

    //Particles

    ParticleManager->Render(View, Projection);

    //Gridspace
    
    WorldGrid->RenderAll(*mainShader, *mainVBO, View, Projection);

    //Text

    glm::mat4 TextView(1.0);

    timeText->RenderText(std::to_string(TimeLeft), glm::vec2(0.0), 1.0, TextView, Projection);

    //Overlay for screens
    if (CurrentState == Screen::Lose)
        timeText->RenderText("Game Over :(", glm::vec2(100, HEIGHT + 100 - (EaseInOut(glm::clamp(TimeSinceFinished - 2, 0.0f, 1.0f)) * 400)), 1.0, TextView, Projection);
    if (CurrentState == Screen::Win) {
        mainShader->use();

        glm::mat4 Overlay(1.0f);
        Overlay = glm::translate(Overlay, glm::vec3(-WIDTH / 2, -HEIGHT / 2, 0.0));
        Overlay = glm::scale(Overlay, glm::vec3(WIDTH, HEIGHT, 0.0));
        mainShader->setMat4("model", Overlay);

        mainShader->setVec3("Colour", glm::vec3(0.0));
        mainShader->setFloat("Alpha", EaseInOut(glm::clamp(TimeSinceFinished, 0.0f, 1.0f)) / 3);

        glDrawArrays(GL_TRIANGLES, 0, 6);

        timeText->RenderText("You win :)", glm::vec2(100, HEIGHT + 100 - (EaseInOut(glm::clamp(TimeSinceFinished - 2, 0.0f, 1.0f)) * 400)), 1.0, TextView, Projection);
    }
}  

void PlayingScene::Exit() {
    delete MainPlayer;
    delete WorldGrid;
    delete mainVBO;
    delete mainShader;
    delete mainCamera;
    for (Enemy *enemy : Enemies) {
        delete enemy;
        enemy = nullptr;
    }
    delete timeText;
    delete PlayerAnimation;
    MainPlayer = nullptr;
    WorldGrid = nullptr;
    mainVBO = nullptr;
    mainShader = nullptr;
    mainCamera = nullptr;
    timeText = nullptr;
    PlayerAnimation = nullptr;
}

//MainMenuScene

void MainMenuScene::Init() {
    mainVBO = new VertexBuffer(quadData, sizeof(quadData), GL_STATIC_DRAW);
    mainVBO->addAttribute(0, 2, GL_FLOAT, 4, 0);
    mainVBO->addAttribute(1, 2, GL_FLOAT, 4, 2);

    mainShader = new Shader("src/shaders/main.vert", "src/shaders/main.frag");

    text = new TextRenderer("res/img/sans-serif.png", 15, 15, 72, 90, true);
}

void MainMenuScene::Update() {
    if (input.isKeyPressed(GLFW_KEY_SPACE))
        game.ChangeScene(new PlayingScene());
}

void MainMenuScene::Render() {
    glClearColor(0.9f, 0.9f, 0.9f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    glm::mat4 View(1.0f);
    text->RenderText("COUNTDOWN", glm::vec2(WIDTH/2 - 100 - 225, HEIGHT/2 + 50), 1.0f, View, Projection);
    text->RenderText("Press Space to Start", glm::vec2(WIDTH/2 - 120 - 225, HEIGHT/2 - 20), 0.5f, View, Projection); 
}

void MainMenuScene::Exit() {
    delete mainVBO;
    delete mainShader;
    delete text;
    
    mainVBO = nullptr;
    mainShader = nullptr;
    text = nullptr;
   
}