#version 130
uniform mat4 projection_matrix;
uniform mat4 modelview_matrix;
uniform mat3 normal_matrix;
uniform vec3 color;

in  vec3 in_Position;
in  vec3 in_Normal;
in	vec2 in_Texture_Coords;

flat out vec3 ex_Color;
smooth out vec3 ex_View;
smooth out vec3 ex_Light;
smooth out vec3 normal_smooth;
out vec2 ex_Texture_Coords;

void main() {
	vec4 pos = modelview_matrix * vec4(in_Position, 1.0);
	ex_View = normalize(-pos.xyz);
	ex_Light = normalize(vec3(200.0f, 200.0f, 200.0f) - pos.xyz);
	gl_Position = projection_matrix * pos;
	ex_Color = color;

	ex_Texture_Coords = in_Texture_Coords;

	normal_smooth = normal_matrix * in_Normal;
}

