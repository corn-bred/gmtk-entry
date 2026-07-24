#include <cornjam/particles.h>
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

Particles::Particles(const char *computePath, int maxParticles) : mainCompute(computePath), Renderer("src/shaders/particles.vert", "src/shaders/particles.frag"), MaxParticles(maxParticles), SSBO(nullptr, 0, GL_DYNAMIC_DRAW), DummyVBO(nullptr, 0, GL_STATIC_DRAW), particles(MaxParticles) {
    for (int i = 0; i < MaxParticles; i++) {
        particles[i].Origin = glm::vec2(0.0);
        particles[i].Position = glm::vec2(0.0);
        particles[i].Velocity = glm::vec2(0.0);
        particles[i].Duration = 0.0;
        particles[i].MaxDuration = 0.0;
        particles[i].Size = 0.0;
        particles[i].Speed = 0.0;
        particles[i].Flags = 0;

        particles[i].padding = 0.0f;
    }

    ShaderStorageBuffer tempSSBO(particles.data(), MaxParticles * sizeof(Particle), GL_DYNAMIC_DRAW);
    SSBO = std::move(tempSSBO);
    mainCompute.bind();
    SSBO.bindToShader(0);
}

Particles::~Particles() {
    Texture = nullptr;
    Spritesheet = nullptr;
}

//BRUHH deconstructor is TECHNICALLY custom because of the variables inside

Particles::Particles(Particles&& other) noexcept :
    mainCompute(std::move(other.mainCompute)),
    Renderer(std::move(other.Renderer)),
    SSBO(std::move(other.SSBO)),
    DummyVBO(std::move(other.DummyVBO)),
    particles(std::move(other.particles)),
    Texture(other.Texture),
    Spritesheet(other.Spritesheet),
    RenderingType(other.RenderingType),
    SolidColour(other.SolidColour) {
    
    other.Texture = nullptr;
    other.Spritesheet = nullptr;
} //do allat

Particles &Particles::operator=(Particles&& other) noexcept {
    if (this != &other) {
        // move assignment
        mainCompute = std::move(other.mainCompute);
        Renderer = std::move(other.Renderer);
        SSBO = std::move(other.SSBO);
        DummyVBO = std::move(other.DummyVBO);
        particles = std::move(other.particles);
        
        // move all the pointer classes and stuff
        Texture = other.Texture;
        Spritesheet = other.Spritesheet;
        
        RenderingType = other.RenderingType;
        SolidColour = other.SolidColour;
        
        // then destroy
        other.Texture = nullptr;
        other.Spritesheet = nullptr;
    }
    return *this;
}

int Particles::GetNextParticleIndex() {
    int start = _NextParticleIndex;

    // 1. Try to find a dead particle starting from the cursor
    for (int i = 0; i < (int)particles.size(); i++) {
        int index = (start + i) % particles.size();
        if (!(particles[index].Flags & FLAG_IS_ACTIVE)) {
            _NextParticleIndex = (index + 1) % particles.size();// Go to the one after index
            return index;
        }
    }

    // 2. If all are alive, overwrite the cursor position (oldest)
    int index = _NextParticleIndex;
    _NextParticleIndex = (_NextParticleIndex + 1) % particles.size(); //+
    return index;
}

void Particles::UploadToGPU() {
    SSBO.updateData(particles.data(), MaxParticles * sizeof(Particle));
}

void Particles::Emit(glm::vec2 origin, int particleNum, float maxDuration, float size, float variation, float speed, bool hasGravity) {
    for (int i = 0; i < particleNum; i++) {
        int index = GetNextParticleIndex();
        particles[index].Origin = origin;
        particles[index].Position = origin;
        double RandomDouble = randomFloat();
        particles[index].Velocity = glm::vec2(cos(RandomDouble * PI * 2) * speed, sin(RandomDouble * PI * 2) * speed) * (1 + randomFloat() * variation);
        particles[index].Duration = 0.0;
        particles[index].MaxDuration = maxDuration;
        particles[index].Size = size;
        
        particles[index].Speed = speed;

        particles[index].padding = 0.0f;

        particles[index].Flags = FLAG_IS_ACTIVE;
        if (hasGravity) particles[index].Flags |= FLAG_HAS_GRAVITY;
    }
    UploadToGPU();
}

void Particles::Update(glm::vec2 gravity) {
    mainCompute.bind();
    
    mainCompute.setFloat("uDeltaTime", DeltaTime);
    mainCompute.setVec2("uGravity", gravity);
    mainCompute.setFloat("uCurrentTime", CurrentTime);

    mainCompute.use((MaxParticles + 255) / 256, 1, 1, GL_SHADER_STORAGE_BARRIER_BIT | GL_VERTEX_ATTRIB_ARRAY_BARRIER_BIT);
}

void Particles::RenderSolidColourState(glm::vec3 colour) {
    RenderingType = RENDERING_SOLID_COLOUR;
    SolidColour = colour;
}

void Particles::RenderTextureState(TextureBuffer &texture) {
    RenderingType = RENDERING_TEXTURE;
    Texture = &texture;
}

void Particles::RenderAnimatedState(Animation &animation) {
    RenderingType = RENDERING_ANIMATED;
    Spritesheet = &animation;
}

void Particles::Render(glm::mat4 &view, glm::mat4 &projection) {
    Renderer.use();
    Renderer.setMat4("view", view);
    Renderer.setMat4("projection", projection);

    switch(RenderingType) {
        case 0:
        Renderer.setBool("isSolidColour", true);
        Renderer.setVec3("Colour", SolidColour);
        break;

        case 1:
        Renderer.setBool("isSolidColour", false);
        Texture->bindTexture(0);
        break;

        case 2:
        Renderer.setBool("isSolidColour", false);
        Renderer.setBool("isAnimation", true);
        Renderer.setInt("Columns", Spritesheet->SpriteRenderer.Columns);
        Renderer.setInt("Rows", Spritesheet->SpriteRenderer.Rows);
        Renderer.setInt("Frame", Spritesheet->FrameHandler.GetFrame());
        Spritesheet->SpriteRenderer.Sprite.bindTexture(0);
        break;

        default:
        Renderer.setBool("isSolidColour", true);
        Renderer.setVec3("Colour", glm::vec3(1.0));
        break;
    }

    DummyVBO.bind();

    glDrawArrays(GL_TRIANGLES, 0, MaxParticles * 6);
}
