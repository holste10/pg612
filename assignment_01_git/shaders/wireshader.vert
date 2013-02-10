#version 130
uniform mat4 projection_matrix;
uniform mat4 modelview_matrix;
uniform mat3 normal_matrix;
uniform vec3 color;

in  vec3 in_Position;
in  vec3 in_Normal;
flat out vec3 ex_Color;

void main() {
	vec4 pos = modelview_matrix * vec4(in_Position, 1.0f);
	gl_Position = projection_matrix * pos;
	ex_Color = color;
}