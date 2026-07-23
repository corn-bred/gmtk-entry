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

class Entity {
    protected:
    glm::vec2 Direction;
    float DirectionRad;
    public:
    glm::vec3 Position;
    glm::vec2 Velocity;
    
    glm::vec2 ScaleLocal;
    glm::vec2 ScaleGlobal;

    void SetDirection(float directionRad) {
        DirectionRad = directionRad;
        Direction = glm::vec2(std::cos(directionRad), std::sin(directionRad));
    }

    void SetDirection(glm::vec2 direction) {
        Direction = direction;
        DirectionRad = glm::atan(direction.y, direction.x);
    }

    inline float GetDirectionRad() const {
        return DirectionRad;
    }

    inline glm::vec2 GetDirectionVec2() const {
        return Direction;
    }
};

class Player : public Entity {
    public:
    int CollisionAxes[2] = {0, 0}; //{x, y}
    glm::vec2 Resistance;
    glm::vec2 Speed, TerminalSpeed;
    float DashSpeed;
    float DashGauge = 1.0; //How long you can dash
    float DashRefillSpeed, DashUseSpeed;
    AABBHitbox Hitbox;

    bool isDashing = false;

    Player(glm::vec3 position = glm::vec3(0.0), float directionRad = 0.0, glm::vec2 speed = glm::vec2(0.0), glm::vec2 velocity = glm::vec2(0.0), glm::vec2 terminalSpeed = glm::vec2(1.0), glm::vec2 resistance = glm::vec2(1.0), glm::vec2 scaleLocal = glm::vec2(1.0), glm::vec2 scaleGlobal = glm::vec2(1.0), glm::vec2 hitboxPosition = glm::vec2(0.0), glm::vec2 hitboxSize = glm::vec2(1.0), float dashSpeed = 1.0, float dashRefillSpeed = 3.0, float dashUseSpeed = 1.5) : Hitbox(hitboxPosition + glm::vec2(position.x, position.y), hitboxSize) {
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
    }

    void VeloUpdate(GridSpace &grid, int searchRadius) { //If anything is farther than searchRadius, discard it from collision testing.
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
        Hitbox.Origin.y = Position.y; //Update hitbox
        //std::cout << CollisionAxes[0] << ", " << CollisionAxes[1] << std::endl;

        //Update velocities for next loop

        Velocity.x = Velocity.x * glm::pow(Resistance.x, DeltaTime * 60);
        
        
        //Velocity.y = Velocity.y * glm::pow(Resistance.y, DeltaTime * 60);

        Velocity.y = Velocity.y * glm::pow(Resistance.y, DeltaTime * 60);
        
        //std::cout << Position.x << ", " << Position.y << " : ";
        //std::cout << Hitbox.Origin.x << ", " << Hitbox.Origin.y << std::endl;
    }

    glm::mat4 GetTransformMatrix() {
        glm::mat4 Matrix(1.0f);

        Matrix = glm::translate(Matrix, glm::vec3(Position.x, Position.y, 0.0));
        Matrix = glm::scale(Matrix, glm::vec3(ScaleGlobal.x, ScaleGlobal.y, 1.0));
        Matrix = glm::rotate(Matrix, DirectionRad, glm::vec3(0.0, 0.0,1.0));
        Matrix = glm::scale(Matrix, glm::vec3(ScaleLocal.x, ScaleLocal.y, 1.0));
        
        return Matrix;
    }

    void KeyboardUpdate (InputManager &input) {
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

        if (input.isActionPressed(Action::Dash) && DashGauge != 0.0f) {
            Velocity += GetDirectionVec2() * DashSpeed * float(glm::pow(DashGauge, 2.0)) * float(DeltaTime * 60);
            
            DashGauge = std::clamp(float(DashGauge - (1.0 / DashUseSpeed) * DeltaTime), 0.0f, 1.0f);
            isDashing = true;
        } else {
            DashGauge = std::clamp(float(DashGauge + (1.0 / DashRefillSpeed) * DeltaTime), 0.0f, 1.0f);
            isDashing = false;
        }
        std::cout << "DashGauge: " << DashGauge << std::endl;
    }
};

class Enemy : public Entity {
    public:
    int CollisionAxes[2] = {0, 0}; //{x, y}
    glm::vec2 Resistance;
    glm::vec2 Speed, TerminalSpeed;
    float DashSpeed;
    float DashGauge = 1.0; //How long you can dash
    AABBHitbox Hitbox;
    AudioData KillSound;

    Enemy(glm::vec3 position = glm::vec3(0.0), float directionRad = 0.0, glm::vec2 speed = glm::vec2(0.0), glm::vec2 velocity = glm::vec2(0.0), glm::vec2 terminalSpeed = glm::vec2(1.0), glm::vec2 resistance = glm::vec2(1.0), glm::vec2 scaleLocal = glm::vec2(1.0), glm::vec2 scaleGlobal = glm::vec2(1.0), glm::vec2 hitboxPosition = glm::vec2(0.0), glm::vec2 hitboxSize = glm::vec2(1.0)) : Hitbox(hitboxPosition + glm::vec2(position.x, position.y), hitboxSize), KillSound("res/audio/laser_slam.wav") {
        Position = position;
        Resistance = resistance;
        Speed = speed;
        TerminalSpeed = terminalSpeed;
        DirectionRad = directionRad;
        Direction = glm::vec2(std::cos(DirectionRad), std::sin(DirectionRad));
        ScaleLocal = scaleLocal;
        ScaleGlobal = scaleGlobal;
    }

    bool Colliding(Player &player) {
        if (player.isDashing && Collision(player.Hitbox, Hitbox)) {
            KillSound.SetSourcePosition(Position);
            return true;
        } else
            return false;
    }

   void Update(Player &player, GridSpace &grid, int searchRadius) { //If anything is farther than searchRadius, discard it from collision testing.
        if (Colliding(player)) {
            audio.PlaySound(KillSound);
            Position = glm::vec3(WIDTH, HEIGHT, 0.0); // TEMPORARY
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
        Hitbox.Origin.y = Position.y; //Update hitbox
        //std::cout << CollisionAxes[0] << ", " << CollisionAxes[1] << std::endl;

        //Update velocities for next loop

        Velocity.x = Velocity.x * glm::pow(Resistance.x, DeltaTime * 60);
        
        
        //Velocity.y = Velocity.y * glm::pow(Resistance.y, DeltaTime * 60);

        Velocity.y = Velocity.y * glm::pow(Resistance.y, DeltaTime * 60);
        
        //std::cout << Position.x << ", " << Position.y << " : ";
        //std::cout << Hitbox.Origin.x << ", " << Hitbox.Origin.y << std::endl;
    }

    glm::mat4 GetTransformMatrix() {
        glm::mat4 Matrix(1.0f);

        Matrix = glm::translate(Matrix, glm::vec3(Position.x, Position.y, 0.0));
        Matrix = glm::scale(Matrix, glm::vec3(ScaleGlobal.x, ScaleGlobal.y, 1.0));
        Matrix = glm::rotate(Matrix, DirectionRad, glm::vec3(0.0, 0.0,1.0));
        Matrix = glm::scale(Matrix, glm::vec3(ScaleLocal.x, ScaleLocal.y, 1.0));
        
        return Matrix;
    }
};