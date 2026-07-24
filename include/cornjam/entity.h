#pragma once
#include <iostream>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/exponential.hpp>
#include <cornjam/collision.h>
#include <cornjam/grid.h>
#include <cornjam/inputmanager.h>
#include <cornbreadlib/audiodata.h>
#include <cornbreadlib/audiomanager.h>
#include <misc/globals.h>
#include <algorithm>
#include <cmath>
#include <cornjam/particles.h>
#include <vector>
#include <cornjam/camera2D.h>

class Enemy;
class Camera2D;

enum class EnemyState {
    Wander, //Will try to get to generator
    Chase, //Flee away from the player
    ReadyDash, //Preparing for dash
    Dashing //Dashing...
};

class Entity {
    protected:
    glm::vec2 Direction;
    float DirectionRad;
    public:
    glm::vec3 Position;
    glm::vec2 Velocity;
    
    glm::vec2 ScaleLocal;
    glm::vec2 ScaleGlobal;

    void SetDirection(float directionRad);

    void SetDirection(glm::vec2 direction);

    inline float GetDirectionRad() const;

    inline glm::vec2 GetDirectionVec2() const;
};

class Generator : public Entity {
    public:
    AABBHitbox CollisionBox, InteractionBox;

    Generator(glm::vec2 position);

    glm::mat4 GetTransformMatrix();
};

class Player : public Entity {
    public:
    int CollisionAxes[2] = {0, 0}; //{x, y}
    glm::vec2 Resistance;
    glm::vec2 Speed, TerminalSpeed;
    float DashSpeed;
    float DashGauge = 1.0; //How long you can dash
    float DashRefillSpeed, DashUseSpeed;
    float DashCooldown;
    AABBHitbox Hitbox, Hurtbox;
    glm::vec2 HurtboxPosition;

    bool isDashing = false;
    float LastDash = 0.0;

    float Health = 100.0;
    float iTime = 0.0;
    float InvincTime;
    

    Player(glm::vec3 position, float directionRad, glm::vec2 speed, glm::vec2 velocity, glm::vec2 terminalSpeed, glm::vec2 resistance, glm::vec2 scaleLocal, glm::vec2 scaleGlobal, glm::vec2 hitboxPosition, glm::vec2 hitboxSize, glm::vec2 hurtboxPosition, glm::vec2 hurtboxSize, float dashSpeed, float dashRefillSpeed, float dashUseSpeed, float dashCoolDown, float invincibilityTime);

    void VeloUpdate(GridSpace &grid, int searchRadius, Generator &gen1, Generator &gen2, Generator &gen3, Generator &gen4, std::vector<Enemy*> &enemies);

    glm::mat4 GetTransformMatrix();

    void KeyboardUpdate (InputManager &input);
};

class Enemy : public Entity {
    public:
    int CollisionAxes[2] = {0, 0}; //{x, y}
    glm::vec2 Resistance;
    glm::vec2 TerminalSpeed;
    float Speed;
    AABBHitbox Hitbox;
    AudioData KillSound;
    EnemyState CurrentState = EnemyState::Wander;
    float LastDist, Dist;
    bool turningLeft; //bruh bad code but i dont care

    bool isActive = false;

    bool isDashing = false;
    float DashSpeed;
    float DashPrepareTime = 0.0;
    float DashPrepTime;
    float DashCooldown;
    float DashGauge = 1.0;
    float CooldownTime = 0.0;

    bool ReadyDashStartup = false;
    bool DashingStartup = false;
    bool Dashing = false;

    float TimeInContact = 0.0;
    float ContactTime;

    Enemy(glm::vec3 position, float directionRad, float speed, glm::vec2 velocity, glm::vec2 terminalSpeed, glm::vec2 resistance, glm::vec2 scaleLocal, glm::vec2 scaleGlobal, glm::vec2 hitboxPosition, glm::vec2 hitboxSize, float contactTime, float dashspeed, float dashPrepTime, float cooldownTime);

    bool Colliding(Player &player);

    void Respawn();

    void Update(Player &player, GridSpace &grid, int searchRadius, Generator &gen1, Generator &gen2, Generator &gen3, Generator &gen4, int &Time, Camera2D &camera, Particles &particleManager);

    glm::mat4 GetTransformMatrix();
};