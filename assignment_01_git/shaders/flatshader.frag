#version 130
flat in vec3 ex_Color;
out vec4 out_color;

uniform sampler2D texture_sampler;
in vec2 ex_Texture_Coords;

void main() {
	vec4 textureColor = texture(texture_sampler, ex_Texture_Coords);
	out_color = textureColor * vec4(ex_Color, 1.0f);
}