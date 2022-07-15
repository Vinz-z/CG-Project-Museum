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

layout(set = 1, binding = 1) uniform sampler2D skybox;

layout(location = 0) in vec2 fragTexCoord;

layout(location = 0) out vec4 outColor;

void main() {
	float coeff = clamp(dot(gubo.sunLightDir, vec3(0.0f, 1.0f, 0.0f)), 0.01f, 1.0f);
	outColor = texture(skybox, fragTexCoord) * vec4(coeff,coeff, coeff, 1.0f);
	//outColor = vec4(0.545f, 0.545f, 0.545f, 1.0f);
	//outColor = texture(skybox, fragTexCoord);
}