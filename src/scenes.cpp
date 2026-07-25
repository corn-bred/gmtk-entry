#include "scenes.h"
#include "game.h"          
#include <cornjam/inputmanager.h>  
#include <misc/globals.h>

//PlayingScene

void RenderBarX(Shader &mainShader, VertexBuffer &mainVBO, float progress, glm::vec2 scale, glm::vec2 position, glm::vec3 barColor) {
    glm::mat4 UIView(1.0);
    // Background of health bar when empty
    mainShader.use();
    mainShader.setMat4("view", UIView); 
    mainShader.setMat4("projection", Projection);
    mainShader.setBool("isSolidColour", true);
    mainShader.setBool("isAnimation", false);
    mainShader.setVec3("Colour", glm::vec3(0.15f));
    
    glm::mat4 bgModel(1.0);
    bgModel = glm::translate(bgModel, glm::vec3(position.x + scale.x / 2, position.y, 0.0f));
    bgModel = glm::scale(bgModel, glm::vec3(scale, 1.0f));
    mainShader.setMat4("model", bgModel);
    mainVBO.bind();
    glDrawArrays(GL_TRIANGLES, 0, 6);
    
    glm::mat4 fgModel(1.0);
    fgModel = glm::translate(fgModel, glm::vec3(position.x + (scale.x * progress) / 2, position.y, 0.0f));
    fgModel = glm::scale(fgModel, glm::vec3(scale.x * progress, scale.y, 1.0f));

    mainShader.setMat4("model", fgModel);
    mainShader.setVec3("Colour", barColor);
    glDrawArrays(GL_TRIANGLES, 0, 6);
}

void RenderBarY(Shader &mainShader, VertexBuffer &mainVBO, float progress, glm::vec2 scale, glm::vec2 position, glm::vec3 barColor) {
    glm::mat4 UIView(1.0);
    // Background of health bar when empty
    mainShader.use();
    mainShader.setMat4("view", UIView); 
    mainShader.setMat4("projection", Projection);
    mainShader.setBool("isSolidColour", true);
    mainShader.setBool("isAnimation", false);
    mainShader.setVec3("Colour", glm::vec3(0.15f));
    
    glm::mat4 bgModel(1.0);
    bgModel = glm::translate(bgModel, glm::vec3(position.x, position.y + scale.y / 2, 0.0f));
    bgModel = glm::scale(bgModel, glm::vec3(scale, 1.0f));
    mainShader.setMat4("model", bgModel);
    mainVBO.bind();
    glDrawArrays(GL_TRIANGLES, 0, 6);
    
    glm::mat4 fgModel(1.0);
    fgModel = glm::translate(fgModel, glm::vec3(position.x, position.y + (scale.y * progress) / 2, 0.0f));
    fgModel = glm::scale(fgModel, glm::vec3(scale.x, scale.y * progress, 1.0f));

    mainShader.setMat4("model", fgModel);
    mainShader.setVec3("Colour", barColor);
    glDrawArrays(GL_TRIANGLES, 0, 6);
}

float bgVertices[] = {
    -2500.0f, -2500.0f,   0.0f,   0.0f,
     2500.0f, -2500.0f,   50.0f,  0.0f,
     2500.0f,  2500.0f,   50.0f,  50.0f,

    -2500.0f, -2500.0f,   0.0f,   0.0f,
     2500.0f,  2500.0f,   50.0f,  50.0f,
    -2500.0f,  2500.0f,   0.0f,   50.0f
};

void debugDraw(Shader &mainShader, VertexBuffer &mainVBO, AABBHitbox &hitbox) {
    mainShader.use();
    mainShader.setVec4("ColourMultiplier", glm::vec4(1.0));
    mainShader.setBool("isSolidColour", true);
    mainShader.setBool("isAnimation", false);
    mainShader.setVec3("Colour", glm::vec3(1.0));
    glm::mat4 yes(1.0f);
    yes = glm::translate(yes, glm::vec3(hitbox.Origin, 0.0));
    yes = glm::scale(yes, glm::vec3(hitbox.Scale, 0.0));
    mainShader.setMat4("model", yes);
    mainVBO.bind();
    glDrawArrays(GL_TRIANGLES, 0, 6);
}

void PlayingScene::Init() {
    Time = 0.0f;   
    LastTime = 0.0f;
    TimeLeft = 60;
    CurrentState = Screen::Playing;

    MainPlayer = new Player(glm::vec3(0.0, -500.0, 0.0), 0.0, glm::vec2(200.0), glm::vec2(0.0), glm::vec2(10000.0), glm::vec2(0.8), glm::vec2(100), glm::vec2(1.0), glm::vec2(-25), glm::vec2(50), glm::vec2(0, 0), glm::vec2(30), 4000.0, 1.5, 0.5, 0.7, 1.0);
    
    PlayerAnimation = new Anim_SpriteRenderer("res/img/player.png", 1, 11, false);

    EnemyWalking = new Animation(0, 10, 1.0/24, true, "res/img/enemy.png", 5, 5, false);
    EnemyDashing = new Animation(11, 21, 1.0/24, true, "res/img/enemy.png", 5, 5, false);

    for (int i = 0; i < 30; i++) {
        Enemies.push_back(new Enemy(glm::vec3(rand() % 100 - 50, rand() % 100 - 50, 0.0), 0.0, 125.0, glm::vec2(0.0), glm::vec2(10000.0), glm::vec2(0.8), glm::vec2(100), glm::vec2(1.0), glm::vec2(-25), glm::vec2(50), 1.5, 2250.0, 1.5f, 3.5f));
    }

    Background = new TextureBuffer("res/img/0055.png", GL_REPEAT, GL_REPEAT);

    Generators[0] = new Generator(glm::vec2(1000, 0.0));
    Generators[1] = new Generator(glm::vec2(-1000, 0.0));
    Generators[2] = new Generator(glm::vec2(0.0, 1000));
    Generators[3] = new Generator(glm::vec2(0.0, -1000));

    GeneratorAnimation = new Animation(0, 20, 1.0/12, true, "res/img/generator.png", 3, 7, false);

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

    bgVBO = new VertexBuffer(bgVertices, sizeof(bgVertices), GL_STATIC_DRAW);
    bgVBO->addAttribute(0, 2, GL_FLOAT, 4, 0); // Position
    bgVBO->addAttribute(1, 2, GL_FLOAT, 4, 2); // UV

    GameLost = new AudioData("res/audio/gamelost.wav");
    GameWon = new AudioData("res/audio/gamewon.wav");
}   

void PlayingScene::Update() {
    if (TimeLeft <= 0 || CurrentState == Screen::Win) {
        if (TimeSinceFinished + DeltaTime >= 2.0 && TimeSinceFinished < 2.0)
            audio.PlaySound(*GameWon);
        CurrentState = Screen::Win;
        TimeSinceFinished += DeltaTime;
    } else if (MainPlayer->Health <= 0 || CurrentState == Screen::Lose) {
        if (TimeSinceFinished + DeltaTime >= 2.0 && TimeSinceFinished < 2.0)
            audio.PlaySound(*GameLost);
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
        ParticleManager->Update(glm::vec2(0.0));
        mainCamera->UpdateShake();
    }

    TimeStop = glm::max(TimeStop - DeltaTime, 0.0);

    if (CurrentState == Screen::Playing) {
        if (TimeStop > 0)
            DeltaTime = 0;

        Time += DeltaTime;
        if (floor(Time) != floor(LastTime)) {
            TimeLeft--;
            if (TimeLeft >= 60 && TimeLeft % 5 == 0) {
                std::cout << "60>\n";
                Enemies[TimeLeft % 29]->Respawn(glm::vec2(rand() % 100 - 50, rand() % 100 - 50));
            } else if (TimeLeft < 60 && TimeLeft % 4 == 0) {
                std::cout << "<60\n";
                Enemies[TimeLeft % 29]->Respawn(glm::vec2(rand() % 100 - 50, rand() % 100 - 50));
            } else if (TimeLeft < 50 && TimeLeft >= 40 && TimeLeft % 3 == 0) {
                std::cout << "<50\n";
                Enemies[TimeLeft % 29]->Respawn(glm::vec2(rand() % 100 - 50, rand() % 100 - 50));
            } else if (TimeLeft < 40 && TimeLeft >= 30  && TimeLeft % 2 == 0) {
                std::cout << "<40\n";
                Enemies[TimeLeft % 29]->Respawn(glm::vec2(rand() % 100 - 50, rand() % 100 - 50));
            } else if (TimeLeft < 30 && TimeLeft >= 20) {
                std::cout << "<30\n";
                Enemies[TimeLeft % 29]->Respawn(glm::vec2(rand() % 100 - 50, rand() % 100 - 50));
            } else if (TimeLeft < 20 && TimeLeft >= 10) {
                std::cout << "<20\n";
                Enemies[TimeLeft % 29]->Respawn(glm::vec2(rand() % 300 - 150, rand() % 300 - 150));
            } else if (TimeLeft < 10) {
                std::cout << "<10\n";
                Enemies[TimeLeft % 29]->Respawn(glm::vec2(rand() % 700 - 350, rand() % 700 - 350));
            }
        }
    
        MainPlayer->KeyboardUpdate(input);
    
        MainPlayer->VeloUpdate(*WorldGrid, 200, *Generators[0], *Generators[1], *Generators[2], *Generators[3], Enemies, *ParticleManager);
    
        if (!MainPlayer->isDashing) {
            glm::vec2 PCMousePos = glm::vec2(MousePos.x - WIDTH / 2, (MousePos.y - HEIGHT / 2) * -1);
            //std::cout << PCMousePos.x << ", " << PCMousePos.y << std::endl;
            glm::vec2 PCPlayerScreenPos = glm::vec2((MainPlayer->Position - mainCamera->Position).x - WIDTH / 2, (MainPlayer->Position - mainCamera->Position).y - HEIGHT / 2);
            //std::cout << PCPlayerScreenPos.x << ", " << PCPlayerScreenPos.y << std::endl;
            MainPlayer->SetDirection(glm::normalize( glm::vec2(PCMousePos - PCPlayerScreenPos) ));
        }

        mainCamera->Position = mainCamera->CameraToEntity(*MainPlayer, WIDTH, HEIGHT, lerpToTime(0.5, DeltaTime));
        ParticleManager->Update(glm::vec2(0.0));

        GeneratorAnimation->Update();
    
        for (int i = 0; i < Enemies.size(); i++) {
            Enemies[i]->Update(*MainPlayer, *WorldGrid, 200, *Generators[0], *Generators[1], *Generators[2], *Generators[3], TimeLeft, *mainCamera, *ParticleManager, TimeStop);
        }

        EnemyWalking->Update();
        EnemyDashing->Update();
        
        mainCamera->UpdateShake();

        LastTime = Time;
    }
    if (CurrentState == Screen::Win) {
        MainPlayer->Velocity = glm::vec2(0.0);
        mainCamera->UpdateShake();
    }
    
    
}


void PlayingScene::Render() {
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    mainShader->use();

    glm::mat4 View = mainCamera->GetViewMatrix(WIDTH, HEIGHT, (TimeStop > 0.0) ? false : true);
    mainShader->setMat4("view", View);

    mainShader->setMat4("projection", Projection);

    mainShader->setFloat("Alpha", 1.0);

    mainShader->setVec4("ColourMultiplier", glm::vec4(1.0));

    glm::mat4 matBackground(1.0f);
    
    mainShader->setMat4("model", matBackground);
    mainShader->setBool("isSolidColour", false);
    mainShader->setBool("isAnimation", false);
    mainShader->setInt("Texture", 0);

    Background->bindTexture(0);

    bgVBO->bind();

    glDrawArrays(GL_TRIANGLES, 0, 6);
    
    //Player
    
    if (CurrentState != Screen::Lose) {
        if (MainPlayer->iTime > 0) {
            if (MainPlayer->iTime >= MainPlayer->InvincTime - 0.2)
                mainShader->setVec4("ColourMultiplier", glm::vec4(1.5, 0.7, 0.7, 1.0));
            else 
                mainShader->setVec4("ColourMultiplier", glm::vec4(1.3, 1.3, 1.3, sin(CurrentTime) / 8 + 0.75));
        }
            
        glm::mat4 PlayerModel = MainPlayer->GetTransformMatrix();
        if (MainPlayer->isDashing)
            PlayerAnimation->RenderSprite(*mainShader, *mainVBO, PlayerModel, View, Projection, int(CurrentTime * glm::distance(glm::vec2(0.0), MainPlayer->Velocity) * 0.005) % 12);
        else {
            PlayerAnimation->RenderSprite(*mainShader, *mainVBO, PlayerModel, View, Projection, 0);
        }
    }

    mainShader->setVec4("ColourMultiplier", glm::vec4(1.0));
    
    //Enemy
    mainShader->use();
    mainShader->setBool("isSolidColour", true);
    mainShader->setBool("isAnimation", false);
    for (int i = 0; i < Enemies.size(); i++) {
        if (!Enemies[i]->isActive) continue;
    
        glm::mat4 EnemyModel = Enemies[i]->GetTransformMatrix();
        mainShader->setMat4("model", EnemyModel);

        if (Enemies[i]->SpawnProtTime > 0.0)
            mainShader->setVec4("ColourMultiplier", glm::vec4(glm::vec3(1.7), glm::clamp(-Enemies[i]->SpawnProtTime + 1.0, 0.0, 0.7)));
        else
            mainShader->setVec4("ColourMultiplier", glm::vec4(1.0));


        if (Enemies[i]->CurrentState == EnemyState::Dashing)
            EnemyDashing->RenderSprite(*mainShader, *mainVBO, EnemyModel, View, Projection);
        else
            EnemyWalking->RenderSprite(*mainShader, *mainVBO, EnemyModel, View, Projection);
    }

    mainShader->setVec4("ColourMultiplier", glm::vec4(1.0));
    //Generators

    mainShader->use();
    mainShader->setBool("isSolidColour", true);
    mainShader->setBool("isAnimation", false);
    
    glm::mat4 Gen1Model = Generators[0]->GetTransformMatrix();
    GeneratorAnimation->RenderSprite(*mainShader, *mainVBO, Gen1Model, View, Projection);
    
    glm::mat4 Gen2Model = Generators[1]->GetTransformMatrix();
    GeneratorAnimation->RenderSprite(*mainShader, *mainVBO, Gen2Model, View, Projection);
    
    glm::mat4 Gen3Model = Generators[2]->GetTransformMatrix();
    GeneratorAnimation->RenderSprite(*mainShader, *mainVBO, Gen3Model, View, Projection);
    
    glm::mat4 Gen4Model = Generators[3]->GetTransformMatrix();
    GeneratorAnimation->RenderSprite(*mainShader, *mainVBO, Gen4Model, View, Projection);

    //Particles

    ParticleManager->Render(View, Projection);

    //Gridspace
    
    WorldGrid->RenderAll(*mainShader, *mainVBO, View, Projection);

    //Text

    glm::mat4 UIView(1.0);

    timeText->RenderText(std::to_string(TimeLeft) + "s", glm::vec2(40, 40), 1.0, UIView, Projection);

    RenderBarY(*mainShader, *mainVBO, MainPlayer->DashGauge, glm::vec2(30, 300), glm::vec2(WIDTH - 30 - 40, 40), glm::vec3(0.8 + 0.2 * MainPlayer->DashGauge, 0.6 * MainPlayer->DashGauge, 0.0));

    timeText->RenderText("HP: " + std::to_string(int(MainPlayer->Health)), glm::vec2(WIDTH / 2.0 - 250, 65), 0.5, UIView, Projection);

    RenderBarX(*mainShader, *mainVBO, MainPlayer->Health / 100.0, glm::vec2(500, 30), glm::vec2(WIDTH / 2.0 - 250, 40), (MainPlayer->Health / 100.0 >= 0.7) ? glm::vec3(0.0, 1.0, 0.0) : (MainPlayer->Health / 100.0 >= 0.3) ? glm::vec3(1.0, 1.0, 0.0): glm::vec3(1.0, 0.0, 0.0));

    //Overlay for screens
    if (CurrentState == Screen::Lose) {
        mainShader->use();

        glm::mat4 Overlay(1.0f);
        Overlay = glm::translate(Overlay, glm::vec3(WIDTH/2, HEIGHT/2, 0.0));
        Overlay = glm::scale(Overlay, glm::vec3(WIDTH, HEIGHT, 0.0));

        mainShader->setMat4("model", Overlay);

        mainShader->setMat4("view", UIView);

        mainShader->setVec3("Colour", glm::vec3(0.0));
        mainShader->setFloat("Alpha", EaseInOut(glm::clamp(TimeSinceFinished, 0.0f, 1.0f)) / 3);

        glDrawArrays(GL_TRIANGLES, 0, 6);

        timeText->RenderText("Game Over :(", glm::vec2(100, HEIGHT + 100 - (EaseInOut(glm::clamp(TimeSinceFinished - 2, 0.0f, 1.0f)) * 400)), 1.0, UIView, Projection);
    }
    if (CurrentState == Screen::Win) {
        mainShader->use();

        glm::mat4 Overlay(1.0f);
        Overlay = glm::translate(Overlay, glm::vec3(WIDTH/2, HEIGHT/2, 0.0));
        Overlay = glm::scale(Overlay, glm::vec3(WIDTH, HEIGHT, 0.0));

        mainShader->setMat4("model", Overlay);

        mainShader->setMat4("view", UIView);

        mainShader->setVec3("Colour", glm::vec3(0.0));
        mainShader->setFloat("Alpha", EaseInOut(glm::clamp(TimeSinceFinished, 0.0f, 1.0f)) / 3);

        glDrawArrays(GL_TRIANGLES, 0, 6);

        timeText->RenderText("You win :)", glm::vec2(100, HEIGHT + 100 - (EaseOut(glm::clamp(TimeSinceFinished - 2, 0.0f, 1.0f)) * 400)), 1.0, UIView, Projection);
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
    delete EnemyWalking;
    delete EnemyDashing;
    delete Background;
    delete bgVBO;
    MainPlayer = nullptr;
    WorldGrid = nullptr;
    mainVBO = nullptr;
    mainShader = nullptr;
    mainCamera = nullptr;
    timeText = nullptr;
    PlayerAnimation = nullptr;
    EnemyWalking = nullptr;
    EnemyDashing = nullptr;
    Background = nullptr;
    bgVBO = nullptr;
}

//MainMenuScene

void MainMenuScene::Init() {
    mainVBO = new VertexBuffer(quadData, sizeof(quadData), GL_STATIC_DRAW);
    mainVBO->addAttribute(0, 2, GL_FLOAT, 4, 0);
    mainVBO->addAttribute(1, 2, GL_FLOAT, 4, 2);

    mainShader = new Shader("src/shaders/main.vert", "src/shaders/main.frag");

    text = new TextRenderer("res/img/sans-serif.png", 15, 15, 72, 90, true);
    
    GameStart = new AudioData("res/audio/gamestart.wav");
}

void MainMenuScene::Update() {
    if (input.isKeyPressed(GLFW_KEY_ENTER)) {
        audio.PlaySound(*GameStart);
        game.ChangeScene(new PlayingScene());
    }
}

void MainMenuScene::Render() {
    glClearColor(0.9f, 0.9f, 0.9f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    glm::mat4 View(1.0f);
    text->RenderText("COUNTDOWN", glm::vec2(WIDTH/2 - 100 - 225, HEIGHT/2 + 50), 1.0f, View, Projection);
    text->RenderText("Press ENTER to Start", glm::vec2(WIDTH/2 - 120 - 225, HEIGHT/2 - 20), 0.5f, View, Projection); 
}

void MainMenuScene::Exit() {
    delete mainVBO;
    delete mainShader;
    delete text;
    
    mainVBO = nullptr;
    mainShader = nullptr;
    text = nullptr;
   
}