#include <cornbreadlib/utility.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

float quadData[24] = 
{   
    -0.5, -0.5,    0.0f, 0.0f,
    0.5, -0.5,    1.0f, 0.0f,
    0.5, 0.5,    1.0f, 1.0f,
    -0.5, -0.5,    0.0f, 0.0f,
    0.5, 0.5,    1.0f, 1.0f,
    -0.5, 0.5,    0.0f, 1.0f
};


float lerpToTime(float lerp, float DeltaTime) {
    float Smoothness = -60.0 * log(1.0 - lerp);
    return (1.0 - glm::exp(-Smoothness * DeltaTime));
}

float EaseIn(float lerp) {
    return glm::pow(lerp, 2);
}

float EaseOut(float lerp) {
    return 1 - glm::pow((1 - lerp), 2);
}

float EaseInOut(float lerp) {
    return lerp < 0.5 ? 2 * glm::pow(lerp, 2) : 1 - glm::pow(-2 * lerp + 2, 2) / 2;
}