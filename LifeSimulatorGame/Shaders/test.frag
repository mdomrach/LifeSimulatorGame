#version 450 core

layout(location = 0) in vec4 fragNormal;

layout (location = 0) out vec4 outFragColor;

void main(void)
{
	outFragColor = fragNormal;
}
