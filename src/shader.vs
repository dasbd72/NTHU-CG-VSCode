#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aColor;

out vec3 vertex_color;
uniform mat4 mvp;

void main()
{
	// [TODO]
	gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);
	gl_Position = mvp * gl_Position;
	vertex_color = aColor;
}

