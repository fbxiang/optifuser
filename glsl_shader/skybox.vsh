#version 450 core

uniform mat4 gbufferModelMatrix;
uniform mat4 gbufferViewMatrix;
uniform mat4 gbufferProjectionMatrix;

layout(location=0) in vec3 vpos;
layout(location=1) in vec3 vnormal;
layout(location=2) in vec2 vtexcoord;
layout(location=3) in vec3 vtangent;
layout(location=4) in vec3 vbitangent;
out vec3 texcoord;

void main() {
  gl_Position = gbufferProjectionMatrix * gbufferViewMatrix * vec4(vpos, 1.0);
  texcoord = vpos;
}
