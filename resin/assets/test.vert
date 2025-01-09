#version 330 core
				
layout(location = 0) in vec3 a_Position;

uniform vec2 u_resolution;
uniform float u_camSize;

out vec2 v_Pos;

void main() {
    v_Pos = a_Position.xy * u_camSize / 2.0; // the division by 2 is required to match the glm::perspective
    v_Pos.x *= u_resolution.x / u_resolution.y;

    gl_Position = vec4(a_Position, 1.0);
}
