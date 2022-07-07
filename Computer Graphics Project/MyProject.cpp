#include "MyProject.hpp";
#include <list>

const std::string MODEL_PATH = "models/";
const std::string TEXTURE_PATH = "textures/";

// The uniform buffer object used in this example
struct GlobalUniformBufferObject {
	alignas(16) glm::mat4 view; // alignas lo usa cpp per allineare i byte della matrice... 
	alignas(16) glm::mat4 proj; // la shader puo avere problemi con dei padding tra campi di una struttura
};

struct UniformBufferObject {
	alignas(16) glm::mat4 model;
};

struct SceneObject {
	Texture texture;
	Model mesh;
	Model colliderMesh;

	DescriptorSet DS;
	DescriptorSetLayout DSL;
	UniformBufferObject ubo;

	void init(DescriptorSetLayout *global, BaseProject *bs, std::string modelString, std::string textureString, glm::mat4 initialPosition) {}
	void populateCommandBuffer(VkCommandBuffer commandBuffer, int currentImage, Pipeline pipeline) {}
	void cleanup() {}
	void handleClick() {}
};

// -------------------- start Player --------------------

glm::vec3 vectorProjection(glm::vec3 from, glm::vec3 to) {
	//                  dot(u,v)
	// proj_v(u) = ------------------- * v -> the projection of u on v
	//               v.x*v.x + v.y*v.y     -> it should be the squared modulo of the vect
	
	return (glm::dot(from, to)) * to /
				glm::dot(to, to);
}

void printVec(std::string label, glm::vec3 vvv) {
	std::cout << " + " << label << ": " << vvv[0] << " " << vvv[1] << " " << vvv[2] << std::endl;
}

struct Camera {
	void init(glm::vec3 angles, glm::vec3 position, float near, float far, float fov, float aspectRatio) {
		this->angles = angles;
		this->position = position;
		this->near = near;
		this->far = far;
		this->fov = fov;

		projection = glm::perspective(glm::radians(fov), aspectRatio, near, far);
		projection[1][1] *= -1;
	}

	void rotate(glm::vec3 deltaRotation) {
		angles += deltaRotation;
		camera = 
			glm::rotate(glm::mat4(1.0f), angles.y, glm::vec3(0.0f, 1.0f, 0.0f)) *
			glm::rotate(glm::mat4(1.0f), glm::clamp(angles.x, -glm::half_pi<float>(), glm::half_pi<float>()), glm::vec3(1.0f, 0.0f, 0.0f)) *
			glm::rotate(glm::mat4(1.0f), angles.z, glm::vec3(0.0f, 0.0f, 1.0f));
	}

	void move(glm::vec3 ds) {
		position += ds;
	}

	glm::mat4 getCameraMatrix() {
		return glm::translate(glm::transpose(camera), -position);
	}

	glm::mat4 getProjectionMatrix() {
		return projection;
	}

	glm::vec4 getViewDirection() {
		return
			glm::rotate(glm::mat4(1.0), angles.z, glm::vec3(0, 0, 1)) *
			glm::rotate(glm::mat4(1.0), glm::clamp(angles.x, -glm::half_pi<float>(), glm::half_pi<float>()), glm::vec3(1, 0, 0)) *
			glm::rotate(glm::mat4(1.0), angles.y, glm::vec3(0, 1, 0)) *
			glm::vec4(0, 0, 1, 1);
		//return glm::transpose(camera) * glm::vec4(0,0,1,1);
	}

private:
	glm::vec3 angles;
	glm::vec3 position;
	float near;
	float far;
	float fov;
	glm::mat4 camera;
	glm::mat4 projection;
};

struct Triangle {
	glm::vec3 A;
	glm::vec3 B;
	glm::vec3 C;
	glm::vec3 AB;
	glm::vec3 AC;
	glm::vec3  norm;

	Triangle (glm::vec3 a, glm::vec3 b, glm::vec3 c) {
		this->A = a;
		this->B = b;
		this->C = c;
		this->AB = B - A;
		this->AC = C - A;
		norm = glm::cross(AB, AC);
		//norm.x = norm.x < 0.00001 ? 0.0f : norm.x;
		//norm.y = norm.y < 0.00001 ? 0.0f : norm.y;
		//norm.z = norm.z < 0.00001 ? 0.0f : norm.z;
	}

	bool collide(glm::vec3 pos, glm::vec3 dir) {
		// find the intersection between the plane of the triangle and the line 
		// of the direction u are moving
		float d = glm::dot(norm, A);
		float perp = glm::dot(dir, norm);
		if (perp == 0) return false;
		float t = (d - glm::dot(norm, pos)) / perp;

		glm::vec3 intersection = pos + (dir * t);

		// walking on the triangle you must have the intersection always on the same side
		// check it using dot between edge and point - start edge: the sign need to be always the same
		//int a = glm::dot(intersection - A, B - A) >= 0;
		//int b = glm::dot(intersection - B, C - B) >= 0;
		//int c = glm::dot(intersection - C, A - C) >= 0;

		Triangle a{ A, B, intersection };
		Triangle b{ B, C, intersection };
		Triangle c{ C, A, intersection };

		return
			glm::dot(glm::normalize(a.norm), glm::normalize(b.norm)) == 1.0f &&
			glm::dot(glm::normalize(b.norm), glm::normalize(c.norm)) == 1.0f &&
			glm::length(intersection - pos) <= (glm::length(dir) + 0.1f);
	}

	void print() {
		printVec("A", A);
		printVec("B", B);
		printVec("C", C);
		printVec("norm", norm);
	}
};

struct Player {
	std::list<Triangle> boundaries;
	Camera camera;

	const float movementSpeed = 3.0f;
	const float jumping_speed = 0.5f;

	void init(float aspectRatio, glm::vec3 initPos) {
		position = initPos;
		camera.init(glm::vec3(0.0f, 0.0f, 0.0f), position, 0.1f, 100.0f, 70.0f, aspectRatio);
	}

	void forward(float dt) {
		auto vd = camera.getViewDirection();
		auto ds = (vectorProjection(vd, glm::vec3(1, 0, 0)) + vectorProjection(vd, glm::vec3(0, 0, 1))) 
			* movementSpeed 
			* dt;
		move(-ds);
	}

	void backward(float dt) {
		auto vd = camera.getViewDirection();
		auto ds = (vectorProjection(vd, glm::vec3(1, 0, 0)) + vectorProjection(vd, glm::vec3(0, 0, 1)))
			* movementSpeed
			* dt;
		move(ds);
	}

	void left(float dt) {
		auto vd = glm::rotate(glm::mat4(1), glm::half_pi<float>(), glm::vec3(0, 1, 0)) * camera.getViewDirection();
		auto ds = (vectorProjection(glm::vec3(vd), glm::vec3(1, 0, 0)) + vectorProjection(glm::vec3(vd), glm::vec3(0, 0, 1)))
			* movementSpeed
			* dt;
		move(-ds);
	}

	void right(float dt) {
		auto vd = glm::rotate(glm::mat4(1), glm::half_pi<float>(), glm::vec3(0, 1, 0)) * camera.getViewDirection();
		auto ds = (vectorProjection(glm::vec3(vd), glm::vec3(1, 0, 0)) + vectorProjection(glm::vec3(vd), glm::vec3(0, 0, 1)))
			* movementSpeed
			* dt;
		move(ds);
	}

	void moveHead(glm::vec3 deltaRotation) {
		camera.rotate(deltaRotation);
	}

	void addTriangle(Triangle t) {
		boundaries.push_front(t);
	}

private:
	glm::vec3 position;

	void move(glm::vec3 dir) {
		// check for each boundary if there is a collision -> if yes then cant move in this direction
		for (Triangle& t : boundaries) { if (t.collide(position, dir)) { return; } }
		position += dir;
		camera.move(dir);
	}
};

// -------------------- end Player --------------------

// -------------------- start Picture --------------------

struct Picture : SceneObject {
	PushConstantObject pco;

	void init(DescriptorSetLayout *global, BaseProject *bs, std::string modelString, std::string textureString, glm::mat4 initialPosition);
	void populateCommandBuffer(VkCommandBuffer commandBuffer, int currentImage, Pipeline pipeline);
	void handleClick();
	void cleanup();
};

void Picture::handleClick() {
	std::cout << "picture cliccato" << std::endl;
}

void Picture::cleanup(){
	DS.cleanup();
	texture.cleanup();
	mesh.cleanup();
}

void Picture::init(DescriptorSetLayout *DSL, BaseProject *bs, std::string modelString, std::string textureString, glm::mat4 position) {
	mesh.init(bs, modelString);
	texture.init(bs, textureString);
	DS.init(bs, DSL, {
		{0, UNIFORM, sizeof(UniformBufferObject), nullptr},
		{1, TEXTURE, 0, &texture}
	});
	pco.worldMat = position;
}

void Picture::populateCommandBuffer(VkCommandBuffer commandBuffer, int currentImage, Pipeline pipeline) {
	VkBuffer vertexBuffers[] = { mesh.vertexBuffer };
	VkDeviceSize offsets[] = { 0 };
	vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);
	vkCmdBindIndexBuffer(commandBuffer, mesh.indexBuffer, 0, VK_INDEX_TYPE_UINT32);
	vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, 
		pipeline.pipelineLayout, 1, 1, &DS.descriptorSets[currentImage], 
		0, nullptr);

	// push constant before drawing the picture
	vkCmdPushConstants(commandBuffer, pipeline.pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT,
		0, sizeof(PushConstantObject), &pco
	);

	// draw the picture
	vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(mesh.indices.size()), 1, 0, 0, 0);
}

// -------------------- end Picture --------------------

// -------------------- start Statue --------------------

struct Statue : SceneObject {
	PushConstantObject pco;

	void init(DescriptorSetLayout *DSL, BaseProject *bs, std::string modelString, std::string textureString, glm::mat4 position);
	void populateCommandBuffer(VkCommandBuffer commandBuffer, int currentImage, Pipeline pipeline);
	void handleClick();
	void cleanup();
};

void Statue::handleClick() {
	std::cout << "statua cliccato" << std::endl;
}

void Statue::cleanup() {
	DS.cleanup();
	texture.cleanup();
	mesh.cleanup();
}

void Statue::init(DescriptorSetLayout *DSL, BaseProject *bs, std::string modelString, std::string textureString, glm::mat4 position) {
	mesh.init(bs, modelString);
	texture.init(bs, textureString);
	DS.init(bs, DSL, {
		{0, UNIFORM, sizeof(UniformBufferObject), nullptr},
		{1, TEXTURE, 0, &texture}
		});
	pco.worldMat = position;
}

void Statue::populateCommandBuffer(VkCommandBuffer commandBuffer, int currentImage, Pipeline pipeline) {
	VkBuffer vertexBuffers[] = { mesh.vertexBuffer };
	VkDeviceSize offsets[] = { 0 };
	vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);
	vkCmdBindIndexBuffer(commandBuffer, mesh.indexBuffer, 0, VK_INDEX_TYPE_UINT32);
	vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
		pipeline.pipelineLayout, 1, 1, &DS.descriptorSets[currentImage],
		0, nullptr);

	// push constant before drawing the picture
	vkCmdPushConstants(commandBuffer, pipeline.pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT,
		0, sizeof(PushConstantObject), &pco
	);

	vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(mesh.indices.size()), 1, 0, 0, 0);
}

// -------------------- end Statue --------------------

void copyInMemory(SceneObject obj, int currentImage, UniformBufferObject ubo, void* data, VkDevice device) {
	vkMapMemory(device, obj.DS.uniformBuffersMemory[0][currentImage], 0,
		sizeof(ubo), 0, &data);
	memcpy(data, &ubo, sizeof(ubo));
	vkUnmapMemory(device, obj.DS.uniformBuffersMemory[0][currentImage]);
};

// -------------------- start Skybox --------------------

struct Skybox {
	Model model;
	Texture texture;
	Pipeline pipeline;

	DescriptorSet DS;
	DescriptorSetLayout DSL;
	UniformBufferObject ubo;

	//BaseProject *baseProject;

	void init(BaseProject *bp, DescriptorSetLayout *global);
	void populateCommandBuffer(VkCommandBuffer commandBuffer, int currentImage, DescriptorSet& global);
	void updateVkMemory(VkDevice device, uint32_t currentImage, void *data);
	void cleanup();
};

void Skybox::cleanup() {
	DS.cleanup();
	model.cleanup();
	texture.cleanup();
	pipeline.cleanup();
	DSL.cleanup();
}

void Skybox::updateVkMemory(VkDevice device, uint32_t currentImage, void *data) {
	vkMapMemory(device, DS.uniformBuffersMemory[0][currentImage], 0, sizeof(ubo), 0, &data);
	memcpy(data, &ubo, sizeof(ubo));
	vkUnmapMemory(device, DS.uniformBuffersMemory[0][currentImage]);
}

void Skybox::populateCommandBuffer(VkCommandBuffer commandBuffer, int currentImage, DescriptorSet& global) {
	vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.graphicsPipeline);
	VkBuffer vertexBuffersSkybox[] = { model.vertexBuffer };
	VkDeviceSize offsetsSkybox[] = { 0 };
	vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffersSkybox, offsetsSkybox);
	vkCmdBindIndexBuffer(commandBuffer, model.indexBuffer, 0, VK_INDEX_TYPE_UINT32);
	vkCmdBindDescriptorSets(commandBuffer,
		VK_PIPELINE_BIND_POINT_GRAPHICS,
		pipeline.pipelineLayout, 0, 1, &global.descriptorSets[currentImage],
		0, nullptr
	);
	vkCmdBindDescriptorSets(commandBuffer,
		VK_PIPELINE_BIND_POINT_GRAPHICS,
		pipeline.pipelineLayout, 1, 1, &DS.descriptorSets[currentImage],
		0, nullptr
	);
	vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(model.indices.size()), 1, 0, 0, 0);
}

void Skybox::init(BaseProject *bp, DescriptorSetLayout *global) {
	//baseProject = bp;

	DSL.init(bp, {
		{0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT},
		{1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT}
	});

	pipeline.init(bp, "shaders/skyboxVert.spv", "shaders/skyboxFrag.spv", { global, &DSL });
	model.init(bp, MODEL_PATH + "skybox_cube.obj");
	texture.init(bp, TEXTURE_PATH + "skybox.png");

	DS.init(bp, &DSL, {
		{0, UNIFORM, sizeof(UniformBufferObject), nullptr},
		{1, TEXTURE, 0, &texture}
		});

	ubo.model = glm::scale(glm::mat4(1.0f), glm::vec3(50, 50, 50));
}

// -------------------- end Skybox --------------------

class MyProject : public BaseProject {
	protected:
	// Here you list all the Vulkan objects you need:
	// Camera
		glm::vec3 CamAng = glm::vec3(0.0f);
		glm::vec3 CamPos = glm::vec3(-0.2f, 0.95f, 14.5f);

	Player player;

	// Descriptor Layouts [what will be passed to the shaders]
	DescriptorSetLayout DSL_global;
	DescriptorSetLayout DSL_museum; //Object descriptor
	DescriptorSetLayout DSL_skybox;

	DescriptorSetLayout DSL_pic;
	DescriptorSetLayout DSL_statue;

	// Pipelines
	Pipeline museumPipeline;
	Pipeline picturePipeline;
	Pipeline statuePipeline;

	// Models, textures and Descriptors (values assigned to the uniforms)
	Model M_Museum;
	Model M_museumCollisionMesh;
	Texture T_Museum;
	DescriptorSet DS_Museum; //Instance of DSL_museum
	UniformBufferObject ubo_museum{};

	DescriptorSet DS_global; // used for cam and light points


	Statue Venus_Milo;
	Statue David;
	Statue Discobolus;

	Picture Sunday;
	Picture StarringNight;
	Picture VanGogh;
	Picture Munch_Scream;
	Picture Guernica;
	Picture Boulevard_monmarte;
	Picture Volpedo_FourthEstate;
	Picture Persistenza;
	Picture Impression_Sunrise;
	Picture TheDance;
	Picture Manet_Dejeuner;
	Picture Girasoli;
	Picture La_Camera;
	Picture Composizione_VI;
	Picture Cavalli;
	Picture Dream;
	Picture Cigni;
	Picture Donna_Cappello;

	//Signals
	Picture Dalì1;
	Picture Dalì2;
	Picture Impressionism1;
	Picture Impressionism2;
	Picture VanGoghSign1;
	Picture VanGoghSign2;
	Picture Expressionism1;
	Picture Expressionism2;
	Picture GuernicaSign1;
	Picture GuernicaSign2;
	Picture Sculptures1;
	Picture Sculptures2;
	Picture Matisse1;
	Picture Matisse2;
	Picture Fourth1;
	Picture Fourth2;

	//Platform
	Model M_Platform;
	Texture T_Platform;
	DescriptorSet DS_Platform;

	Skybox skybox;
	
	// Here you set the main application parameters
	void setWindowParameters() {
		// window size, titile and initial background
		windowWidth = 1280;
		windowHeight = 720;
		windowTitle = "Museum Project";
		initialBackgroundColor = {0.0f, 0.0f, 0.0f, 1.0f};
		
		// Descriptor pool sizes  !!!!
		uniformBlocksInPool = 2;
		texturesInPool = 31;
		setsInPool = texturesInPool+10;
	}
	
	// Here you load and setup all your Vulkan objects
	void localInit() {

		// setting things for glfw
		double halfw = windowWidth * 1.0 / 2;
		double halfh = windowHeight * 1.0 / 2;
		glfwGetCursorPos(window, &halfw, &halfh);

		ubo_museum.model = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -1.5f, 0.0f)) *
			glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f))*
			glm::scale(glm::mat4(1.0f), glm::vec3(2.2f, 1.5f, 2.2f));

		// Descriptor Layouts [what will be passed to the shaders]
		DSL_museum.init(this, {
					// this array contains the binding:
					// first  element : the binding number
					// second element : the time of element (buffer or texture)
					// third  element : the pipeline stage where it will be used
					{0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT},
					{1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT}
				  });
		
		DSL_pic.init(this, {
					// this array contains the binding:
					// first  element : the binding number
					// second element : the time of element (buffer or texture)
					// third  element : the pipeline stage where it will be used
					{0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT},
					{1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT}
				  });
		
		DSL_statue.init(this, {
					// this array contains the binding:
					// first  element : the binding number
					// second element : the time of element (buffer or texture)
					// third  element : the pipeline stage where it will be used
					{0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT},
					{1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT}
				  });

		DSL_global.init(this, {
			{0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS},
			});

		// Pipelines [Shader couples]
		// The last array, is a vector of pointer to the layouts of the sets that will
		// be used in this pipeline. The first element will be set 0, and so on..
		picturePipeline.init(this, "shaders/picture_vert.spv", "shaders/picture_frag.spv", { &DSL_global, &DSL_pic });
		museumPipeline.init(this, "shaders/vert.spv", "shaders/frag.spv", {&DSL_global, &DSL_museum});
		statuePipeline.init(this, "shaders/statue_vert.spv", "shaders/statue_frag.spv", {&DSL_global, &DSL_statue});

		// Models, textures and Descriptors (values assigned to the uniforms)
		
		M_Museum.init(this, MODEL_PATH + "museumTri.obj");
		T_Museum.init(this, TEXTURE_PATH + "wall.jpg");
		DS_Museum.init(this, &DSL_museum, {
		// the second parameter, is a pointer to the Uniform Set Layout of this set
		// the last parameter is an array, with one element per binding of the set.
		// first  elmenet : the binding number
		// second element : UNIFORM or TEXTURE (an enum) depending on the type
		// third  element : only for UNIFORMs, the size of the corresponding C++ object
		// fourth element : only for TEXTUREs, the pointer to the corresponding texture object
			{0, UNIFORM, sizeof(UniformBufferObject), nullptr},
			{1, TEXTURE, 0, &T_Museum},
		});

	
		M_Platform.init(this, MODEL_PATH + "Floating_Platform.obj");
		T_Platform.init(this, TEXTURE_PATH + "Floating_Platform.png");
		DS_Platform.init(this, &DSL_museum, {
			// the second parameter, is a pointer to the Uniform Set Layout of this set
			// the last parameter is an array, with one element per binding of the set.
			// first  elmenet : the binding number
			// second element : UNIFORM or TEXTURE (an enum) depending on the type
			// third  element : only for UNIFORMs, the size of the corresponding C++ object
			// fourth element : only for TEXTUREs, the pointer to the corresponding texture object
				{0, UNIFORM, sizeof(UniformBufferObject), nullptr},
				{1, TEXTURE, 0, &T_Platform},
			});
		
		// reading the config json
		//std::ifstream i("config/config.json");
		//json avoidHardcode;
		//i >> avoidHardcode;

		//-----Pictures----//

		//Matisse
		glm::mat4 temp = glm::translate(glm::mat4(1.0f), glm::vec3(-1.9f, 0.9f, 12.5f))*
			glm::scale(glm::mat4(1.0f), glm::vec3(0.009f, 0.009f, 0.009f))*
			glm::scale(glm::mat4(1.0f), glm::vec3(1.0f, 0.658f, 1.0f));
		TheDance.init(&DSL_pic, this, MODEL_PATH + "pictures.obj", TEXTURE_PATH + "TheDance.png", temp);
		
		temp = glm::translate(glm::mat4(1.0f), glm::vec3(2.3f, 0.8f, 12.3f))*
			glm::rotate(glm::mat4(1.0f), glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f))*
			glm::scale(glm::mat4(1.0f), glm::vec3(0.003f, 0.003f, 0.003f))*
			glm::scale(glm::mat4(1.0f), glm::vec3(1.0f, 1.37f, 1.0f));
		Donna_Cappello.init(&DSL_pic, this, MODEL_PATH + "pictures.obj", TEXTURE_PATH + "Donna_Cappello.png", temp);
		
		//Van Gogh
		temp = glm::translate(glm::mat4(1.0f), glm::vec3(-1.9f, 0.90f, 4.2f))*
			glm::scale(glm::mat4(1.0f), glm::vec3(0.006f, 0.006f, 0.006f))*
			glm::scale(glm::mat4(1.0f), glm::vec3(1.0f, 0.770f, 1.0f));
		StarringNight.init(&DSL_pic, this, MODEL_PATH + "pictures.obj", TEXTURE_PATH + "starringNight.png", temp);

		temp = glm::translate(glm::mat4(1.0f), glm::vec3(-1.9f, 1.1f, 2.4f))*
			glm::scale(glm::mat4(1.0f), glm::vec3(0.002f, 0.002f, 0.002f))*
			glm::scale(glm::mat4(1.0f), glm::vec3(1.0f, 1.24f, 1.0f));
		VanGogh.init(&DSL_pic, this, MODEL_PATH + "pictures.obj", TEXTURE_PATH + "VanGogh.png", temp);
		
		temp = glm::translate(glm::mat4(1.0f), glm::vec3(2.3f, 1.0f, 3.5f))*
			glm::rotate(glm::mat4(1.0f), glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f))*
			glm::scale(glm::mat4(1.0f), glm::vec3(0.0032f, 0.0032f, 0.0032f))*
			glm::scale(glm::mat4(1.0f), glm::vec3(1.0f, 1.311f, 1.0f));
		Girasoli.init(&DSL_pic, this, MODEL_PATH + "pictures.obj", TEXTURE_PATH + "Girasoli.png", temp);

		temp = glm::translate(glm::mat4(1.0f), glm::vec3(2.3f, 1.0f, 5.5f))*
			glm::rotate(glm::mat4(1.0f), glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f))*
			glm::scale(glm::mat4(1.0f), glm::vec3(0.006f, 0.006f, 0.006f))*
			glm::scale(glm::mat4(1.0f), glm::vec3(1.0f, 0.793f, 1.0f));
		La_Camera.init(&DSL_pic, this, MODEL_PATH + "pictures.obj", TEXTURE_PATH + "La_Camera.png", temp);

		//Guernica
		temp = glm::translate(glm::mat4(1.0f), glm::vec3(-1.8f, 1.3f, -0.5f))*
			glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f))*
			glm::rotate(glm::mat4(1.0f), glm::radians(180.0f), glm::vec3(1.0f, 0.0f, 0.0f))*
			glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f))*
			glm::scale(glm::mat4(1.0f), glm::vec3(0.2f, 0.2f, 0.2f));
		Guernica.init(&DSL_pic, this, MODEL_PATH + "Guernica.obj", TEXTURE_PATH + "Guernica.png", temp);

		//Impressionism
		temp = glm::translate(glm::mat4(1.0f), glm::vec3(-1.9f, 0.9f, 6.65f))*
			glm::scale(glm::mat4(1.0f), glm::vec3(0.006f, 0.006f, 0.006f))*
			glm::scale(glm::mat4(1.0f), glm::vec3(1.0f, 0.776f, 1.0f));
		Boulevard_monmarte.init(&DSL_pic, this, MODEL_PATH + "pictures.obj", TEXTURE_PATH + "Boulevard_monmarte.png", temp);

		temp = glm::translate(glm::mat4(1.0f), glm::vec3(-1.9f, 0.9f, 8.9f))*
			glm::scale(glm::mat4(1.0f), glm::vec3(0.005f, 0.005f, 0.005f))*
			glm::scale(glm::mat4(1.0f), glm::vec3(1.0f, 0.776f, 1.0f));
		Impression_Sunrise.init(&DSL_pic, this, MODEL_PATH + "pictures.obj", TEXTURE_PATH + "Impression_Sunrise.png", temp);

		temp = glm::translate(glm::mat4(1.0f), glm::vec3(2.3f, 0.9f, 8.2f))*
			glm::rotate(glm::mat4(1.0f), glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f))*
			glm::scale(glm::mat4(1.0f), glm::vec3(0.007f, 0.007f, 0.007f))*
			glm::scale(glm::mat4(1.0f), glm::vec3(1.0f, 0.671f, 1.0f));
		Sunday.init(&DSL_pic, this, MODEL_PATH + "pictures.obj", TEXTURE_PATH + "a_sunday_afternoon.png", temp);

		temp = glm::translate(glm::mat4(1.0f), glm::vec3(2.3f, 0.9f, 10.2f))*
			glm::rotate(glm::mat4(1.0f), glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f))*
			glm::scale(glm::mat4(1.0f), glm::vec3(0.007f, 0.007f, 0.007f))*
			glm::scale(glm::mat4(1.0f), glm::vec3(1.0f, 0.777f, 1.0f));
		Manet_Dejeuner.init(&DSL_pic, this, MODEL_PATH + "pictures.obj", TEXTURE_PATH + "Manet_Dejeuner.jpg", temp);
		
		
		//Espressionismo
		temp = glm::translate(glm::mat4(1.0f), glm::vec3(6.8f, 0.85f, 3.2f))*
			glm::rotate(glm::mat4(1.0f), glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f))*
			glm::scale(glm::mat4(1.0f), glm::vec3(0.004f, 0.004f, 0.004f))*
			glm::scale(glm::mat4(1.0f), glm::vec3(1.0f, 1.27f, 1.0f));
		Munch_Scream.init(&DSL_pic, this, MODEL_PATH + "pictures.obj", TEXTURE_PATH + "Munch_Scream.png", temp);
		
		temp = glm::translate(glm::mat4(1.0f), glm::vec3(6.8f, 0.9f, 5.4f))*
			glm::rotate(glm::mat4(1.0f), glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f))*
			glm::scale(glm::mat4(1.0f), glm::vec3(0.007f, 0.007f, 0.007f))*
			glm::scale(glm::mat4(1.0f), glm::vec3(1.0f, 0.658f, 1.0f));
		Composizione_VI.init(&DSL_pic, this, MODEL_PATH + "pictures.obj", TEXTURE_PATH + "Composizione_VI.png", temp);
		
		temp = glm::translate(glm::mat4(1.0f), glm::vec3(2.6f, 0.90f, 3.4f))*
			glm::scale(glm::mat4(1.0f), glm::vec3(0.007f, 0.007f, 0.007f))*
			glm::scale(glm::mat4(1.0f), glm::vec3(1.0f, 0.587f, 1.0f));
		Cavalli.init(&DSL_pic, this, MODEL_PATH + "pictures.obj", TEXTURE_PATH + "Cavalli.png", temp);


		//Volpedo Fourth Estate
		/*
		temp = glm::translate(glm::mat4(1.0f), glm::vec3(6.8f, 1.2f, 13.0f))*
			glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f))*
			glm::rotate(glm::mat4(1.0f), glm::radians(180.0f), glm::vec3(1.0f, 0.0f, 0.0f))*
			glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f))*
			glm::scale(glm::mat4(1.0f), glm::vec3(0.15f, 0.15f, 0.15f));
		Volpedo_FourthEstate.init(&DSL_pic, this, MODEL_PATH + "Volpedo_FourthEstate.obj", TEXTURE_PATH + "Volpedo_FourthEstate.png", temp);*/
		
		temp = glm::translate(glm::mat4(1.0f), glm::vec3(6.8f, 0.7f, 14.6f))*
			glm::rotate(glm::mat4(1.0f), glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f))*
			glm::scale(glm::mat4(1.0f), glm::vec3(0.016f, 0.016f, 0.016f))*
			glm::scale(glm::mat4(1.0f), glm::vec3(1.0f, 0.540f, 1.0f));
		Volpedo_FourthEstate.init(&DSL_pic, this, MODEL_PATH + "pictures.obj", TEXTURE_PATH + "Volpedo_FourthEstate.png", temp);


		//Dalì
		temp = glm::translate(glm::mat4(1.0f), glm::vec3(6.8f, 1.0f, 7.8f))*
			glm::rotate(glm::mat4(1.0f), glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f))*
			glm::scale(glm::mat4(1.0f), glm::vec3(0.0057f, 0.0057f, 0.0057f))*
			glm::scale(glm::mat4(1.0f), glm::vec3(1.0f, 0.666f, 1.0f));
		Persistenza.init(&DSL_pic, this, MODEL_PATH + "pictures.obj", TEXTURE_PATH + "Persistenza.png", temp);
		
		temp = glm::translate(glm::mat4(1.0f), glm::vec3(6.8f, 0.9f, 9.9f))*
			glm::rotate(glm::mat4(1.0f), glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f))*
			glm::scale(glm::mat4(1.0f), glm::vec3(0.0032f, 0.0032f, 0.0032f))*
			glm::scale(glm::mat4(1.0f), glm::vec3(1.0f, 1.26, 1.0f));
		Dream.init(&DSL_pic, this, MODEL_PATH + "pictures.obj", TEXTURE_PATH + "Dream.png", temp);
		
		temp = glm::translate(glm::mat4(1.0f), glm::vec3(2.6f, 0.90f, 8.0f))*
			glm::scale(glm::mat4(1.0f), glm::vec3(0.007f, 0.007f, 0.007f))*
			glm::scale(glm::mat4(1.0f), glm::vec3(1.0f, 0.65f, 1.0f));
		Cigni.init(&DSL_pic, this, MODEL_PATH + "pictures.obj", TEXTURE_PATH + "Cigni.png", temp);

		//-----Statues-----//
		temp = glm::translate(glm::mat4(1.0f), glm::vec3(6.0f, 0.05f, -2.0f))*
			glm::rotate(glm::mat4(1.0f), glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f))*
			glm::rotate(glm::mat4(1.0f), glm::radians(-90.0f), glm::vec3(0.0f, 0.0f, 1.0f))*
			glm::scale(glm::mat4(1.0f), glm::vec3(0.007f, 0.007f, 0.007f));
		Venus_Milo.init(&DSL_statue, this, MODEL_PATH + "Venus_Milo.obj", TEXTURE_PATH + "Venus_Milo.jpg", temp);
		
		temp = glm::translate(glm::mat4(1.0f), glm::vec3(6.0f, 0.05f, -0.5f))*
			glm::rotate(glm::mat4(1.0f), glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f))*
			glm::rotate(glm::mat4(1.0f), glm::radians(-90.0f), glm::vec3(0.0f, 0.0f, 1.0f))*
			glm::scale(glm::mat4(1.0f), glm::vec3(0.003f, 0.003f, 0.003f));
		David.init(&DSL_statue, this, MODEL_PATH + "David.obj", TEXTURE_PATH + "David.jpg", temp);
		
		temp = glm::translate(glm::mat4(1.0f), glm::vec3(6.0f, 0.05f, 1.0f))*
			glm::rotate(glm::mat4(1.0f), glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f))*
			glm::rotate(glm::mat4(1.0f), glm::radians(-90.0f), glm::vec3(0.0f, 0.0f, 1.0f))*
			glm::scale(glm::mat4(1.0f), glm::vec3(0.007f, 0.007f, 0.007f));
		Discobolus.init(&DSL_statue, this, MODEL_PATH + "Discobolus.obj", TEXTURE_PATH + "Discobolus.jpg", temp);

		//Signals
		temp = glm::translate(glm::mat4(1.0f), glm::vec3(4.15f, 2.0f, 10.9f))*
			glm::rotate(glm::mat4(1.0f), glm::radians(-90.0f), glm::vec3(0.0f, 0.0f, 1.0f))*
			glm::rotate(glm::mat4(1.0f), glm::radians(-90.0f), glm::vec3(0.0f, 1.0f, 0.0f))*
			glm::rotate(glm::mat4(1.0f), glm::radians(-90.0f), glm::vec3(0.0f, 0.0f, 1.0f))*
			glm::scale(glm::mat4(1.0f), glm::vec3(0.08f, 0.08f, 0.08f));
		Dalì1.init(&DSL_pic, this, MODEL_PATH + "Sign.obj", TEXTURE_PATH + "Dalì.png", temp);
		
		temp = glm::translate(glm::mat4(1.0f), glm::vec3(4.15f, 2.0f, 6.2f))*
			glm::rotate(glm::mat4(1.0f), glm::radians(-90.0f), glm::vec3(0.0f, 0.0f, 1.0f))*
			glm::rotate(glm::mat4(1.0f), glm::radians(-90.0f), glm::vec3(0.0f, 1.0f, 0.0f))*
			glm::rotate(glm::mat4(1.0f), glm::radians(-90.0f), glm::vec3(0.0f, 0.0f, 1.0f))*
			glm::rotate(glm::mat4(1.0f), glm::radians(180.0f), glm::vec3(1.0f, 0.0f, 0.0f))*
			glm::rotate(glm::mat4(1.0f), glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f))*
			glm::scale(glm::mat4(1.0f), glm::vec3(0.08f, 0.08f, 0.08f));
		Dalì2.init(&DSL_pic, this, MODEL_PATH + "Sign.obj", TEXTURE_PATH + "Dalì.png", temp);
		
		temp = glm::translate(glm::mat4(1.0f), glm::vec3(-0.35f, 2.0f, 10.9f))*
			glm::rotate(glm::mat4(1.0f), glm::radians(-90.0f), glm::vec3(0.0f, 0.0f, 1.0f))*
			glm::rotate(glm::mat4(1.0f), glm::radians(-90.0f), glm::vec3(0.0f, 1.0f, 0.0f))*
			glm::rotate(glm::mat4(1.0f), glm::radians(-90.0f), glm::vec3(0.0f, 0.0f, 1.0f))*
			glm::scale(glm::mat4(1.0f), glm::vec3(0.08f, 0.08f, 0.08f));
		Impressionism1.init(&DSL_pic, this, MODEL_PATH + "Sign.obj", TEXTURE_PATH + "Impressionism.png", temp);
		
		temp = glm::translate(glm::mat4(1.0f), glm::vec3(-0.35f, 2.0f, 6.2f))*
			glm::rotate(glm::mat4(1.0f), glm::radians(-90.0f), glm::vec3(0.0f, 0.0f, 1.0f))*
			glm::rotate(glm::mat4(1.0f), glm::radians(-90.0f), glm::vec3(0.0f, 1.0f, 0.0f))*
			glm::rotate(glm::mat4(1.0f), glm::radians(-90.0f), glm::vec3(0.0f, 0.0f, 1.0f))*
			glm::rotate(glm::mat4(1.0f), glm::radians(180.0f), glm::vec3(1.0f, 0.0f, 0.0f))*
			glm::rotate(glm::mat4(1.0f), glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f))*
			glm::scale(glm::mat4(1.0f), glm::vec3(0.08f, 0.08f, 0.08f));
		Impressionism2.init(&DSL_pic, this, MODEL_PATH + "Sign.obj", TEXTURE_PATH + "Impressionism.png", temp);
		
		temp = glm::translate(glm::mat4(1.0f), glm::vec3(-0.35f, 2.0f, 6.4f))*
			glm::rotate(glm::mat4(1.0f), glm::radians(-90.0f), glm::vec3(0.0f, 0.0f, 1.0f))*
			glm::rotate(glm::mat4(1.0f), glm::radians(-90.0f), glm::vec3(0.0f, 1.0f, 0.0f))*
			glm::rotate(glm::mat4(1.0f), glm::radians(-90.0f), glm::vec3(0.0f, 0.0f, 1.0f))*
			glm::scale(glm::mat4(1.0f), glm::vec3(0.08f, 0.08f, 0.08f));
		VanGoghSign1.init(&DSL_pic, this, MODEL_PATH + "Sign.obj", TEXTURE_PATH + "VanGoghSign.png", temp);
		
		temp = glm::translate(glm::mat4(1.0f), glm::vec3(-0.35f, 2.0f, 1.7f))*
			glm::rotate(glm::mat4(1.0f), glm::radians(-90.0f), glm::vec3(0.0f, 0.0f, 1.0f))*
			glm::rotate(glm::mat4(1.0f), glm::radians(-90.0f), glm::vec3(0.0f, 1.0f, 0.0f))*
			glm::rotate(glm::mat4(1.0f), glm::radians(-90.0f), glm::vec3(0.0f, 0.0f, 1.0f))*
			glm::rotate(glm::mat4(1.0f), glm::radians(180.0f), glm::vec3(1.0f, 0.0f, 0.0f))*
			glm::rotate(glm::mat4(1.0f), glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f))*
			glm::scale(glm::mat4(1.0f), glm::vec3(0.08f, 0.08f, 0.08f));
		VanGoghSign2.init(&DSL_pic, this, MODEL_PATH + "Sign.obj", TEXTURE_PATH + "VanGoghSign.png", temp);
		
		temp = glm::translate(glm::mat4(1.0f), glm::vec3(4.15f, 2.0f, 6.4f))*
			glm::rotate(glm::mat4(1.0f), glm::radians(-90.0f), glm::vec3(0.0f, 0.0f, 1.0f))*
			glm::rotate(glm::mat4(1.0f), glm::radians(-90.0f), glm::vec3(0.0f, 1.0f, 0.0f))*
			glm::rotate(glm::mat4(1.0f), glm::radians(-90.0f), glm::vec3(0.0f, 0.0f, 1.0f))*
			glm::scale(glm::mat4(1.0f), glm::vec3(0.08f, 0.08f, 0.08f));
		Expressionism1.init(&DSL_pic, this, MODEL_PATH + "Sign.obj", TEXTURE_PATH + "Expressionism.png", temp);
		
		temp = glm::translate(glm::mat4(1.0f), glm::vec3(4.15f, 2.0f, 1.7f))*
			glm::rotate(glm::mat4(1.0f), glm::radians(-90.0f), glm::vec3(0.0f, 0.0f, 1.0f))*
			glm::rotate(glm::mat4(1.0f), glm::radians(-90.0f), glm::vec3(0.0f, 1.0f, 0.0f))*
			glm::rotate(glm::mat4(1.0f), glm::radians(-90.0f), glm::vec3(0.0f, 0.0f, 1.0f))*
			glm::rotate(glm::mat4(1.0f), glm::radians(180.0f), glm::vec3(1.0f, 0.0f, 0.0f))*
			glm::rotate(glm::mat4(1.0f), glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f))*
			glm::scale(glm::mat4(1.0f), glm::vec3(0.08f, 0.08f, 0.08f));
		Expressionism2.init(&DSL_pic, this, MODEL_PATH + "Sign.obj", TEXTURE_PATH + "Expressionism.png", temp);
		
		temp = glm::translate(glm::mat4(1.0f), glm::vec3(-0.35f, 2.0f, 1.85f))*
			glm::rotate(glm::mat4(1.0f), glm::radians(-90.0f), glm::vec3(0.0f, 0.0f, 1.0f))*
			glm::rotate(glm::mat4(1.0f), glm::radians(-90.0f), glm::vec3(0.0f, 1.0f, 0.0f))*
			glm::rotate(glm::mat4(1.0f), glm::radians(-90.0f), glm::vec3(0.0f, 0.0f, 1.0f))*
			glm::scale(glm::mat4(1.0f), glm::vec3(0.08f, 0.08f, 0.08f));
		GuernicaSign1.init(&DSL_pic, this, MODEL_PATH + "Sign.obj", TEXTURE_PATH + "GuernicaSign.png", temp);
		
		temp = glm::translate(glm::mat4(1.0f), glm::vec3(2.6, 2.0f, -1.0f))*
			glm::rotate(glm::mat4(1.0f), glm::radians(-90.0f), glm::vec3(0.0f, 0.0f, 1.0f))*
			glm::rotate(glm::mat4(1.0f), glm::radians(-90.0f), glm::vec3(0.0f, 1.0f, 0.0f))*
			glm::scale(glm::mat4(1.0f), glm::vec3(0.08f, 0.08f, 0.08f));
		GuernicaSign2.init(&DSL_pic, this, MODEL_PATH + "Sign.obj", TEXTURE_PATH + "GuernicaSign.png", temp);
		
		temp = glm::translate(glm::mat4(1.0f), glm::vec3(4.15f, 2.0f, 1.85f))*
			glm::rotate(glm::mat4(1.0f), glm::radians(-90.0f), glm::vec3(0.0f, 0.0f, 1.0f))*
			glm::rotate(glm::mat4(1.0f), glm::radians(-90.0f), glm::vec3(0.0f, 1.0f, 0.0f))*
			glm::rotate(glm::mat4(1.0f), glm::radians(-90.0f), glm::vec3(0.0f, 0.0f, 1.0f))*
			glm::scale(glm::mat4(1.0f), glm::vec3(0.08f, 0.08f, 0.08f));
		Sculptures1.init(&DSL_pic, this, MODEL_PATH + "Sign.obj", TEXTURE_PATH + "Sculptures.png", temp);
		
		temp = glm::translate(glm::mat4(1.0f), glm::vec3(2.38, 2.0f, -1.0f))*
			glm::rotate(glm::mat4(1.0f), glm::radians(-90.0f), glm::vec3(0.0f, 0.0f, 1.0f))*
			glm::rotate(glm::mat4(1.0f), glm::radians(-90.0f), glm::vec3(0.0f, 1.0f, 0.0f))*
			glm::rotate(glm::mat4(1.0f), glm::radians(180.0f), glm::vec3(1.0f, 0.0f, 0.0f))*
			glm::rotate(glm::mat4(1.0f), glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f))*
			glm::scale(glm::mat4(1.0f), glm::vec3(0.08f, 0.08f, 0.08f));
		Sculptures2.init(&DSL_pic, this, MODEL_PATH + "Sign.obj", TEXTURE_PATH + "Sculptures.png", temp);
		
		temp = glm::translate(glm::mat4(1.0f), glm::vec3(-0.35f, 2.0f, 10.7f))*
			glm::rotate(glm::mat4(1.0f), glm::radians(-90.0f), glm::vec3(0.0f, 0.0f, 1.0f))*
			glm::rotate(glm::mat4(1.0f), glm::radians(-90.0f), glm::vec3(0.0f, 1.0f, 0.0f))*
			glm::rotate(glm::mat4(1.0f), glm::radians(-90.0f), glm::vec3(0.0f, 0.0f, 1.0f))*
			glm::rotate(glm::mat4(1.0f), glm::radians(180.0f), glm::vec3(1.0f, 0.0f, 0.0f))*
			glm::rotate(glm::mat4(1.0f), glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f))*
			glm::scale(glm::mat4(1.0f), glm::vec3(0.08f, 0.08f, 0.08f));
		Matisse1.init(&DSL_pic, this, MODEL_PATH + "Sign.obj", TEXTURE_PATH + "Matisse.png", temp);
		
		temp = glm::translate(glm::mat4(1.0f), glm::vec3(2.6, 2.0f, 13.6f))*
			glm::rotate(glm::mat4(1.0f), glm::radians(-90.0f), glm::vec3(0.0f, 0.0f, 1.0f))*
			glm::rotate(glm::mat4(1.0f), glm::radians(-90.0f), glm::vec3(0.0f, 1.0f, 0.0f))*
			glm::scale(glm::mat4(1.0f), glm::vec3(0.08f, 0.08f, 0.08f));
		Matisse2.init(&DSL_pic, this, MODEL_PATH + "Sign.obj", TEXTURE_PATH + "Matisse.png", temp);
		
		temp = glm::translate(glm::mat4(1.0f), glm::vec3(4.15f, 2.0f, 10.65f))*
			glm::rotate(glm::mat4(1.0f), glm::radians(-90.0f), glm::vec3(0.0f, 0.0f, 1.0f))*
			glm::rotate(glm::mat4(1.0f), glm::radians(-90.0f), glm::vec3(0.0f, 1.0f, 0.0f))*
			glm::rotate(glm::mat4(1.0f), glm::radians(-90.0f), glm::vec3(0.0f, 0.0f, 1.0f))*
			glm::rotate(glm::mat4(1.0f), glm::radians(180.0f), glm::vec3(1.0f, 0.0f, 0.0f))*
			glm::rotate(glm::mat4(1.0f), glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f))*
			glm::scale(glm::mat4(1.0f), glm::vec3(0.08f, 0.08f, 0.08f));
		Fourth1.init(&DSL_pic, this, MODEL_PATH + "Sign.obj", TEXTURE_PATH + "TheFourthSign.png", temp);
		
		temp = glm::translate(glm::mat4(1.0f), glm::vec3(2.38, 2.0f, 13.6f))*
			glm::rotate(glm::mat4(1.0f), glm::radians(-90.0f), glm::vec3(0.0f, 0.0f, 1.0f))*
			glm::rotate(glm::mat4(1.0f), glm::radians(-90.0f), glm::vec3(0.0f, 1.0f, 0.0f))*
			glm::rotate(glm::mat4(1.0f), glm::radians(180.0f), glm::vec3(1.0f, 0.0f, 0.0f))*
			glm::rotate(glm::mat4(1.0f), glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f))*
			glm::scale(glm::mat4(1.0f), glm::vec3(0.08f, 0.08f, 0.08f));
		Fourth2.init(&DSL_pic, this, MODEL_PATH + "Sign.obj", TEXTURE_PATH + "TheFourthSign.png", temp);

		for (int i = 0; i < M_Museum.indices.size() - 1; i += 3) {
			player.addTriangle(Triangle{
					ubo_museum.model * glm::vec4(M_Museum.vertices[M_Museum.indices[i]].pos, 1.0f),
					ubo_museum.model * glm::vec4(M_Museum.vertices[M_Museum.indices[i + 1]].pos, 1.0f),
					ubo_museum.model * glm::vec4(M_Museum.vertices[M_Museum.indices[i + 2]].pos, 1.0f)
				});
		}
		

		//----Skybox---//
		skybox.init(this, &DSL_global);

		DS_global.init(this, &DSL_global, {
			{0, UNIFORM, sizeof(GlobalUniformBufferObject), nullptr}
		});

		// init the player with the right aspect ratio of the image
		player.init(swapChainExtent.width / (float)swapChainExtent.height, { -0.2f, 0.95f, 14.94f });
	}

	// Here you destroy all the objects you created!		
	void localCleanup() {
		DS_Museum.cleanup();
		T_Museum.cleanup();
		M_Museum.cleanup();

		DS_Platform.cleanup();
		T_Platform.cleanup();
		M_Platform.cleanup();

		Sunday.cleanup();
		StarringNight.cleanup();
		VanGogh.cleanup();
		Munch_Scream.cleanup();
		Guernica.cleanup();
		Boulevard_monmarte.cleanup();
		Volpedo_FourthEstate.cleanup();
		Persistenza.cleanup();
		Impression_Sunrise.cleanup();
		TheDance.cleanup();
		Manet_Dejeuner.cleanup();
		Girasoli.cleanup();
		La_Camera.cleanup();
		Composizione_VI.cleanup();
		Cavalli.cleanup();
		Dream.cleanup();
		Cigni.cleanup();
		Donna_Cappello.cleanup();

		Venus_Milo.cleanup();
		David.cleanup();
		Discobolus.cleanup();

		Dalì1.cleanup();
		Dalì2.cleanup();
		Impressionism1.cleanup();
		Impressionism2.cleanup();
		VanGoghSign1.cleanup();
		VanGoghSign2.cleanup();
		Expressionism1.cleanup();
		Expressionism2.cleanup();
		GuernicaSign1.cleanup();
		GuernicaSign2.cleanup();
		Sculptures1.cleanup();
		Sculptures2.cleanup();
		Matisse1.cleanup();
		Matisse2.cleanup();
		Fourth1.cleanup();
		Fourth2.cleanup();

		DS_global.cleanup();

		museumPipeline.cleanup();
		picturePipeline.cleanup();
		DSL_global.cleanup();
		DSL_museum.cleanup();
		DSL_pic.cleanup();
		skybox.cleanup();
	}
	
	// Here it is the creation of the command buffer:
	// You send to the GPU all the objects you want to draw,
	// with their buffers and textures
	void populateCommandBuffer(VkCommandBuffer commandBuffer, int currentImage) {
		
// ---------- museum command buffer ----------

		vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
				museumPipeline.graphicsPipeline);
	
		VkBuffer vertexBuffers[] = {M_Museum.vertexBuffer};
		// property .vertexBuffer of models, contains the VkBuffer handle to its vertex buffer
		VkDeviceSize offsets[] = {0};
		vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);
		// property .indexBuffer of models, contains the VkBuffer handle to its index buffer
		vkCmdBindIndexBuffer(commandBuffer, M_Museum.indexBuffer, 0, VK_INDEX_TYPE_UINT32);

		// property .pipelineLayout of a pipeline contains its layout.
		// property .descriptorSets of a descriptor set contains its elements.
		vkCmdBindDescriptorSets(commandBuffer,
			VK_PIPELINE_BIND_POINT_GRAPHICS,
			museumPipeline.pipelineLayout, 1, 1, &DS_Museum.descriptorSets[currentImage], //(?)
			0, nullptr);

		vkCmdBindDescriptorSets(commandBuffer,
			VK_PIPELINE_BIND_POINT_GRAPHICS,
			museumPipeline.pipelineLayout, 0, 1, &DS_global.descriptorSets[currentImage],
			0, nullptr);
						
		// property .indices.size() of models, contains the number of triangles * 3 of the mesh.
		vkCmdDrawIndexed(commandBuffer,
					static_cast<uint32_t>(M_Museum.indices.size()), 1, 0, 0, 0);

// --------------------------------------------

// ---------- picture command buffer ----------

		vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
			picturePipeline.graphicsPipeline);

		//Adding the global set to the second pipeline
		vkCmdBindDescriptorSets(commandBuffer,
			VK_PIPELINE_BIND_POINT_GRAPHICS,
			picturePipeline.pipelineLayout, 0, 1, &DS_global.descriptorSets[currentImage],
			0, nullptr);

		Sunday.populateCommandBuffer(commandBuffer, currentImage, picturePipeline);
		StarringNight.populateCommandBuffer(commandBuffer, currentImage, picturePipeline);
		VanGogh.populateCommandBuffer(commandBuffer, currentImage, picturePipeline);
		Munch_Scream.populateCommandBuffer(commandBuffer, currentImage, picturePipeline);
		Guernica.populateCommandBuffer(commandBuffer, currentImage, picturePipeline);
		Boulevard_monmarte.populateCommandBuffer(commandBuffer, currentImage, picturePipeline);
		Volpedo_FourthEstate.populateCommandBuffer(commandBuffer, currentImage, picturePipeline);
		Persistenza.populateCommandBuffer(commandBuffer, currentImage, picturePipeline);
		Impression_Sunrise.populateCommandBuffer(commandBuffer, currentImage, picturePipeline);
		TheDance.populateCommandBuffer(commandBuffer, currentImage, picturePipeline);
		Manet_Dejeuner.populateCommandBuffer(commandBuffer, currentImage, picturePipeline);
		Girasoli.populateCommandBuffer(commandBuffer, currentImage, picturePipeline);
		La_Camera.populateCommandBuffer(commandBuffer, currentImage, picturePipeline);
		Composizione_VI.populateCommandBuffer(commandBuffer, currentImage, picturePipeline);
		Cavalli.populateCommandBuffer(commandBuffer, currentImage, picturePipeline);
		Dream.populateCommandBuffer(commandBuffer, currentImage, picturePipeline);
		Cigni.populateCommandBuffer(commandBuffer, currentImage, picturePipeline);
		Donna_Cappello.populateCommandBuffer(commandBuffer, currentImage, picturePipeline);

		// ---------- statues command buffer ----------
		Venus_Milo.populateCommandBuffer(commandBuffer, currentImage, statuePipeline);
		David.populateCommandBuffer(commandBuffer, currentImage, statuePipeline);
		Discobolus.populateCommandBuffer(commandBuffer, currentImage, statuePipeline);


		Dalì1.populateCommandBuffer(commandBuffer, currentImage, picturePipeline);
		Dalì2.populateCommandBuffer(commandBuffer, currentImage, picturePipeline);
		Impressionism1.populateCommandBuffer(commandBuffer, currentImage, picturePipeline);
		Impressionism2.populateCommandBuffer(commandBuffer, currentImage, picturePipeline);
		VanGoghSign1.populateCommandBuffer(commandBuffer, currentImage, picturePipeline);
		VanGoghSign2.populateCommandBuffer(commandBuffer, currentImage, picturePipeline);
		Expressionism1.populateCommandBuffer(commandBuffer, currentImage, picturePipeline);
		Expressionism2.populateCommandBuffer(commandBuffer, currentImage, picturePipeline);
		GuernicaSign1.populateCommandBuffer(commandBuffer, currentImage, picturePipeline);
		GuernicaSign2.populateCommandBuffer(commandBuffer, currentImage, picturePipeline);
		Sculptures1.populateCommandBuffer(commandBuffer, currentImage, picturePipeline);
		Sculptures2.populateCommandBuffer(commandBuffer, currentImage, picturePipeline);
		Matisse1.populateCommandBuffer(commandBuffer, currentImage, picturePipeline);
		Matisse2.populateCommandBuffer(commandBuffer, currentImage, picturePipeline);
		Fourth1.populateCommandBuffer(commandBuffer, currentImage, picturePipeline);
		Fourth2.populateCommandBuffer(commandBuffer, currentImage, picturePipeline);


		// skybox
		skybox.populateCommandBuffer(commandBuffer, currentImage, DS_global);

		//------------Platform--------------
		VkBuffer vertexBuffersP[] = { M_Platform.vertexBuffer };
		VkDeviceSize offsetsP[] = { 0 };
		vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffersP, offsetsP);
		vkCmdBindIndexBuffer(commandBuffer, M_Platform.indexBuffer, 0, VK_INDEX_TYPE_UINT32);
		vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
			museumPipeline.pipelineLayout, 1, 1, &DS_Platform.descriptorSets[currentImage],
			0, nullptr);
		vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(M_Platform.indices.size()), 1, 0, 0, 0);
	}

	// Here is where you update the uniforms.
	// Very likely this will be where you will be writing the logic of your application.
	void updateUniformBuffer(uint32_t currentImage) {
		static auto startTime = std::chrono::high_resolution_clock::now();
		auto currentTime = std::chrono::high_resolution_clock::now();
		float time = std::chrono::duration<float, std::chrono::seconds::period>
			(currentTime - startTime).count();
		static float lastTime = 0.0f;
		float deltaT = time - lastTime;
		lastTime = time;

		float ROT_SPEED = glm::radians(60.0f); //rotation speed of the camera
		float MOVE_SPEED = 3.0f; //speed of camera movements
		float MOUSE_RES = 500.0f;

		static bool hideMouse = true, showMouse = false, isMouseHidden = false;
		double xpos, ypos;
		glfwGetCursorPos(window, &xpos, &ypos);
		static double old_xpos = xpos, old_ypos = ypos;
		double m_dx = old_xpos - xpos;
		double m_dy = old_ypos - ypos;
		old_xpos = xpos; old_ypos = ypos;

		// ------ camera movement ------

		glm::vec3 viewChange = glm::vec3(0.0f, 0.0f, 0.0f);
		glfwSetInputMode(window, GLFW_STICKY_MOUSE_BUTTONS, GLFW_TRUE);
		if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
			viewChange.y -= m_dx * ROT_SPEED / MOUSE_RES;
			viewChange.x -= m_dy * ROT_SPEED / MOUSE_RES;
		}

		if (glfwGetKey(window, GLFW_KEY_LEFT)) {
			viewChange.y = deltaT * ROT_SPEED;
		}
		if (glfwGetKey(window, GLFW_KEY_RIGHT)) {
			viewChange.y = -(deltaT * ROT_SPEED);
		}
		if (glfwGetKey(window, GLFW_KEY_UP)) {
			viewChange.x = deltaT * ROT_SPEED;
		}
		if (glfwGetKey(window, GLFW_KEY_DOWN)) {
			viewChange.x = -(deltaT * ROT_SPEED);
		}

		player.moveHead(viewChange);

		// ------ player movement ------

		if (glfwGetKey(window, GLFW_KEY_A)) {
			player.left(deltaT);
		}
		if (glfwGetKey(window, GLFW_KEY_D)) {
			player.right(deltaT);
		}
		if (glfwGetKey(window, GLFW_KEY_S)) {
			player.backward(deltaT);
		}
		if (glfwGetKey(window, GLFW_KEY_W)) {
			player.forward(deltaT);
		}

		// ------ copying data into buffers ------

		void* data;

		//Update the Camera
		GlobalUniformBufferObject gubo{};
		gubo.view = player.camera.getCameraMatrix();
		gubo.proj = player.camera.getProjectionMatrix();

		// gubo
		vkMapMemory(device, DS_global.uniformBuffersMemory[0][currentImage], 0, sizeof(gubo), 0, &data);
		memcpy(data, &gubo, sizeof(gubo));
		vkUnmapMemory(device, DS_global.uniformBuffersMemory[0][currentImage]);

		// museum ubo
		vkMapMemory(device, DS_Museum.uniformBuffersMemory[0][currentImage], 0, sizeof(ubo_museum), 0, &data);
		memcpy(data, &ubo_museum, sizeof(ubo_museum));
		vkUnmapMemory(device, DS_Museum.uniformBuffersMemory[0][currentImage]);

		//Platform ubo
		UniformBufferObject ubo_platform{};
		ubo_platform.model = glm::translate(glm::mat4(1.0f), glm::vec3(2.8f, -6.47f, -1.7f))*
			glm::scale(glm::mat4(1.0f), glm::vec3(0.28f, 0.25f, 0.28f));

		vkMapMemory(device, DS_Platform.uniformBuffersMemory[0][currentImage], 0, sizeof(ubo_platform), 0, &data);
		memcpy(data, &ubo_platform, sizeof(ubo_platform));
		vkUnmapMemory(device, DS_Platform.uniformBuffersMemory[0][currentImage]);

		// skybox -> ma se tanto è costante una volta che lo ho copiato non rimane li per sempre?
		skybox.updateVkMemory(device, currentImage, data);
	}
};

// This is the main: probably you do not need to touch this!
int main() {
	MyProject app;

    try {
        app.run();
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}