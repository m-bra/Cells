#version 130
smooth in vec3 fragXYZ;
smooth in vec2 fragUV;
out vec4 fragRGBA;

uniform sampler2D tex;
uniform float tex_off_y;
uniform vec4 overlay_color;

void main()
{
    vec2 tex_off = vec2(0, tex_off_y);
    fragRGBA = texture(tex, fragUV + tex_off) + overlay_color
	+ vec4(fragXYZ.z * 0.2, fragXYZ.z * 0.2, fragXYZ.z * 0.2, 0);
}
