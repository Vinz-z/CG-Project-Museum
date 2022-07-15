#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(set = 0, binding = 0) uniform GlobalUniformBufferObject {
	mat4 view;
	mat4 proj;
	vec3 lightPos[11];
	vec3 lightColor;
	vec3 sunLightDir;
	vec3 sunLightColor;
	vec4 coneInOutDecayExp;
} gubo;

layout(set = 1, binding = 0) uniform UniformBufferObject {
	mat4 model;
} ubo;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 inTexCoord;

layout(location = 0) out vec2 fragTexCoord;

void main()
{
    //fragTexCoord = inPosition;
	fragTexCoord = inTexCoord;
	gl_Position = gubo.proj * gubo.view * ubo.model * vec4(inPosition, 1.0);
}  