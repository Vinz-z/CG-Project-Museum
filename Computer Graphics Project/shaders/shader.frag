#version 450

layout(set = 1, binding = 1) uniform sampler2D texSampler;

layout(location = 0) in vec3 fragPos;
layout(location = 1) in vec3 fragNorm;
layout(location = 2) in vec2 fragTexCoord;
layout(location = 3) in float reflectance;

layout(location = 0) out vec4 outColor;

layout(set = 0, binding = 0) uniform GlobalUniformBufferObject {
	mat4 view;
	mat4 proj;
	vec3 lightPos[11];
	vec3 lightColor;
	vec3 sunLightDir;
	vec3 sunLightColor;
	vec2 coneInOutDecayExp;
} gubo;

vec3 point_light_dir(vec3 lightPos ,vec3 pos) {
	// Point light direction
	return normalize(lightPos - pos);
}

vec3 point_light_color(vec3 lightPos, vec3 pos) {
	// Point light color
	return gubo.lightColor * pow(gubo.coneInOutDecayExp.x/length(lightPos - pos),gubo.coneInOutDecayExp.y);
}

vec4 createPointLight(vec3 lightPos, vec3 pos, vec3 N, vec3 V, vec3 diffColor, float specPower){
	vec3 lightDir = normalize(lightPos - pos);
	vec3 lightColor = gubo.lightColor * pow(gubo.coneInOutDecayExp.x / length(lightPos - pos), gubo.coneInOutDecayExp.y);
	vec3 R = -reflect(lightDir, N);
	vec3 lambertDiffuse = diffColor * max(dot(N, lightDir), 0.0f);
	vec3 phongSpecular;

	vec3 ambient = vec3(0.4f,0.4f,0.4f) * diffColor;

	if (specPower != 0){
		phongSpecular = vec3(pow(max(dot(R,V),0.0f), specPower));
	} else {
		phongSpecular = vec3(0.0f, 0.0f, 0.0f);
	}

	vec4 pointLight = vec4((lambertDiffuse + ambient + phongSpecular) * lightColor, 1.0f);
	return pointLight;
}

vec4 createSunLight(vec3 N, vec3 V, vec3 diffColor, float specPower){
	vec3 lightDir = gubo.sunLightDir;
	vec3 lightColor = gubo.sunLightColor;
	vec3 R = -reflect(lightDir, N);
	vec3 lambertDiffuse = diffColor * max(dot(N, lightDir), 0.0f);
	vec3 phongSpecular;

	vec3 ambient = vec3(0.4f,0.4f,0.4f) * diffColor;

	if (specPower != 0){
		phongSpecular = vec3(pow(max(dot(R,V),0.0f), specPower));
	} else {
		phongSpecular = vec3(0.0f, 0.0f, 0.0f);
	}

	vec4 sunLight = vec4((lambertDiffuse + ambient + phongSpecular) * lightColor, 1.0f);
	return sunLight;
}



void main() {
	const vec3  diffColor = texture(texSampler, fragTexCoord).rgb;
	float specPower = reflectance;
	vec3 N = normalize(fragNorm);
	vec3 V = normalize((gubo.view[3]).xyz - fragPos);
	
	outColor = createPointLight(gubo.lightPos[0], fragPos, N, V, diffColor, specPower);
	for (int i = 1; i < (gubo.lightPos).length(); i++){
		outColor = outColor + createPointLight(gubo.lightPos[i], fragPos, N, V, diffColor, specPower);
	} 
	outColor = outColor + createSunLight(N, V, diffColor, specPower);

}