#version 130
in vec3 vertXYZ;
in vec2 vertUV;
smooth out vec3 fragXYZ;
smooth out vec2 fragUV;

uniform mat4 mvp;

void main()
{
	gl_Position = mvp * vec4(vertXYZ, 1);
	fragXYZ = vertXYZ;
	fragUV = vertUV;
}
