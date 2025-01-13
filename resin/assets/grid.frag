#version 330 core

layout(location = 0) out vec4 fragColor;

// fragment
in vec2 v_Pos;

// camera
uniform mat4 u_iV;
uniform bool u_ortho;
uniform float u_nearPlane;

uniform float u_spacing = 1.0;

const vec4 minorLineColor = vec4(0.5, 0.5, 0.5, 0.7); 
const vec4 majorLineColor = vec4(0.7, 0.6, 0.6, 0.9); 
const vec4 XColor = vec4(1.0, 0.0, 0.0, 1.0);
const vec4 ZColor = vec4(0.0, 0.0, 1.0, 1.0);
const float lineThickness = 0.05;

const float epsilon = 1e-4;

void grid(vec3 rayOrigin, vec3 rayDir) {
    fragColor = vec4(0.0);

    float majorSpacing = 10.0 * u_spacing;

    if (abs(rayDir.y) < 0.0001) return;
    
    float t = -rayOrigin.y / rayDir.y;  
    vec3 pos = rayOrigin + t * rayDir;  // world-space intersection with XZ plane 

    if (!u_ortho && t <= 0.0) return; 

    vec3 modMinor = mod(pos, u_spacing);
    vec3 modMajor = mod(pos, majorSpacing);

    vec3 minorGrid = 1 - step(lineThickness, modMinor);
    vec3 majorGrid = 1 - step(lineThickness, modMajor);
    vec3 onAxis    = 1 - step(lineThickness, abs(pos));

    // Determine grid color
    vec4 color = minorLineColor * max(minorGrid.x, minorGrid.z);
    color = mix(color, majorLineColor, max(majorGrid.x, majorGrid.z));
    color = mix(color, XColor, onAxis.x);
    color = mix(color, ZColor, onAxis.z);

    // Fade the grid based on an angle (seems to give a better effect than based on distance)
    float fadeFactor = pow(abs(rayDir.y), 0.5);
    fragColor = vec4(color.rgb, color.a * fadeFactor);

    // TODO(SDF-116): Depth
}

void main() {
    vec4 ray_origin;    // vec4(ro, 1) as it needs to be translated
    vec4 ray_direction; // vec4(rd, 0) as it cannot be translated
    if (u_ortho) {
        ray_origin = vec4(v_Pos, 0, 1); // apply desired scaling from fov
        ray_direction = vec4(0, 0, -1, 0);
    } else {
        ray_origin = vec4(0, 0, 0, 1); 
        ray_direction = normalize(vec4(v_Pos, -u_nearPlane, 0));
    }
    
    grid((u_iV * ray_origin).xyz, ((u_iV * ray_direction).xyz));
}