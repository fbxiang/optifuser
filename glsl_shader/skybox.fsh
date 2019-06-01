#version 450 core

in vec3 texcoord;

uniform samplerCube skybox;

layout (location=0) out vec4 GCOLOR;

void main() {
  GCOLOR = texture(skybox, texcoord);
}
