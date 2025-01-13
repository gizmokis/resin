#version 330 core

layout(location = 0) out vec4 fragColor;

// fragment
in vec2 v_Pos;

// camera
uniform mat4 u_iV;
uniform bool u_ortho;
uniform float u_nearPlane;
uniform float u_farPlane;

uniform float u_spacing = 1.0;

const vec4 minorLineColor = vec4(0.5, 0.5, 0.5, 0.7); 
const vec4 majorLineColor = vec4(0.7, 0.6, 0.6, 0.9); 
const vec4 XColor = vec4(1.0, 0.0, 0.0, 1.0);
const vec4 ZColor = vec4(0.0, 0.0, 1.0, 1.0);
const float lineThickness = 0.05;

const float epsilon = 1e-4;

float grid(vec3 rayOrigin, vec3 rayDir) {
    fragColor = vec4(0.0);

    float majorSpacing = 10.0 * u_spacing;

    if (abs(rayDir.y) < 0.0001) return u_farPlane;
    
    float t = -rayOrigin.y / rayDir.y;  
    vec3 pos = rayOrigin + t * rayDir;  // world-space intersection with XZ plane 

    if (!u_ortho && t <= 0.0) return u_farPlane; 

    vec3 modMinor = mod(pos, u_spacing);
    vec3 modMajor = mod(pos, majorSpacing);

    vec3 minorGrid = 1 - step(lineThickness, modMinor);
    vec3 majorGrid = 1 - step(lineThickness, modMajor);
    vec3 onAxis    = 1 - step(lineThickness, abs(pos));

    // Determine grid color
    vec4 color = minorLineColor * max(minorGrid.x, minorGrid.z);
    color = mix(color, majorLineColor, max(majorGrid.x, majorGrid.z));
    color = mix(color, XColor, onAxis.z);
    color = mix(color, ZColor, onAxis.x);

    if(color.a == 0) return u_farPlane;

    // Fade the grid based on an angle (seems to give a better effect than based on distance)
    float fadeFactor = pow(abs(rayDir.y), 0.5);
    fragColor = vec4(color.rgb, color.a * fadeFactor);

    return t;
}

void main() {
    vec4 ray_origin;    // vec4(ro, 1) as it needs to be translated
    vec4 ray_direction; // vec4(rd, 0) as it cannot be translated
    if (u_ortho) {
        ray_origin = vec4(v_Pos, 0, 1);
        ray_direction = vec4(0, 0, -1, 0);
    } else {
        ray_origin = vec4(0, 0, 0, 1); 
        ray_direction = normalize(vec4(v_Pos, -u_nearPlane, 0));
    }
    
    float t = grid((u_iV * ray_origin).xyz, ((u_iV * ray_direction).xyz));

    // Depth
    float A = u_nearPlane * u_farPlane;
    float B = u_farPlane - u_nearPlane;
    if (u_ortho) {
        gl_FragDepth = (t - u_nearPlane) / B;
    } else {
        gl_FragDepth = (A/(t * ray_direction.z) + u_farPlane) / B;
    }
}