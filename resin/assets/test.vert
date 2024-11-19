#version 330 core
				
layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec4 a_Color;

out vec3 v_Pos;

void main() {
    v_Pos = a_Position;
    gl_Position = vec4(2*a_Position, 1.0);
}