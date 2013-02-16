#version 130
uniform mat4 projection_matrix;
uniform mat4 modelview_matrix;
uniform mat3 normal_matrix;
uniform vec3 color;

in vec3 in_Position;
in vec3 in_Normal;
in vec2 in_Texture_Coords;

flat out vec3 ex_Color;
out vec2 ex_Texture_Coords;

void main() {
	vec4 pos = modelview_matrix * vec4(in_Position, 1.0f);
	
	vec3 view = normalize(-pos.xyz);
	vec3 light = normalize(vec3(200.0f, 200.0f, 200.0f) - pos.xyz);

	vec3 h = normalize(view + light);
	vec3 n = normalize(normal_matrix * in_Normal);

	float diff = max(0.1f, dot(n, light));

	gl_Position = projection_matrix * pos;

	ex_Texture_Coords = in_Texture_Coords;
	ex_Color = diff * color;
}