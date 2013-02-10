#version 130
flat in vec3 ex_Color;
out vec4 out_color;

void main() {
	out_color = vec4(ex_Color, 1.0f);
}