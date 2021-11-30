#version 450 core

struct Light {
    vec4 color;
    vec4 position;
};

struct Material {
    vec4 color_factor;
    vec4 emission_factor;
};

layout(binding = 0) uniform sampler2DArray albedo_array_texture;

layout(std430, binding = 0) buffer material_ids {
    int triangle_material_ids[/*primitive id*/];
};

layout(std430, binding = 1) buffer material_buffer {
    Material materials[/*material id*/];
};

layout(std430, binding = 2) buffer primitive_offset_buffer {
    uint primitive_offsets[/*draw id*/]; 
};

layout(std430, binding = 3) readonly buffer light_buffer {
    Light lights[];
};

uniform int light_count = 0;

in flat uint vertex_draw_id;
in vec3 vertex_normal;
in vec3 vertex_position;
in vec2 vertex_texcoords;

out vec4 fragment_color;

void main() {
    uint primitive_id = primitive_offsets[vertex_draw_id] + gl_PrimitiveID;

    triangle_material_ids.length;

    vec3 normal = normalize(vertex_normal);

    vec3 color = normal * 0.5 + 0.5;
    fragment_color = vec4(color, 1.);

    vec3 lighting = vec3(0.);
    {
        for(int l = 0; l < light_count; ++l) {
            Light light = lights[l]; 
            float distance = max(distance(vertex_position, light.position.xyz), 1.);
            lighting += light.color.xyz / distance;
        }
    }

    fragment_color = vec4(lighting, 1.);

    // int material_id = triangle_material_ids[primitive_id];
    // if(material_id != -1) {
    //     const Material material = materials[material_id];
    //     vec4 texel = texture(albedo_array_texture, vec3(vertex_texcoords, material_id));
    //     fragment_color = material.emission_factor * material.color_factor * texel;
    // } else {
    //     fragment_color = vec4(vec3(1., 1., 1.), 1.);
    // }
}
