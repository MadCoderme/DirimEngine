#version 330 core
in vec3 TexCoord;
out vec4 FragColor;

uniform samplerCube texture_skybox;

void main() {
    FragColor = texture(texture_skybox, TexCoord);
}