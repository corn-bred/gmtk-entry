#include <cornjam/camera2D.h>

//Construction
Camera2D::Camera2D(glm::vec3 position, glm::vec2 scale, glm::vec2 directionVec2) : Position(position), Scale(scale) {
    SetDirection(directionVec2);
}
Camera2D::Camera2D(glm::vec3 position, glm::vec2 scale, float directionRad) : Position(position), Scale(scale), DirectionRad(directionRad) {
    SetDirection(directionRad);
}

//Direction stuff
void Camera2D::SetDirection(float directionRad) {
    DirectionRad = directionRad;
    Direction = glm::vec2(std::cos(directionRad), std::sin(directionRad));
}

void Camera2D::SetDirection(glm::vec2 direction) {
    Direction = glm::normalize(direction);
    DirectionRad = glm::atan(direction.y, direction.x);
}

inline float Camera2D::GetDirectionRad() {
    return DirectionRad;
}

inline glm::vec2 Camera2D::GetDirection() {
    return Direction;
}

//Utilities I guess

void Camera2D::TriggerShake(float intensity, float duration) {
    ShakeIntensity = intensity;
    ShakeDuration = duration;
    ShakeTimer = duration;
}

void Camera2D::UpdateShake() {
    if (ShakeTimer > 0.0f) {
        ShakeTimer -= DeltaTime;
        if (ShakeTimer < 0.0) ShakeTimer = 0.0f;
    }
}

glm::vec2 Camera2D::GetShakeOffset() {
    if (ShakeTimer <= 0.0f) return glm::vec2(0.0);
    
    float CurrentIntensity = ShakeIntensity * glm::pow(ShakeTimer / ShakeDuration, 4.0);
    return glm::vec2((rand() % 100 - 50) / 50.0f * CurrentIntensity, (rand() % 100 - 50) / 50.0f * CurrentIntensity); 
}

glm::vec3 Camera2D::CameraToEntity(Entity &entity, int ScreenWidth, int ScreenHeight, float lerp = 1.0) {
    glm::vec3 Pos = glm::vec3(0.0);
    Pos.x = entity.Position.x - ScreenWidth / 2;
    Pos.y = entity.Position.y - ScreenHeight / 2;

    //Holy lerping
    Pos.x = Position.x * (1.0 - lerp) + Pos.x * lerp;
    Pos.y = Position.y * (1.0 - lerp) + Pos.y * lerp;
    return Pos;
}

glm::vec2 Camera2D::DirTowardsEntity_Vec2(Entity &entity, int ScreenWidth, int ScreenHeight) {
    glm::vec2 DirectionVec2 = glm::normalize(glm::vec2(entity.Position.x, entity.Position.y) - glm::vec2(Position.x + ScreenWidth / 2, Position.y + ScreenHeight / 2));
    return DirectionVec2;
}

float Camera2D::DirTowardsEntity_Radian(Entity &entity, int ScreenWidth, int ScreenHeight) {
    glm::vec2 DirectionVec2 = glm::normalize(glm::vec2(entity.Position.x, entity.Position.y) - glm::vec2(Position.x + ScreenWidth / 2, Position.y + ScreenHeight / 2));
    return (glm::atan(DirectionVec2.x, DirectionVec2.y));
}

//View Matrix Creation

glm::mat4 Camera2D::GetViewMatrix(int ScreenWidth, int ScreenHeight) {
    glm::mat4 Matrix(1.0f);

    Matrix = glm::translate(Matrix, glm::vec3(ScreenWidth / 2, ScreenHeight / 2, 0.0));
    Matrix = glm::rotate(Matrix, -DirectionRad, glm::vec3(0.0, 0.0, 1.0));
    Matrix = glm::scale(Matrix, glm::vec3(1.0 / Scale.x, 1.0 / Scale.y, 1.0));
    Matrix = glm::translate(Matrix, glm::vec3(-ScreenWidth / 2, -ScreenHeight / 2, 0.0));

    Matrix = glm::translate(Matrix, glm::vec3(-Position.x, -Position.y, 0.0));

    glm::vec2 shake = GetShakeOffset();
    Matrix = glm::translate(Matrix, glm::vec3(shake.x, shake.y, 0.0f));
    
    return Matrix;
}
