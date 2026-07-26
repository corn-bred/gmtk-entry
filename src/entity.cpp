#include <cornjam/entity.h>
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
#include <cornjam/particles.h>

void Entity::SetDirection(float directionRad) {
    DirectionRad = directionRad;
    Direction = glm::vec2(std::cos(directionRad), std::sin(directionRad));
}

void Entity::SetDirection(glm::vec2 direction) {
    Direction = direction;
    DirectionRad = glm::atan(direction.y, direction.x);
}

inline float Entity::GetDirectionRad() const {
    return DirectionRad;
}

inline glm::vec2 Entity::GetDirectionVec2() const {
    return Direction;
}

Generator::Generator(glm::vec2 position) : CollisionBox(glm::vec2(position.x - 25, position.y - 25), glm::vec2(100, 100)), InteractionBox(glm::vec2(position.x - 35, position.y - 35), glm::vec2(120, 120)) {
    Position = glm::vec3(position, 0.0);

    SetDirection(0.0f);
    
    ScaleLocal = glm::vec2(150, 150);
    ScaleGlobal = glm::vec2(1.0);
}

glm::mat4 Generator::GetTransformMatrix() {
    glm::mat4 Matrix(1.0f);

    Matrix = glm::translate(Matrix, glm::vec3(Position.x, Position.y, 0.0));
    Matrix = glm::scale(Matrix, glm::vec3(ScaleGlobal.x, ScaleGlobal.y, 1.0));
    Matrix = glm::rotate(Matrix, DirectionRad, glm::vec3(0.0, 0.0,1.0));
    Matrix = glm::scale(Matrix, glm::vec3(ScaleLocal.x, ScaleLocal.y, 1.0));
    
    return Matrix;
}

Player::Player(glm::vec3 position = glm::vec3(0.0), float directionRad = 0.0, glm::vec2 speed = glm::vec2(0.0), glm::vec2 velocity = glm::vec2(0.0), glm::vec2 terminalSpeed = glm::vec2(1.0), glm::vec2 resistance = glm::vec2(1.0), glm::vec2 scaleLocal = glm::vec2(1.0), glm::vec2 scaleGlobal = glm::vec2(1.0), glm::vec2 hitboxPosition = glm::vec2(0.0), glm::vec2 hitboxSize = glm::vec2(1.0), glm::vec2 hurtboxPosition = glm::vec2(0.0), glm::vec2 hurtboxSize = glm::vec2(1.0), float dashSpeed = 1.0, float dashRefillSpeed = 3.0, float dashUseSpeed = 1.5, float dashCoolDown = 0.3, float invincibilityTime = 0.5) : Hitbox(hitboxPosition + glm::vec2(position.x, position.y), hitboxSize), Hurtbox(hurtboxPosition + glm::vec2(position.x, position.y), hurtboxSize), PlayerDash("res/audio/playerdash.wav"), PlayerDeath("res/audio/playerdeath.wav"), PlayerHurt("res/audio/playerhurt.wav") {
    Position = position;
    Resistance = resistance;
    Speed = speed;
    TerminalSpeed = terminalSpeed;
    DirectionRad = directionRad;
    Direction = glm::vec2(std::cos(DirectionRad), std::sin(DirectionRad));
    ScaleLocal = scaleLocal;
    ScaleGlobal = scaleGlobal;
    DashSpeed = dashSpeed; 
    DashRefillSpeed = dashRefillSpeed;
    DashUseSpeed = dashUseSpeed;
    DashCooldown = dashCoolDown;
    InvincTime = invincibilityTime;
    HurtboxPosition = hurtboxPosition;
}

void Player::VeloUpdate(GridSpace &grid, int searchRadius, Generator &gen1, Generator &gen2, Generator &gen3, Generator &gen4, std::vector<Enemy*> &enemies, Particles &particleManager, int &score) { //If anything is farther than searchRadius, discard it from collision testing.
    //First, clamp the velocities
    Velocity.x = glm::clamp(Velocity.x, -TerminalSpeed.x, TerminalSpeed.x);
    Velocity.y = glm::clamp(Velocity.y, -TerminalSpeed.y, TerminalSpeed.y);
    
    CollisionAxes[0] = 0;
    Position.x += Velocity.x * (float)(DeltaTime);
    Hitbox.Origin.x = Position.x;

    for(int i = 0; i < (int)grid.Data.size() / 2; i++) { //Cycling through each piece of data, inefficient but works for now

        glm::vec2 DataPosition = grid.getPosition(i); //Get data
        
        glm::vec2 Diff = glm::abs(DataPosition - glm::vec2(Position.x, Position.y));

        if (Diff.x <= searchRadius && Diff.y <= searchRadius) { //if distance between the grid and the entity < searchRadius, then test. If not, then discard

            AABBHitbox gridHitbox(DataPosition, grid.GridRes);

            if(Collision(Hitbox, gridHitbox)) {

                float Snapped = snapToSurfaceX(Hitbox, gridHitbox, Velocity.x);
                Position.x += Snapped;
                
                Velocity.x = 0;

                if (Snapped > 0) //Swapped, because Snapped is from colliding to snapped, not the other way around
                    CollisionAxes[0] = -1;
                else if (Snapped < 0)
                    CollisionAxes[0] = 1;
                    
            }
        }
        
    }

    if(Collision(Hitbox, gen1.CollisionBox)) {
        float Snapped = snapToSurfaceX(Hitbox, gen1.CollisionBox, Velocity.x);
        Position.x += Snapped;
        
        Velocity.x = 0;

        if (Snapped > 0) //Swapped, because Snapped is from colliding to snapped, not the other way around
            CollisionAxes[0] = -1;
        else if (Snapped < 0)
            CollisionAxes[0] = 1;
            
    }

    if(Collision(Hitbox, gen2.CollisionBox)) {
        float Snapped = snapToSurfaceX(Hitbox, gen2.CollisionBox, Velocity.x);
        Position.x += Snapped;
        
        Velocity.x = 0;
        
        if (Snapped > 0) //Swapped, because Snapped is from colliding to snapped, not the other way around
            CollisionAxes[0] = -1;
        else if (Snapped < 0)
            CollisionAxes[0] = 1;
    }

    if(Collision(Hitbox, gen3.CollisionBox)) {
        float Snapped = snapToSurfaceX(Hitbox, gen3.CollisionBox, Velocity.x);
        Position.x += Snapped;
        
        Velocity.x = 0;
        
        if (Snapped > 0) //Swapped, because Snapped is from colliding to snapped, not the other way around
            CollisionAxes[0] = -1;
        else if (Snapped < 0)
            CollisionAxes[0] = 1;
            
    }
    if(Collision(Hitbox, gen4.CollisionBox)) {
        float Snapped = snapToSurfaceX(Hitbox, gen4.CollisionBox, Velocity.x);
        Position.x += Snapped;
        
        Velocity.x = 0;
        
        if (Snapped > 0) //Swapped, because Snapped is from colliding to snapped, not the other way around
            CollisionAxes[0] = -1;
        else if (Snapped < 0)
            CollisionAxes[0] = 1;
            
    }

    Position.x = glm::clamp(Position.x, -1050.0f, 1050.0f);

    Hitbox.Origin.x = Position.x; //Update hitbox
    
    CollisionAxes[1] = 0;
    
    Position.y += Velocity.y * (float)(DeltaTime);
    
    Hitbox.Origin.y = Position.y;
    
    for(int i = 0; i < (int)grid.Data.size() / 2; i++) { //Cycling through each piece of data, inefficient but works for now
        
        glm::vec2 DataPosition = grid.getPosition(i); //Get data
        glm::vec2 Diff = glm::abs(DataPosition - glm::vec2(Position.x, Position.y));
        
        if (Diff.x <= searchRadius && Diff.y <= searchRadius) { //if distance between the grid and the entity < searchRadius, then test. If not, then discard
            
            AABBHitbox gridHitbox(DataPosition, grid.GridRes);
            if(Collision(Hitbox, gridHitbox)) {
                float Snapped = snapToSurfaceY(Hitbox, gridHitbox, Velocity.y);
                Position.y += Snapped;
                Velocity.y = 0;
                if (Snapped > 0)
                    CollisionAxes[1] = -1;
                else if (Snapped < 0)
                    CollisionAxes[1] = 1;
            }
        }
        
    }
    
    if(Collision(Hitbox, gen1.CollisionBox)) {
        float Snapped = snapToSurfaceY(Hitbox, gen1.CollisionBox, Velocity.y);
        Position.y += Snapped;
        
        Velocity.y = 0;
        
        if (Snapped > 0) //Swapped, because Snapped is from colliding to snapped, not the other way around
            CollisionAxes[0] = -1;
        else if (Snapped < 0)
            CollisionAxes[0] = 1;
            
    }

    if(Collision(Hitbox, gen2.CollisionBox)) {
        float Snapped = snapToSurfaceY(Hitbox, gen2.CollisionBox, Velocity.y);
        Position.y += Snapped;
        
        Velocity.y = 0;
        
        if (Snapped > 0) //Swapped, because Snapped is from colliding to snapped, not the other way around
            CollisionAxes[0] = -1;
        else if (Snapped < 0)
            CollisionAxes[0] = 1;
            
    }

    if(Collision(Hitbox, gen3.CollisionBox)) {
        float Snapped = snapToSurfaceY(Hitbox, gen3.CollisionBox, Velocity.y);
        Position.y += Snapped;
        
        Velocity.y = 0;
        
        if (Snapped > 0) //Swapped, because Snapped is from colliding to snapped, not the other way around
            CollisionAxes[0] = -1;
        else if (Snapped < 0)
            CollisionAxes[0] = 1;
            
    }

    if(Collision(Hitbox, gen4.CollisionBox)) {
        float Snapped = snapToSurfaceY(Hitbox, gen4.CollisionBox, Velocity.y);
        Position.y += Snapped;
        
        Velocity.y = 0;
        
        if (Snapped > 0) //Swapped, because Snapped is from colliding to snapped, not the other way around
            CollisionAxes[0] = -1;
        else if (Snapped < 0)
            CollisionAxes[0] = 1;
            
    }

    //Lag safegaurd
    std::cout << Position.x << ", " << Position.y << std::endl;
    Position.y = glm::clamp(Position.y, -1050.0f, 1050.0f);

    Hitbox.Origin.y = Position.y; //Update hitbox
    //std::cout << CollisionAxes[0] << ", " << CollisionAxes[1] << std::endl;

    Hurtbox.Origin = HurtboxPosition + glm::vec2(Position.x, Position.y);

    for (int i = 0; i < enemies.size(); i++) {
        if (Collision(Hurtbox, enemies[i]->Hitbox) && iTime <= 0.0 && ((!isDashing && DashingTime > 0.2) || enemies[i]->isDashing) && enemies[i]->isActive && enemies[i]->SpawnProtTime <= 0.0) {
            score -= 10.0;
            Health -= 10.0;
            iTime = InvincTime;
            particleManager.Emit(Position, 5, 0.5, 10, 10, 10, false);
            if (Health > 0.0)
                audio.PlaySound(PlayerHurt);
            else
                audio.PlaySound(PlayerDeath);
            Velocity = glm::normalize(Hurtbox.Origin - enemies[i]->Hitbox.Origin) * 4000.0f;
        }
    }

    //Update velocities for next loop
    Velocity.x = Velocity.x * glm::pow(Resistance.x, DeltaTime * 60);
    
    
    //Velocity.y = Velocity.y * glm::pow(Resistance.y, DeltaTime * 60);
    Velocity.y = Velocity.y * glm::pow(Resistance.y, DeltaTime * 60);
    
    //std::cout << Position.x << ", " << Position.y << " : ";
    //std::cout << Hitbox.Origin.x << ", " << Hitbox.Origin.y << std::endl;
    iTime = glm::clamp(iTime - DeltaTime, 0.0, 1000.0);
}

glm::mat4 Player::GetTransformMatrix() {
    glm::mat4 Matrix(1.0f);
    Matrix = glm::translate(Matrix, glm::vec3(Position.x, Position.y, 0.0));
    Matrix = glm::scale(Matrix, glm::vec3(ScaleGlobal.x, ScaleGlobal.y, 1.0));
    Matrix = glm::rotate(Matrix, DirectionRad, glm::vec3(0.0, 0.0,1.0));
    Matrix = glm::scale(Matrix, glm::vec3(ScaleLocal.x, ScaleLocal.y, 1.0));
    
    return Matrix;
}

void Player::KeyboardUpdate (InputManager &input) {
    if (input.isActionPressed(Action::MoveLeft)) {
        Velocity.x += -Speed.x * (DeltaTime * 60);
    }

    if (input.isActionPressed(Action::MoveRight)) {
        Velocity.x += Speed.x * (DeltaTime * 60);
    }

    if (input.isActionPressed(Action::MoveUp)) {
        Velocity.y += Speed.y * (DeltaTime * 60);
    }

    if (input.isActionPressed(Action::MoveDown)) {
        Velocity.y += -Speed.y * (DeltaTime * 60);
    }

    if (input.isActionPressed(Action::Dash) && LastDash > DashCooldown) {
        if (!isDashing) {
            audio.PlaySound(PlayerDash);
            DashingTime = 0;
        }
        Velocity = GetDirectionVec2() * DashSpeed * float(glm::pow(DashGauge, 1.0));
        
        DashGauge = std::clamp(float(DashGauge - (1.0 / DashUseSpeed) * DeltaTime), 0.0f, 1.0f);
        isDashing = true;
        LastDash = 1001.0f; //unreachable value with clamp
    } else {
        DashingTime = glm::clamp(DashingTime + DeltaTime, 0.0, 100.0);
        
        if (LastDash == 1001.0f) {
            LastDash = 0;
        }
        DashGauge = std::clamp(float(DashGauge + (1.0 / DashRefillSpeed) * DeltaTime), 0.0f, 1.0f);
        isDashing = false;
        LastDash = std::clamp(LastDash + float(DeltaTime), -1000.0f, 1000.0f);//arbitrary large value
    }
    //std::cout << "DashGauge: " << DashGauge << ", HP: " << Health << std::endl;
}

Enemy::Enemy(glm::vec3 position = glm::vec3(0.0), float directionRad = 0.0, float speed = 0.0, glm::vec2 velocity = glm::vec2(0.0), glm::vec2 terminalSpeed = glm::vec2(1.0), glm::vec2 resistance = glm::vec2(1.0), glm::vec2 scaleLocal = glm::vec2(1.0), glm::vec2 scaleGlobal = glm::vec2(1.0), glm::vec2 hitboxPosition = glm::vec2(0.0), glm::vec2 hitboxSize = glm::vec2(1.0), float contactTime = 1.0, float dashspeed = 1.0f, float dashPrepTime = 1.0, float cooldownTime = 3.0) : Hitbox(hitboxPosition + glm::vec2(position.x, position.y), hitboxSize), KillSound("res/audio/enemydeath.wav"), AddTime("res/audio/addtime.wav") {
    Position = position;
    Resistance = resistance;
    Speed = speed;
    TerminalSpeed = terminalSpeed;
    DirectionRad = directionRad;
    Direction = glm::vec2(std::cos(DirectionRad), std::sin(DirectionRad));
    ScaleLocal = scaleLocal;
    ScaleGlobal = scaleGlobal;
    ContactTime = contactTime;
    DashPrepTime = dashPrepTime;
    CooldownTime = cooldownTime;
    DashSpeed = dashspeed;
}

bool Enemy::Colliding(Player &player) {
    if (((player.isDashing && player.DashGauge > 0.0) || player.DashingTime <= 0.2) && Collision(player.Hitbox, Hitbox)) {
        return true;
    } else
        return false;
}

void Enemy::Respawn(glm::vec2 position) {
    Position = glm::vec3(position, 0.0);
    SpawnProtTime = 1.0;
    isActive = true;
}

void Enemy::Update(Player &player, GridSpace &grid, int searchRadius, Generator &gen1, Generator &gen2, Generator &gen3, Generator &gen4, int &Time, Camera2D &camera, Particles &particleManager, float &TimeStop, int &score) { //If anything is farther than searchRadius, discard it from collision testing.
    if (isActive == false) return;

    if (CurrentState == EnemyState::ReadyDash) Velocity = glm::vec2(0.0);
    else Velocity = GetDirectionVec2() * Speed;

    float gen1Dist = glm::distance(Position, gen1.Position);
    float gen2Dist = glm::distance(Position, gen2.Position);
    float gen3Dist = glm::distance(Position, gen3.Position);
    float gen4Dist = glm::distance(Position, gen4.Position);
    Dist = std::min(gen1Dist, std::min(gen2Dist, std::min(gen3Dist, gen4Dist)));

    glm::vec2 GenDir = glm::vec2(1.0, 0.0);

    int genTarget = 1;

    if (Dist == gen1Dist) {
        genTarget = 1;
        GenDir = glm::normalize(gen1.Position - Position);
    }
    if (Dist == gen2Dist) {
        genTarget = 2;
        GenDir = glm::normalize(gen2.Position - Position);
    }
    if (Dist == gen3Dist) {
        genTarget = 3;
        GenDir = glm::normalize(gen3.Position - Position);
    }
    if (Dist == gen4Dist) {
        genTarget = 4;
        GenDir = glm::normalize(gen4.Position - Position);
    }

    if ((glm::distance(Position, player.Position) > 450 || Dist < 200) && !Dashing)
        CurrentState = EnemyState::Wander;
    else if ((glm::distance(Position, player.Position) > 200) && !Dashing)
        CurrentState = EnemyState::Chase;
    else if (!isDashing && DashCooldown <= 0) {
        if (CurrentState != EnemyState::ReadyDash)
            ReadyDashStartup = true;
        CurrentState = EnemyState::ReadyDash;
    } else if (isDashing && DashCooldown <= 0)
        CurrentState = EnemyState::Dashing;
    else 
        CurrentState = EnemyState::Chase;

    switch (CurrentState) {
        case EnemyState::Wander:
            switch (genTarget) {
                case 1:
                    if(Collision(Hitbox, gen1.InteractionBox)) {
                        TimeInContact += DeltaTime;
                        if (TimeInContact >= ContactTime) {
                            score -= 10.0;
                            Time += 3;
                            isActive = false;
                            audio.PlaySound(AddTime);
                        }
                    } else TimeInContact = 0;
                    break;
                case 2:
                    if(Collision(Hitbox, gen2.InteractionBox)) {
                        TimeInContact += DeltaTime;
                        if (TimeInContact >= ContactTime) { 
                            score -= 10.0;
                            Time += 3;
                            isActive = false;
                            audio.PlaySound(AddTime);
                        }
                    } else TimeInContact = 0;
                    break;
                case 3:
                    if(Collision(Hitbox, gen3.InteractionBox)) {
                        TimeInContact += DeltaTime;
                        if (TimeInContact >= ContactTime) {
                            score -= 10.0;
                            Time += 3;
                            isActive = false;
                            audio.PlaySound(AddTime);
                        }
                    } else TimeInContact = 0;
                    break;
                case 4:
                    if(Collision(Hitbox, gen4.InteractionBox)) {
                        TimeInContact += DeltaTime;
                        if (TimeInContact >= ContactTime) {
                            score -= 10.0;
                            Time += 3;
                            isActive = false;
                            audio.PlaySound(AddTime);
                        }
                    } else TimeInContact = 0;
                    break;
                default:
                    break;
            }
            SetDirection(GenDir);
            break;
        case EnemyState::Chase: {
            glm::vec2 playerDir = glm::normalize(player.Position - Position);
            SetDirection(glm::normalize(GetDirectionVec2() + playerDir));
            break;
        }
        case EnemyState::ReadyDash:
            Dashing = true;

            if (ReadyDashStartup) {
                DashPrepareTime = DashPrepTime;
                ReadyDashStartup = false;
            }

            Velocity = glm::vec2(0.0);
                
            DashPrepareTime = glm::clamp(float(DashPrepareTime - DeltaTime), -1.0f, DashPrepTime);
            if (DashPrepareTime <= 0.0) {
                isDashing = true;
                DashingStartup = true;
            }
            break;
        case EnemyState::Dashing:
            Dashing = true;

            DashPrepareTime = 0.0;

            if (DashingStartup) {
                DashGauge = 1.0f;
                DashingStartup = false;
            }

            Velocity = GetDirectionVec2() * DashSpeed * float(glm::pow(DashGauge, 2.0));

            DashGauge = glm::clamp(float(DashGauge - (DeltaTime * 2)), 0.0f, 1.0f);
            
            if (DashGauge <= 0.0) {
                Velocity = glm::vec2(0.0);
                Dashing = false;
                isDashing = false;
                DashCooldown = CooldownTime;
            }
    }

    if (DashCooldown - DeltaTime <= 0.0) {
        DashCooldown = 0.0f;
    } else {
        DashCooldown -= DeltaTime;
    }

    SpawnProtTime = glm::clamp(SpawnProtTime - DeltaTime, -10.0, 10000.0);

    //is bro touching a player
    if (Colliding(player) && SpawnProtTime <= 0.0) {
        score += 10.0 + SpawnProtTime + 2.0;
        camera.TriggerShake(10.0, 1.0);
        audio.PlaySound(KillSound);
        particleManager.Emit(Position, 13, 1.5, 15, 10, 10, false);
        TimeStop = 0.05;
        isActive = false;
    }

    //First, clamp the velocities
    Velocity.x = glm::clamp(Velocity.x, -TerminalSpeed.x, TerminalSpeed.x);
    Velocity.y = glm::clamp(Velocity.y, -TerminalSpeed.y, TerminalSpeed.y);
    
    CollisionAxes[0] = 0;

    Position.x += Velocity.x * (float)(DeltaTime);
    Hitbox.Origin.x = Position.x;

    for(int i = 0; i < (int)grid.Data.size() / 2; i++) { //Cycling through each piece of data, inefficient but works for now

        glm::vec2 DataPosition = grid.getPosition(i); //Get data
        
        glm::vec2 Diff = glm::abs(DataPosition - glm::vec2(Position.x, Position.y));

        if (Diff.x <= searchRadius && Diff.y <= searchRadius) { //if distance between the grid and the entity < searchRadius, then test. If not, then discard
            AABBHitbox gridHitbox(DataPosition, grid.GridRes);
            if(Collision(Hitbox, gridHitbox)) {
                float Snapped = snapToSurfaceX(Hitbox, gridHitbox, Velocity.x);
                Position.x += Snapped;
                Velocity.x = 0;

                if (Snapped > 0) //Swapped, because Snapped is from colliding to snapped, not the other way around
                    CollisionAxes[0] = -1;
                else if (Snapped < 0)
                    CollisionAxes[0] = 1;
                    
            }
        }
        
    }

    if(Collision(Hitbox, gen1.CollisionBox)) {
        float Snapped = snapToSurfaceX(Hitbox, gen1.CollisionBox, Velocity.x);
        Position.x += Snapped;
        Velocity.x = 0;

        if (Snapped > 0) //Swapped, because Snapped is from colliding to snapped, not the other way around
            CollisionAxes[0] = -1;
        else if (Snapped < 0)
            CollisionAxes[0] = 1;
            
    }

    if(Collision(Hitbox, gen2.CollisionBox)) {
        float Snapped = snapToSurfaceX(Hitbox, gen2.CollisionBox, Velocity.x);
        Position.x += Snapped;
        Velocity.x = 0;

        if (Snapped > 0) //Swapped, because Snapped is from colliding to snapped, not the other way around
            CollisionAxes[0] = -1;
        else if (Snapped < 0)
            CollisionAxes[0] = 1;
            
    }

    if(Collision(Hitbox, gen3.CollisionBox)) {
        float Snapped = snapToSurfaceX(Hitbox, gen3.CollisionBox, Velocity.x);
        Position.x += Snapped;
        Velocity.x = 0;

        if (Snapped > 0) //Swapped, because Snapped is from colliding to snapped, not the other way around
            CollisionAxes[0] = -1;
        else if (Snapped < 0)
            CollisionAxes[0] = 1;
            
    }

    if(Collision(Hitbox, gen4.CollisionBox)) {
        float Snapped = snapToSurfaceX(Hitbox, gen4.CollisionBox, Velocity.x);
        Position.x += Snapped;
        Velocity.x = 0;

        if (Snapped > 0) //Swapped, because Snapped is from colliding to snapped, not the other way around
            CollisionAxes[0] = -1;
        else if (Snapped < 0)
            CollisionAxes[0] = 1;
            
    }

    Hitbox.Origin.x = Position.x; //Update hitbox
    

    CollisionAxes[1] = 0;

    Position.y += Velocity.y * (float)(DeltaTime);
    Hitbox.Origin.y = Position.y;

    for(int i = 0; i < (int)grid.Data.size() / 2; i++) { //Cycling through each piece of data, inefficient but works for now

        glm::vec2 DataPosition = grid.getPosition(i); //Get data

        glm::vec2 Diff = glm::abs(DataPosition - glm::vec2(Position.x, Position.y));

        if (Diff.x <= searchRadius && Diff.y <= searchRadius) { //if distance between the grid and the entity < searchRadius, then test. If not, then discard
            AABBHitbox gridHitbox(DataPosition, grid.GridRes);
            if(Collision(Hitbox, gridHitbox)) {
                float Snapped = snapToSurfaceY(Hitbox, gridHitbox, Velocity.y);
                Position.y += Snapped;
                Velocity.y = 0;

                if (Snapped > 0)
                    CollisionAxes[1] = -1;
                else if (Snapped < 0)
                    CollisionAxes[1] = 1;

            }
        }
        
    }

    if(Collision(Hitbox, gen1.CollisionBox)) {
        float Snapped = snapToSurfaceY(Hitbox, gen1.CollisionBox, Velocity.y);
        Position.y += Snapped;
        Velocity.y = 0;

        if (Snapped > 0) //Swapped, because Snapped is from colliding to snapped, not the other way around
            CollisionAxes[0] = -1;
        else if (Snapped < 0)
            CollisionAxes[0] = 1;
    }

    if(Collision(Hitbox, gen2.CollisionBox)) {
        float Snapped = snapToSurfaceY(Hitbox, gen2.CollisionBox, Velocity.y);
        Position.y += Snapped;
        Velocity.y = 0;

        if (Snapped > 0) //Swapped, because Snapped is from colliding to snapped, not the other way around
            CollisionAxes[0] = -1;
        else if (Snapped < 0)
            CollisionAxes[0] = 1;
    }

    if(Collision(Hitbox, gen3.CollisionBox)) {
        float Snapped = snapToSurfaceY(Hitbox, gen3.CollisionBox, Velocity.y);
        Position.y += Snapped;
        Velocity.y = 0;

        if (Snapped > 0) //Swapped, because Snapped is from colliding to snapped, not the other way around
            CollisionAxes[0] = -1;
        else if (Snapped < 0)
            CollisionAxes[0] = 1;
            
    }

    if(Collision(Hitbox, gen4.CollisionBox)) {
        float Snapped = snapToSurfaceY(Hitbox, gen4.CollisionBox, Velocity.y);
        Position.y += Snapped;
        Velocity.y = 0;

        if (Snapped > 0) //Swapped, because Snapped is from colliding to snapped, not the other way around
            CollisionAxes[0] = -1;
        else if (Snapped < 0)
            CollisionAxes[0] = 1;
            
    }

    Hitbox.Origin.y = Position.y; //Update hitbox
    //std::cout << CollisionAxes[0] << ", " << CollisionAxes[1] << std::endl;

    //Update velocities for next loop

    Velocity.x = Velocity.x * glm::pow(Resistance.x, DeltaTime * 60);
    
    
    //Velocity.y = Velocity.y * glm::pow(Resistance.y, DeltaTime * 60);

    Velocity.y = Velocity.y * glm::pow(Resistance.y, DeltaTime * 60);
    
    //std::cout << Position.x << ", " << Position.y << " : ";
    //std::cout << Hitbox.Origin.x << ", " << Hitbox.Origin.y << std::endl;
        
    LastDist = Dist;
}

glm::mat4 Enemy::GetTransformMatrix() {
    glm::mat4 Matrix(1.0f);

    Matrix = glm::translate(Matrix, glm::vec3(Position.x, Position.y, 0.0));
    Matrix = glm::scale(Matrix, glm::vec3(ScaleGlobal.x, ScaleGlobal.y, 1.0));
    Matrix = glm::rotate(Matrix, DirectionRad, glm::vec3(0.0, 0.0,1.0));
    Matrix = glm::scale(Matrix, glm::vec3(ScaleLocal.x, ScaleLocal.y, 1.0));
    
    return Matrix;
}