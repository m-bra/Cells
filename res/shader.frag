#version 130
smooth in vec3 fragXYZ;
smooth in vec2 fragUV;
out vec4 fragRGBA;

uniform sampler2D tex;
uniform float tex_off_x;

void main()
{
    fragRGBA = texture(tex, fragUV + vec2(tex_off_x, 0));
}
