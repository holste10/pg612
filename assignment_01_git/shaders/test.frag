#version 130
flat in vec3 ex_Color;
smooth in vec3 normal_smooth;
smooth in vec3 ex_View;
smooth in vec3 ex_Light;
out vec4 out_color;

void main() {
    vec3 h = normalize(ex_View + ex_Light);
    vec3 n = normalize(normal_smooth);
    float diff = max(0.1f, dot(n, ex_Light));
    float spec = pow(max(0.0f, dot(n, h)), 500.0f);
    out_color = diff*vec4(ex_Color, 1.0f) + vec4(spec);
}