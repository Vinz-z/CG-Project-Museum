#version 450

layout(set = 1, binding = 1) uniform sampler2D texSampler;

layout(location = 0) in vec3 fragViewDir;
layout(location = 1) in vec3 fragNorm;
layout(location = 2) in vec2 fragTexCoord;

layout(location = 0) out vec4 outColor;

void main() {
	const vec3  diffColor = texture(texSampler, fragTexCoord).rgb;
	const vec3  specColor = vec3(1.0f, 1.0f, 1.0f);
	const float specPower = 150.0f;
	const vec3  L = vec3(-0.4830f, 0.8365f, -0.2588f);
	//const vec3 topColor = vec3(0.325f,0.847f,0.9843f);
	//const vec3 bottomColor = vec3(0.384f, 0.580f, 0.3764f);

	const vec3 topColor = vec3(0.1f,0.1f,0.1f);
	const vec3 bottomColor = vec3(0.1f, 0.1f, 0.1f);
	
	vec3 N = normalize(fragNorm);
	vec3 R = -reflect(L, N);
	vec3 V = normalize(fragViewDir);
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
	vec3 diffuse  = diffColor * max(dot(N,L), 0.0f);
	// Phong specular
	vec3 specular = specColor * pow(max(dot(R,V), 0.0f), specPower);
	// Hemispheric ambient
	vec3 HemiDir = vec3(0.0f, 1.0f, 0.0f);
	vec3 l_A = (((dot(N,HemiDir)+1.0f)/2.0f)*bottomColor) + (((1.0f-dot(N,HemiDir))/2.0f)*topColor);
	vec3 ambient  = (vec3(0.1f,0.1f, 0.1f) * (1.0f + N.y) + vec3(0.0f,0.0f, 0.1f) * (1.0f - N.y)) * diffColor;
	//vec3 ambient = (vec3(N.y + 1.0f) * diffuse) + l_A*vec3(0.1f, 0.1f, 0.1f);
	

	
	//outColor = vec4(clamp(ambient + diffuse + specular, vec3(0.0f), vec3(1.0f)), 1.0f);
	outColor = vec4(clamp(ambient + diffuse, vec3(0.0f), vec3(1.0f)), 1.0f);
}