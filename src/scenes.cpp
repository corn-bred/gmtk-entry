#include "scenes.h"
#include "game.h"          
#include <cornjam/inputmanager.h>  
#include <misc/globals.h>

//PlayingScene

void PlayingScene::Init() {
    MainPlayer = new Player(glm::vec3(0.0), 0.0, glm::vec2(200.0), glm::vec2(0.0), glm::vec2(10000.0), glm::vec2(0.8), glm::vec2(50), glm::vec2(1.0), glm::vec2(0.0), glm::vec2(50), 1000.0, 3.0, 0.75);
    testEnemy = new Enemy(glm::vec3(0.0), 0.0, glm::vec2(200.0), glm::vec2(0.0), glm::vec2(10000.0), glm::vec2(0.8), glm::vec2(50), glm::vec2(1.0), glm::vec2(0.0), glm::vec2(50));
    WorldGrid = new GridSpace(glm::vec2(50), glm::vec3(0.0));

    mainVBO = new VertexBuffer(quadData, sizeof(quadData), GL_STATIC_DRAW);
    mainVBO->addAttribute(0, 2, GL_FLOAT, 4, 0);
    mainVBO->addAttribute(1, 2, GL_FLOAT, 4, 2);

    mainShader = new Shader("src/shaders/main.vert", "src/shaders/main.frag");

    mainCamera = new Camera2D(glm::vec3(0.0, 0.0, 0.0), glm::vec2(1.0), 0.0);
}   

void PlayingScene::Update() {
    MainPlayer->KeyboardUpdate(input);
    MainPlayer->VeloUpdate(*WorldGrid, 200);

    glm::vec2 PCMousePos = glm::vec2(MousePos.x - WIDTH / 2, (MousePos.y - HEIGHT / 2) * -1);
    //std::cout << PCMousePos.x << ", " << PCMousePos.y << std::endl;
    glm::vec2 PCPlayerScreenPos = glm::vec2((MainPlayer->Position - mainCamera->Position).x - WIDTH / 2, (MainPlayer->Position - mainCamera->Position).y - HEIGHT / 2);
    //std::cout << PCPlayerScreenPos.x << ", " << PCPlayerScreenPos.y << std::endl;
    MainPlayer->SetDirection(glm::normalize( glm::vec2(PCMousePos - PCPlayerScreenPos) ));

    mainCamera->Position = mainCamera->CameraToEntity(*MainPlayer, WIDTH, HEIGHT, lerpToTime(0.5, DeltaTime));

    testEnemy->Update(*MainPlayer, *WorldGrid, 200);
}

void PlayingScene::Render() {
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    mainShader->use();

    glm::mat4 View = mainCamera->GetViewMatrix(WIDTH, HEIGHT);
    mainShader->setMat4("view", View);

    mainShader->setMat4("projection", Projection);
    
    //Player

    glm::mat4 PlayerModel = MainPlayer->GetTransformMatrix();
    mainShader->setMat4("model", PlayerModel);
    
    
    mainShader->setBool("isSolidColour", true);
    mainShader->setBool("isAnimation", false);

    mainShader->setVec3("Colour", glm::vec3(1.0));

    mainVBO->bind();
    
    glDrawArrays(GL_TRIANGLES, 0, 6);
    
    //Enemy

    mainShader->use();
    
    glm::mat4 EnemyModel = testEnemy->GetTransformMatrix();
    mainShader->setMat4("model", EnemyModel);

    mainShader->setVec3("Colour", glm::vec3(1.0, 0.2, 0.3));
    
    glDrawArrays(GL_TRIANGLES, 0, 6);
}

void PlayingScene::Exit() {
    delete MainPlayer;
    delete WorldGrid;
    delete mainVBO;
    delete mainShader;
    delete mainCamera;
    delete testEnemy;
    MainPlayer = nullptr;
    WorldGrid = nullptr;
    mainVBO = nullptr;
    mainShader = nullptr;
    mainCamera = nullptr;
    testEnemy = nullptr;
}