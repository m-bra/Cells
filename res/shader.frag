#version 130
smooth in vec3 fragXYZ;
smooth in vec2 fragUV;
out vec4 fragRGBA;

uniform sampler2D tex;

void main()
{
    fragRGBA = texture(tex, fragUV);
    //fragRGBA = vec4(1, 0, 0, 1);
}
