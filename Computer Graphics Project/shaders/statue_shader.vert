#version 450

layout(set=0, binding = 0) uniform GlobalUniformBufferObject {
	mat4 view;
	mat4 proj;
} gubo;

layout(set=1,binding = 0) uniform UniformBufferObject {
	mat4 model;
} ubo;

layout(location = 0) in vec3 pos;
layout(location = 1) in vec3 norm;
layout(location = 2) in vec2 texCoord;

layout(location = 0) out vec3 fragViewDir;
layout(location = 1) out vec3 fragNorm;
layout(location = 2) out vec2 fragTexCoord;

layout(push_constant) uniform Push {
	mat4 wordMat;
} push;

void main() {
	gl_Position = gubo.proj * gubo.view * push.wordMat * vec4(pos, 1.0);
	fragViewDir  = (gubo.view[3]).xyz - (push.wordMat * vec4(pos,  1.0)).xyz;
	fragNorm     = (push.wordMat * vec4(norm, 0.0)).xyz;
	fragTexCoord = texCoord;
}