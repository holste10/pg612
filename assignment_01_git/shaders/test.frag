#version 130
flat in vec3 ex_Color;
smooth in vec3 normal_smooth;
smooth in vec3 ex_View;
smooth in vec3 ex_Light;
out vec4 out_color;

// Texture
uniform sampler2D texture_sampler;
in vec2 ex_Texture_Coords;

void main() {
    vec3 h = normalize(ex_View + ex_Light);
    vec3 n = normalize(normal_smooth);
    float diff = max(0.1f, dot(n, ex_Light));
    float spec = pow(max(0.0f, dot(n, h)), 128.0f);
	vec4 textureColor = texture(texture_sampler, ex_Texture_Coords);

	vec3 testColor = ex_Color * textureColor;

	//out_color = vec4(textureColor, 1.0f);

    out_color =  textureColor  * vec4(ex_Color, 1.0f) + vec4(spec);
}