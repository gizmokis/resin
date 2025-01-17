#version 330 core
#include "blinn_phong.glsl"

layout(location = 0) out vec4 fragColor;
layout(location = 1) out int id;

// fragment
in vec2 v_Pos;

uniform material u_material;

// https://raytracing.github.io/books/RayTracingInOneWeekend.html#surfacenormalsandmultipleobjects/simplifyingtheray-sphereintersectioncode
float hit_sphere(vec3 center, float radius, vec3 ray) {
    float a = dot(ray, ray);
    float h = dot(ray, center);
    float c = dot(center, center) - radius*radius;
    float discriminant = h*h - a*c;

    if (discriminant < 0.0) {
        return -1.0;
    } else {
        return (h - sqrt(discriminant)) / a;
    }
}

void main() {
    const float focal_length = 1.0;
    const vec3 sphere_pos = vec3(0.0, 0.0, -3.0);
    const float sphere_rad = 1.8;
    
    directional_light dir_light;
    dir_light.color = vec3(1.0, 1.0, 1.0);
    dir_light.dir = normalize(sphere_pos - vec3(1.0, 2.0, 0.5));
    dir_light.ambient_impact = 0.5;

    vec3 ray = normalize(vec3(v_Pos, -focal_length));

    float t = hit_sphere(sphere_pos, sphere_rad, ray);
    if (t > 0) {
        vec3 hit_point = ray * t;
        vec3 n = normalize(hit_point - sphere_pos);
        vec3 col = calc_dir_light(dir_light, u_material, n, -ray) * u_material.albedo + 
                    dir_light.ambient_impact * u_material.ka;
        fragColor = vec4(col, 1.0);
    }
    else {

        fragColor = vec4(0.0);
    }
}

