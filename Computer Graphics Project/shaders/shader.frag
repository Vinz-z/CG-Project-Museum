#version 450

layout(set = 1, binding = 1) uniform sampler2D texSampler;

layout(location = 0) in vec3 fragPos;
layout(location = 1) in vec3 fragNorm;
layout(location = 2) in vec2 fragTexCoord;

layout(location = 0) out vec4 outColor;

layout(set = 0, binding = 0) uniform GlobalUniformBufferObject {
	mat4 view;
	mat4 proj;
	vec3 lightPos[3];
	vec3 lightColor;
	vec4 coneInOutDecayExp;
} gubo;

vec3 point_light_dir(vec3 lightPos ,vec3 pos) {
	// Point light direction
	return normalize(lightPos - pos);
}

vec3 point_light_color(vec3 lightPos, vec3 pos) {
	// Point light color
	return gubo.lightColor * pow(gubo.coneInOutDecayExp.z/length(lightPos - pos),gubo.coneInOutDecayExp.w);
}

vec4 createPointLight(vec3 lightPos, vec3 pos, vec3 N, vec3 V, vec3 diffColor, float specPower){
	vec3 lightDir = normalize(lightPos - pos);
	vec3 lightColor = gubo.lightColor * pow(gubo.coneInOutDecayExp.z / length(lightPos - pos), gubo.coneInOutDecayExp.w);
	vec3 R = -reflect(lightDir, N);
	vec3 lambertDiffuse = diffColor * max(dot(N, lightDir), 0.0f);
	vec3 phongSpecular = vec3(pow(max(dot(R,V),0.0f), specPower));

	vec3 ambient  = (vec3(0.1f,0.1f, 0.1f) * (1.0f + N.y) + vec3(0.0f,0.0f, 0.1f) * (1.0f - N.y)) * diffColor;

	vec4 pointLight = vec4((lambertDiffuse + phongSpecular + ambient) * lightColor, 1.0f);
	return pointLight;
}

void main() {
	const vec3  diffColor = texture(texSampler, fragTexCoord).rgb;
	const float specPower = 64.0f;
	vec3 N = normalize(fragNorm);
	vec3 V = normalize((gubo.view[3]).xyz - fragPos);
	/*--------------------------------------------------------------
	//const vec3  L = vec3(-0.4830f, 0.8365f, -0.2588f);
	const vec3  L = vec3(0.0f, -0.5f, 0.0f);
	//const vec3 topColor = vec3(0.325f,0.847f,0.9843f);
	//const vec3 bottomColor = vec3(0.384f, 0.580f, 0.3764f);

	const vec3 topColor = vec3(0.1f,0.1f,0.1f);
	const vec3 bottomColor = vec3(0.1f, 0.1f, 0.1f);

	//point light
	vec3 lD1;	// light direction from the light model
	vec3 lD2;
	vec3 lC1;	// light color and intensity from the light model
	vec3 lC2;

	lD1 = point_light_dir(gubo.lightPos1, fragPos) ;
	lD2 = point_light_dir(gubo.lightPos2, fragPos);
	lC1 = point_light_color(gubo.lightPos1, fragPos);
	lC2 = point_light_color(gubo.lightPos2, fragPos);
	
	
	vec3 R1 = -reflect(lD1, N);
	vec3 R2 = -reflect(lD2, N);
	
	float sigma = 0.5;
	
	//Oren_Nayar_Diffuse
	float alpha = max(acos(dot(L,N)),acos(dot(V,N)));
	float beta = min(acos(dot(L,N)),acos(dot(V,N)));
	float A = 1 - 0.5 * ((sigma*sigma) / ((sigma*sigma)+0.33));
	float B = 0.45 * ((sigma*sigma)/((sigma*sigma)+0.09));
	vec3 v_i = normalize(L - (dot(L,N)*N));
	vec3 v_r = normalize(V - (dot(V,N)*N));
	float G = max(0,dot(v_i,v_r));
	vec3 Oren = diffColor * clamp(dot(L,N),0,1);
	//vec3 diffuse = Oren * (A + B*G*sin(alpha)*tan(beta));



	// Lambert diffuse
	vec3 diffuse1  = diffColor * max(dot(N,lD1), 0.0f);
	vec3 diffuse2  = diffColor * max(dot(N,lD2), 0.0f);
	// Phong specular
	vec3 specular1 = vec3(pow(max(dot(R1,V), 0.0f), specPower));
	vec3 specular2 = vec3(pow(max(dot(R2,V), 0.0f), specPower));

	// Hemispheric ambient
	vec3 HemiDir = vec3(0.0f, 1.0f, 0.0f);
	vec3 l_A = (((dot(N,HemiDir)+1.0f)/2.0f)*bottomColor) + (((1.0f-dot(N,HemiDir))/2.0f)*topColor);
	vec3 ambient  = (vec3(0.1f,0.1f, 0.1f) * (1.0f + N.y) + vec3(0.0f,0.0f, 0.1f) * (1.0f - N.y)) * diffColor;
	//vec3 ambient = (vec3(N.y + 1.0f) * diffuse) + l_A*vec3(0.1f, 0.1f, 0.1f);
	*/
	//outColor = vec4(clamp(ambient + diffuse + specular, vec3(0.0f), vec3(1.0f)), 1.0f);
	outColor = createPointLight(gubo.lightPos[0], fragPos, N, V, diffColor, specPower);
	for (int i = 1; i < (gubo.lightPos).length(); i++){
		outColor = outColor + createPointLight(gubo.lightPos[i], fragPos, N, V, diffColor, specPower);
	}
	//outColor = vec4((diffuse1 + specular1) * lC1, 1.0f) + vec4((diffuse2 + specular2) * lC2, 1.0f);
}