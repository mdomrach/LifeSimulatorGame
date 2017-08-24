#version 450 core

layout(binding = 0) uniform UniformBufferObject
{
	mat4 model;
	mat4 view;
	mat4 proj;
} ubo;

layout (location = 0) in vec2 inPosition;

layout(location = 0) out vec4 fragNormal;

out gl_PerVertex 
{
    vec4 gl_Position;   
};

void main(void)
{
    //gl_Position = ubo.proj * ubo.view * ubo.model * vec4(inPosition, 0.0, 1.0);
    gl_Position = ubo.proj * ubo.view * ubo.model * vec4(inPosition, 0.0, 1.0);
	//gl_Position.z = 0.5f;
	//gl_Position.w = 1.0f;
    fragNormal = vec4(1);
	//gl_Position = vec4(inPosition, 0.0, 1.0);
}
