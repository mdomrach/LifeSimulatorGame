#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 fragNormal;

layout(location = 0) out vec4 outColor;

void main() 
{
	vec3 normal = normalize(fragNormal);
	vec3 lightDirection = vec3(0.577, 0.577, 0.577);
	float lambertDiffuse = dot(normal, lightDirection);
	
    outColor = vec4(lambertDiffuse, lambertDiffuse, lambertDiffuse, 1.0);
}