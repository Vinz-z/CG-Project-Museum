#version 450

layout(set = 1, binding = 1) uniform sampler2D skybox;

layout(location = 0) in vec2 fragTexCoord;

layout(location = 0) out vec4 outColor;

void main() {
	outColor = texture(skybox, fragTexCoord);
	//outColor = vec4(0.545f, 0.545f, 0.545f, 1.0f);
	//outColor = texture(skybox, fragTexCoord);
}