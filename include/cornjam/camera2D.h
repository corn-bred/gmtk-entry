#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/exponential.hpp>
#include <cornjam/entity.h>

class Entity;

class Camera2D {
    private:
    glm::vec2 Direction;
    float DirectionRad;
    public:
    glm::vec3 Position;
    glm::vec2 Scale;

    //Construction
    Camera2D(glm::vec3 position, glm::vec2 scale, glm::vec2 directionVec2);
    Camera2D(glm::vec3 position, glm::vec2 scale, float directionRad);

    //Direction stuff
    void SetDirection(float directionRad);

    void SetDirection(glm::vec2 direction);

    inline float GetDirectionRad();

    inline glm::vec2 GetDirection();

    //Utilities I guess

    float ShakeTimer = 0.0;
    float ShakeIntensity = 0.0;
    float ShakeDuration = 0.0;

    void TriggerShake(float intensity, float duration);

    void UpdateShake();

    glm::vec2 GetShakeOffset();

    glm::vec3 CameraToEntity(Entity &entity, int ScreenWidth, int ScreenHeight, float lerp);

    glm::vec2 DirTowardsEntity_Vec2(Entity &entity, int ScreenWidth, int ScreenHeight);

    float DirTowardsEntity_Radian(Entity &entity, int ScreenWidth, int ScreenHeight);

    //View Matrix Creation

    glm::mat4 GetViewMatrix(int ScreenWidth, int ScreenHeight, bool Shake);
};