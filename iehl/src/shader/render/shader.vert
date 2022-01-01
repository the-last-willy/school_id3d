#version 450 core

#extension GL_ARB_shader_draw_parameters : require

uniform mat4 world_to_clip;

layout(location = 0) in vec3 normal;
layout(location = 1) in vec3 position;
layout(location = 2) in vec2 texcoords;

out flat uint v_draw_id;
out flat uint v_instance_index;
// World space.
out vec3 v_normal;
// World space.
out vec3 v_position;
out vec2 v_texcoords;

void main() {
    gl_Position = world_to_clip * vec4(position, 1.);
    v_draw_id = gl_DrawIDARB;
    v_instance_index = gl_BaseInstanceARB + gl_InstanceID;
    v_normal = normal;
    v_position = position;
    v_texcoords = texcoords;
}
