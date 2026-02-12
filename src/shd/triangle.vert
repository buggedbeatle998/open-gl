#version 460 core

precision mediump float;

layout (location = 0) in vec2 v_pos;
layout (location = 1) in vec3 v_col;
layout (location = 2) uniform mat4 mvp;

layout (location = 0) out vec3 colour;

void main() {
    gl_position = mvp * vec4(v_pos, 0.0, 1.0);
    colour = v_col;
}
