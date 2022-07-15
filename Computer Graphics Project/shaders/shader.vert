#version 450

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

layout(push_constant) uniform Push {
    mat4 wordMat;
	vec2 reflectance;

} push;

layout(location = 0) in vec3 pos;
layout(location = 1) in vec3 norm;
layout(location = 2) in vec2 texCoord;

layout(location = 0) out vec3 fragPos;
layout(location = 1) out vec3 fragNorm;
layout(location = 2) out vec2 fragTexCoord;

void main() {
	gl_Position = gubo.proj * gubo.view * push.wordMat * vec4(pos, 1.0);
	fragPos = (push.wordMat* vec4(pos,  1.0)).xyz;
	fragNorm = (push.wordMat * vec4(norm, 0.0)).xyz;
	fragTexCoord = texCoord;
}