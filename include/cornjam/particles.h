#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glad/glad.h>
#include <cornbreadlib/SSBO.h>
#include <cornbreadlib/computeshader.h>
#include <cornbreadlib/shaders.h>
#include <cornbreadlib/texturebuffer.h>
#include <cornbreadlib/vertexbuffer.h>
#include <time.h>

#include <cornjam/spritesheet.h>
#include <misc/globals.h>

#define PI 3.14159265358979323846

inline float randomFloat() {
    return rand() / (RAND_MAX + 1.0);
}


struct Particle {
    glm::vec2 Position; //8 bytes = 8 =/8
    glm::vec2 Origin;   //8 bytes = 16 =/8
    glm::vec2 Velocity; //8 bytes = 24 =/8
    float Duration;     //4 bytes = 28 =/4
    float MaxDuration;  //4 bytes = 32 =/4
    float Size;         //4 bytes = 36 =/4
    uint32_t Flags;     //4 bytes = 40 =/4
    float Speed;        //4 bytes = 44 !/8
    float padding;       //4 bytes = 44 =/8
};

#define FLAG_IS_ACTIVE (1u << 0) // 1
#define FLAG_IS_LOOPING   (1u << 1) // 2
#define FLAG_HAS_GRAVITY (1u << 2) // 4

#define RENDERING_SOLID_COLOUR 0
#define RENDERING_TEXTURE 1
#define RENDERING_ANIMATED 2

class Particles {
    private:
    int _NextParticleIndex = 0;
    ComputeShader mainCompute;
    Shader Renderer;
    ShaderStorageBuffer SSBO;

    int RenderingType = 0; //0: Solid Colour, 1: Texture, 2: Animated
    glm::vec3 SolidColour = glm::vec3(1.0);
    TextureBuffer *Texture = nullptr;
    Animation *Spritesheet = nullptr;

    VertexBuffer DummyVBO;

    public:
    const int MaxParticles = 0;

    std::vector<Particle> particles;

    Particles(const char *computePath, int maxParticles);

    ~Particles();

    //BRUHH deconstructor is TECHNICALLY custom because of the variables inside

    Particles(Particles&& other) noexcept;

    Particles& operator=(Particles&& other) noexcept;

    int GetNextParticleIndex();

    void UploadToGPU();

    void Emit(glm::vec2 origin, int particleNum, float maxDuration, float size, float variation, float speed, bool hasGravity);

    void Update(glm::vec2 gravity);

    void RenderSolidColourState(glm::vec3 colour);

    void RenderTextureState(TextureBuffer &texture);

    void RenderAnimatedState(Animation &animation);

    void Render(glm::mat4 &view, glm::mat4 &projection);
};