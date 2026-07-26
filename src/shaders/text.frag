#version 430 core

in vec2 iTexCoords;
uniform sampler2D FontTexture;
uniform vec4 ColourMultiplier;

out vec4 FragColour;

void main() {
    vec4 TexColour = vec4(vec3(1.0) - texture(FontTexture, iTexCoords).rgb, texture(FontTexture, iTexCoords).a) * ColourMultiplier;
    FragColour = TexColour;
}