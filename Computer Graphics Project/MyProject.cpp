#include "MyProject.hpp";
#include <list>

const std::string MODEL_PATH = "models/";
const std::string TEXTURE_PATH = "textures/";

// The uniform buffer object used in this example
struct GlobalUniformBufferObject {
	alignas(16) glm::mat4 view; // alignas lo usa cpp per allineare i byte della matrice... 
	alignas(16) glm::mat4 proj; // la shader puo avere problemi con dei padding tra campi di una struttura
	alignas(16) glm::vec3 lightPos[8];
	alignas(16) glm::vec3 lightColor;
	alignas(16) glm::vec3 sunLightDir;
	alignas(16) glm::vec3 sunLightColor;
	alignas(16) glm::vec4 coneInOutDecayExp;
};

struct UniformBufferObject {
	alignas(16) glm::mat4 model;
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

struct Statue {
	Model model;
	Texture texture;
	DescriptorSet descSet;

	PushConstantObject pco;

	void cleanup();
	void init(DescriptorSetLayout *DSL, BaseProject *bs, std::string modelString, std::string textureString, glm::mat4 position);
	void populateCommandBuffer(VkCommandBuffer commandBuffer, int currentImage, Pipeline pipeline);
};

struct Picture {
	Model model;
	Texture texture;
	DescriptorSet descSet;

	PushConstantObject pco;

	void cleanup();
	void init(DescriptorSetLayout *DSL, BaseProject *bs, std::string modelString, std::string textureString, glm::mat4 position);
	void populateCommandBuffer(VkCommandBuffer commandBuffer, int currentImage, Pipeline pipeline);
};

struct Environment {
	Model model;
	Texture texture;
	DescriptorSet descSet;

	PushConstantObject pco;

	void cleanup();
	void init(DescriptorSetLayout *DSL, BaseProject *bs, std::string modelString, std::string textureString, glm::mat4 position);
	void populateCommandBuffer(VkCommandBuffer commandBuffer, int currentImage, Pipeline pipeline);
};

void Picture::cleanup(){
	descSet.cleanup();
	texture.cleanup();
	model.cleanup();
}

void Picture::init(DescriptorSetLayout *DSL, BaseProject *bs, std::string modelString, std::string textureString, glm::mat4 position) {
	model.init(bs, modelString);
	texture.init(bs, textureString);
	descSet.init(bs, DSL, {
		{0, UNIFORM, sizeof(UniformBufferObject), nullptr},
		{1, TEXTURE, 0, &texture}
	});
	pco.worldMat = position;
}

void Picture::populateCommandBuffer(VkCommandBuffer commandBuffer, int currentImage, Pipeline pipeline) {
	VkBuffer vertexBuffers[] = { model.vertexBuffer };
	VkDeviceSize offsets[] = { 0 };
	vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);
	vkCmdBindIndexBuffer(commandBuffer, model.indexBuffer, 0, VK_INDEX_TYPE_UINT32);
	vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, 
		pipeline.pipelineLayout, 1, 1, &descSet.descriptorSets[currentImage], 
		0, nullptr);

	// push constant before drawing the picture
	vkCmdPushConstants(commandBuffer, pipeline.pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT,
		0, sizeof(PushConstantObject), &pco
	);

	// draw the picture
	vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(model.indices.size()), 1, 0, 0, 0);
}

void Environment::cleanup() {
	descSet.cleanup();
	texture.cleanup();
	model.cleanup();
}

void Environment::init(DescriptorSetLayout *DSL, BaseProject *bs, std::string modelString, std::string textureString, glm::mat4 position) {
	model.init(bs, modelString);
	texture.init(bs, textureString);
	descSet.init(bs, DSL, {
		{0, UNIFORM, sizeof(UniformBufferObject), nullptr},
		{1, TEXTURE, 0, &texture}
		});
	pco.worldMat = position;
}

void Environment::populateCommandBuffer(VkCommandBuffer commandBuffer, int currentImage, Pipeline pipeline) {
	VkBuffer vertexBuffers[] = { model.vertexBuffer };
	VkDeviceSize offsets[] = { 0 };
	vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);
	vkCmdBindIndexBuffer(commandBuffer, model.indexBuffer, 0, VK_INDEX_TYPE_UINT32);
	vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
		pipeline.pipelineLayout, 1, 1, &descSet.descriptorSets[currentImage],
		0, nullptr);

	// push constant before drawing the picture
	vkCmdPushConstants(commandBuffer, pipeline.pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT,
		0, sizeof(PushConstantObject), &pco
	);

	// draw the picture
	vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(model.indices.size()), 1, 0, 0, 0);
}

void copyInMemory(Picture picture, int currentImage, UniformBufferObject ubo, void* data, VkDevice device) {
	vkMapMemory(device, picture.descSet.uniformBuffersMemory[0][currentImage], 0,
		sizeof(ubo), 0, &data);
	memcpy(data, &ubo, sizeof(ubo));
	vkUnmapMemory(device, picture.descSet.uniformBuffersMemory[0][currentImage]);
};

void Statue::cleanup() {
	descSet.cleanup();
	texture.cleanup();
	model.cleanup();
}

void Statue::init(DescriptorSetLayout *DSL, BaseProject *bs, std::string modelString, std::string textureString, glm::mat4 position) {
	model.init(bs, modelString);
	texture.init(bs, textureString);
	descSet.init(bs, DSL, {
		{0, UNIFORM, sizeof(UniformBufferObject), nullptr},
		{1, TEXTURE, 0, &texture}
		});
	pco.worldMat = position;
}

void Statue::populateCommandBuffer(VkCommandBuffer commandBuffer, int currentImage, Pipeline pipeline) {
	VkBuffer vertexBuffers[] = { model.vertexBuffer };
	VkDeviceSize offsets[] = { 0 };
	vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);
	vkCmdBindIndexBuffer(commandBuffer, model.indexBuffer, 0, VK_INDEX_TYPE_UINT32);
	vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
		pipeline.pipelineLayout, 1, 1, &descSet.descriptorSets[currentImage],
		0, nullptr);

	// push constant before drawing the picture
	vkCmdPushConstants(commandBuffer, pipeline.pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT,
		0, sizeof(PushConstantObject), &pco
	);

	vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(model.indices.size()), 1, 0, 0, 0);
}

void copyInMemory(Statue obj, int currentImage, UniformBufferObject ubo, void* data, VkDevice device) {
	vkMapMemory(device, obj.descSet.uniformBuffersMemory[0][currentImage], 0,
		sizeof(ubo), 0, &data);
	memcpy(data, &ubo, sizeof(ubo));
	vkUnmapMemory(device, obj.descSet.uniformBuffersMemory[0][currentImage]);
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

	// Pipelines
	Pipeline museumPipeline;

	Environment Museum;
	Environment Floor;
	Environment Island;

	DescriptorSet DS_global; // used for cam and light points

	//Statues
	Statue Venus_Milo;
	Statue David;
	Statue Discobolus;
	Statue Among_Us;

	//Pictures
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
		texturesInPool = 33;
		setsInPool = texturesInPool+10;
	}
	
	// Here you load and setup all your Vulkan objects
	void localInit() {

		// setting things for glfw
		double halfw = windowWidth * 1.0 / 2;
		double halfh = windowHeight * 1.0 / 2;
		glfwGetCursorPos(window, &halfw, &halfh);
		glm::mat4 temp = glm::mat4(1.0f);

		// Descriptor Layouts [what will be passed to the shaders]
		DSL_museum.init(this, {
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
		museumPipeline.init(this, "shaders/vert.spv", "shaders/frag.spv", {&DSL_global, &DSL_museum});

		temp = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -1.5f, 0.0f)) *
			glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f))*
			glm::scale(glm::mat4(1.0f), glm::vec3(2.2f, 1.5f, 2.2f));

		Museum.init(&DSL_museum, this, MODEL_PATH + "museumTri.obj", TEXTURE_PATH + "wall.jpg", temp);
		
		temp = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -1.5f, 0.0f)) *
			glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f))*
			glm::scale(glm::mat4(1.0f), glm::vec3(2.2f, 1.5f, 2.2f));

		Floor.init(&DSL_museum, this, MODEL_PATH + "Floor.obj", TEXTURE_PATH + "Floor.jpg", temp);
		
		temp = glm::translate(glm::mat4(1.0f), glm::vec3(2.8f, -6.47f, -1.7f))*
			glm::scale(glm::mat4(1.0f), glm::vec3(0.28f, 0.25f, 0.28f));

		Island.init(&DSL_museum, this, MODEL_PATH + "Floating_Platform.obj", TEXTURE_PATH + "Floating_Platform.png", temp);


		//-----Pictures----//

		//Matisse
		temp = glm::translate(glm::mat4(1.0f), glm::vec3(-1.9f, 0.9f, 12.5f))*
			glm::scale(glm::mat4(1.0f), glm::vec3(0.009f, 0.009f, 0.009f))*
			glm::scale(glm::mat4(1.0f), glm::vec3(1.0f, 0.658f, 1.0f));
		TheDance.init(&DSL_museum, this, MODEL_PATH + "pictures.obj", TEXTURE_PATH + "TheDance.png", temp);
		
		temp = glm::translate(glm::mat4(1.0f), glm::vec3(2.3f, 0.8f, 12.3f))*
			glm::rotate(glm::mat4(1.0f), glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f))*
			glm::scale(glm::mat4(1.0f), glm::vec3(0.003f, 0.003f, 0.003f))*
			glm::scale(glm::mat4(1.0f), glm::vec3(1.0f, 1.37f, 1.0f));
		Donna_Cappello.init(&DSL_museum, this, MODEL_PATH + "pictures.obj", TEXTURE_PATH + "Donna_Cappello.png", temp);
		
		//Van Gogh
		temp = glm::translate(glm::mat4(1.0f), glm::vec3(-1.9f, 0.90f, 4.2f))*
			glm::scale(glm::mat4(1.0f), glm::vec3(0.006f, 0.006f, 0.006f))*
			glm::scale(glm::mat4(1.0f), glm::vec3(1.0f, 0.770f, 1.0f));
		StarringNight.init(&DSL_museum, this, MODEL_PATH + "pictures.obj", TEXTURE_PATH + "starringNight.png", temp);

		temp = glm::translate(glm::mat4(1.0f), glm::vec3(-1.9f, 1.1f, 2.4f))*
			glm::scale(glm::mat4(1.0f), glm::vec3(0.002f, 0.002f, 0.002f))*
			glm::scale(glm::mat4(1.0f), glm::vec3(1.0f, 1.24f, 1.0f));
		VanGogh.init(&DSL_museum, this, MODEL_PATH + "pictures.obj", TEXTURE_PATH + "VanGogh.png", temp);
		
		temp = glm::translate(glm::mat4(1.0f), glm::vec3(2.3f, 1.0f, 3.5f))*
			glm::rotate(glm::mat4(1.0f), glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f))*
			glm::scale(glm::mat4(1.0f), glm::vec3(0.0032f, 0.0032f, 0.0032f))*
			glm::scale(glm::mat4(1.0f), glm::vec3(1.0f, 1.311f, 1.0f));
		Girasoli.init(&DSL_museum, this, MODEL_PATH + "pictures.obj", TEXTURE_PATH + "Girasoli.png", temp);

		temp = glm::translate(glm::mat4(1.0f), glm::vec3(2.3f, 1.0f, 5.5f))*
			glm::rotate(glm::mat4(1.0f), glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f))*
			glm::scale(glm::mat4(1.0f), glm::vec3(0.006f, 0.006f, 0.006f))*
			glm::scale(glm::mat4(1.0f), glm::vec3(1.0f, 0.793f, 1.0f));
		La_Camera.init(&DSL_museum, this, MODEL_PATH + "pictures.obj", TEXTURE_PATH + "La_Camera.png", temp);

		//Guernica
		temp = glm::translate(glm::mat4(1.0f), glm::vec3(-1.8f, 1.3f, -0.5f))*
			glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f))*
			glm::rotate(glm::mat4(1.0f), glm::radians(180.0f), glm::vec3(1.0f, 0.0f, 0.0f))*
			glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f))*
			glm::scale(glm::mat4(1.0f), glm::vec3(0.2f, 0.2f, 0.2f));
		Guernica.init(&DSL_museum, this, MODEL_PATH + "Guernica.obj", TEXTURE_PATH + "Guernica.png", temp);

		//Impressionism
		temp = glm::translate(glm::mat4(1.0f), glm::vec3(-1.9f, 0.9f, 6.65f))*
			glm::scale(glm::mat4(1.0f), glm::vec3(0.006f, 0.006f, 0.006f))*
			glm::scale(glm::mat4(1.0f), glm::vec3(1.0f, 0.776f, 1.0f));
		Boulevard_monmarte.init(&DSL_museum, this, MODEL_PATH + "pictures.obj", TEXTURE_PATH + "Boulevard_monmarte.png", temp);

		temp = glm::translate(glm::mat4(1.0f), glm::vec3(-1.9f, 0.9f, 8.9f))*
			glm::scale(glm::mat4(1.0f), glm::vec3(0.005f, 0.005f, 0.005f))*
			glm::scale(glm::mat4(1.0f), glm::vec3(1.0f, 0.776f, 1.0f));
		Impression_Sunrise.init(&DSL_museum, this, MODEL_PATH + "pictures.obj", TEXTURE_PATH + "Impression_Sunrise.png", temp);

		temp = glm::translate(glm::mat4(1.0f), glm::vec3(2.3f, 0.9f, 8.2f))*
			glm::rotate(glm::mat4(1.0f), glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f))*
			glm::scale(glm::mat4(1.0f), glm::vec3(0.007f, 0.007f, 0.007f))*
			glm::scale(glm::mat4(1.0f), glm::vec3(1.0f, 0.671f, 1.0f));
		Sunday.init(&DSL_museum, this, MODEL_PATH + "pictures.obj", TEXTURE_PATH + "a_sunday_afternoon.png", temp);

		temp = glm::translate(glm::mat4(1.0f), glm::vec3(2.3f, 0.9f, 10.2f))*
			glm::rotate(glm::mat4(1.0f), glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f))*
			glm::scale(glm::mat4(1.0f), glm::vec3(0.007f, 0.007f, 0.007f))*
			glm::scale(glm::mat4(1.0f), glm::vec3(1.0f, 0.777f, 1.0f));
		Manet_Dejeuner.init(&DSL_museum, this, MODEL_PATH + "pictures.obj", TEXTURE_PATH + "Manet_Dejeuner.png", temp);
		
		
		//Espressionismo
		temp = glm::translate(glm::mat4(1.0f), glm::vec3(6.8f, 0.85f, 3.2f))*
			glm::rotate(glm::mat4(1.0f), glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f))*
			glm::scale(glm::mat4(1.0f), glm::vec3(0.004f, 0.004f, 0.004f))*
			glm::scale(glm::mat4(1.0f), glm::vec3(1.0f, 1.27f, 1.0f));
		Munch_Scream.init(&DSL_museum, this, MODEL_PATH + "pictures.obj", TEXTURE_PATH + "Munch_Scream.png", temp);
		
		temp = glm::translate(glm::mat4(1.0f), glm::vec3(6.8f, 0.9f, 5.4f))*
			glm::rotate(glm::mat4(1.0f), glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f))*
			glm::scale(glm::mat4(1.0f), glm::vec3(0.007f, 0.007f, 0.007f))*
			glm::scale(glm::mat4(1.0f), glm::vec3(1.0f, 0.658f, 1.0f));
		Composizione_VI.init(&DSL_museum, this, MODEL_PATH + "pictures.obj", TEXTURE_PATH + "Composizione_VI.png", temp);
		
		temp = glm::translate(glm::mat4(1.0f), glm::vec3(2.6f, 0.90f, 3.4f))*
			glm::scale(glm::mat4(1.0f), glm::vec3(0.007f, 0.007f, 0.007f))*
			glm::scale(glm::mat4(1.0f), glm::vec3(1.0f, 0.587f, 1.0f));
		Cavalli.init(&DSL_museum, this, MODEL_PATH + "pictures.obj", TEXTURE_PATH + "Cavalli.png", temp);


		//Volpedo Fourth Estate		
		temp = glm::translate(glm::mat4(1.0f), glm::vec3(6.8f, 0.7f, 14.6f))*
			glm::rotate(glm::mat4(1.0f), glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f))*
			glm::scale(glm::mat4(1.0f), glm::vec3(0.016f, 0.016f, 0.016f))*
			glm::scale(glm::mat4(1.0f), glm::vec3(1.0f, 0.540f, 1.0f));
		Volpedo_FourthEstate.init(&DSL_museum, this, MODEL_PATH + "pictures.obj", TEXTURE_PATH + "Volpedo_FourthEstate.png", temp);


		//Dalì
		temp = glm::translate(glm::mat4(1.0f), glm::vec3(6.8f, 1.0f, 7.8f))*
			glm::rotate(glm::mat4(1.0f), glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f))*
			glm::scale(glm::mat4(1.0f), glm::vec3(0.0057f, 0.0057f, 0.0057f))*
			glm::scale(glm::mat4(1.0f), glm::vec3(1.0f, 0.666f, 1.0f));
		Persistenza.init(&DSL_museum, this, MODEL_PATH + "pictures.obj", TEXTURE_PATH + "Persistenza.png", temp);
		
		temp = glm::translate(glm::mat4(1.0f), glm::vec3(6.8f, 0.9f, 9.9f))*
			glm::rotate(glm::mat4(1.0f), glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f))*
			glm::scale(glm::mat4(1.0f), glm::vec3(0.0032f, 0.0032f, 0.0032f))*
			glm::scale(glm::mat4(1.0f), glm::vec3(1.0f, 1.26, 1.0f));
		Dream.init(&DSL_museum, this, MODEL_PATH + "pictures.obj", TEXTURE_PATH + "Dream.png", temp);
		
		temp = glm::translate(glm::mat4(1.0f), glm::vec3(2.6f, 0.90f, 8.0f))*
			glm::scale(glm::mat4(1.0f), glm::vec3(0.007f, 0.007f, 0.007f))*
			glm::scale(glm::mat4(1.0f), glm::vec3(1.0f, 0.65f, 1.0f));
		Cigni.init(&DSL_museum, this, MODEL_PATH + "pictures.obj", TEXTURE_PATH + "Cigni.png", temp);

		//-----Statues-----//
		temp = glm::translate(glm::mat4(1.0f), glm::vec3(6.0f, 0.05f, -2.0f))*
			glm::rotate(glm::mat4(1.0f), glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f))*
			glm::rotate(glm::mat4(1.0f), glm::radians(-90.0f), glm::vec3(0.0f, 0.0f, 1.0f))*
			glm::scale(glm::mat4(1.0f), glm::vec3(0.007f, 0.007f, 0.007f));
		Venus_Milo.init(&DSL_museum, this, MODEL_PATH + "Venus_Milo.obj", TEXTURE_PATH + "Venus_Milo.jpg", temp);
		
		temp = glm::translate(glm::mat4(1.0f), glm::vec3(6.0f, 0.05f, -0.5f))*
			glm::rotate(glm::mat4(1.0f), glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f))*
			glm::rotate(glm::mat4(1.0f), glm::radians(-90.0f), glm::vec3(0.0f, 0.0f, 1.0f))*
			glm::scale(glm::mat4(1.0f), glm::vec3(0.003f, 0.003f, 0.003f));
		David.init(&DSL_museum, this, MODEL_PATH + "David.obj", TEXTURE_PATH + "David.jpg", temp);
		
		temp = glm::translate(glm::mat4(1.0f), glm::vec3(6.0f, 0.05f, 1.0f))*
			glm::rotate(glm::mat4(1.0f), glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f))*
			glm::rotate(glm::mat4(1.0f), glm::radians(-90.0f), glm::vec3(0.0f, 0.0f, 1.0f))*
			glm::scale(glm::mat4(1.0f), glm::vec3(0.007f, 0.007f, 0.007f));
		Discobolus.init(&DSL_museum, this, MODEL_PATH + "Discobolus.obj", TEXTURE_PATH + "Discobolus.jpg", temp);
		
		temp = glm::translate(glm::mat4(1.0f), glm::vec3(3.0f, 0.05f, 1.0f))*
			glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f))*
			glm::scale(glm::mat4(1.0f), glm::vec3(1.0f, 1.0f, 1.0f));
		Among_Us.init(&DSL_museum, this, MODEL_PATH + "Among_Us.obj", TEXTURE_PATH + "Among_Us.png", temp);

		//Signals
		temp = glm::translate(glm::mat4(1.0f), glm::vec3(4.15f, 2.0f, 10.9f))*
			glm::rotate(glm::mat4(1.0f), glm::radians(-90.0f), glm::vec3(0.0f, 0.0f, 1.0f))*
			glm::rotate(glm::mat4(1.0f), glm::radians(-90.0f), glm::vec3(0.0f, 1.0f, 0.0f))*
			glm::rotate(glm::mat4(1.0f), glm::radians(-90.0f), glm::vec3(0.0f, 0.0f, 1.0f))*
			glm::scale(glm::mat4(1.0f), glm::vec3(0.08f, 0.08f, 0.08f));
		Dalì1.init(&DSL_museum, this, MODEL_PATH + "Sign.obj", TEXTURE_PATH + "Dalì.png", temp);
		
		temp = glm::translate(glm::mat4(1.0f), glm::vec3(4.15f, 2.0f, 6.2f))*
			glm::rotate(glm::mat4(1.0f), glm::radians(-90.0f), glm::vec3(0.0f, 0.0f, 1.0f))*
			glm::rotate(glm::mat4(1.0f), glm::radians(-90.0f), glm::vec3(0.0f, 1.0f, 0.0f))*
			glm::rotate(glm::mat4(1.0f), glm::radians(-90.0f), glm::vec3(0.0f, 0.0f, 1.0f))*
			glm::rotate(glm::mat4(1.0f), glm::radians(180.0f), glm::vec3(1.0f, 0.0f, 0.0f))*
			glm::rotate(glm::mat4(1.0f), glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f))*
			glm::scale(glm::mat4(1.0f), glm::vec3(0.08f, 0.08f, 0.08f));
		Dalì2.init(&DSL_museum, this, MODEL_PATH + "Sign.obj", TEXTURE_PATH + "Dalì.png", temp);
		
		temp = glm::translate(glm::mat4(1.0f), glm::vec3(-0.35f, 2.0f, 10.9f))*
			glm::rotate(glm::mat4(1.0f), glm::radians(-90.0f), glm::vec3(0.0f, 0.0f, 1.0f))*
			glm::rotate(glm::mat4(1.0f), glm::radians(-90.0f), glm::vec3(0.0f, 1.0f, 0.0f))*
			glm::rotate(glm::mat4(1.0f), glm::radians(-90.0f), glm::vec3(0.0f, 0.0f, 1.0f))*
			glm::scale(glm::mat4(1.0f), glm::vec3(0.08f, 0.08f, 0.08f));
		Impressionism1.init(&DSL_museum, this, MODEL_PATH + "Sign.obj", TEXTURE_PATH + "Impressionism.png", temp);
		
		temp = glm::translate(glm::mat4(1.0f), glm::vec3(-0.35f, 2.0f, 6.2f))*
			glm::rotate(glm::mat4(1.0f), glm::radians(-90.0f), glm::vec3(0.0f, 0.0f, 1.0f))*
			glm::rotate(glm::mat4(1.0f), glm::radians(-90.0f), glm::vec3(0.0f, 1.0f, 0.0f))*
			glm::rotate(glm::mat4(1.0f), glm::radians(-90.0f), glm::vec3(0.0f, 0.0f, 1.0f))*
			glm::rotate(glm::mat4(1.0f), glm::radians(180.0f), glm::vec3(1.0f, 0.0f, 0.0f))*
			glm::rotate(glm::mat4(1.0f), glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f))*
			glm::scale(glm::mat4(1.0f), glm::vec3(0.08f, 0.08f, 0.08f));
		Impressionism2.init(&DSL_museum, this, MODEL_PATH + "Sign.obj", TEXTURE_PATH + "Impressionism.png", temp);
		
		temp = glm::translate(glm::mat4(1.0f), glm::vec3(-0.35f, 2.0f, 6.4f))*
			glm::rotate(glm::mat4(1.0f), glm::radians(-90.0f), glm::vec3(0.0f, 0.0f, 1.0f))*
			glm::rotate(glm::mat4(1.0f), glm::radians(-90.0f), glm::vec3(0.0f, 1.0f, 0.0f))*
			glm::rotate(glm::mat4(1.0f), glm::radians(-90.0f), glm::vec3(0.0f, 0.0f, 1.0f))*
			glm::scale(glm::mat4(1.0f), glm::vec3(0.08f, 0.08f, 0.08f));
		VanGoghSign1.init(&DSL_museum, this, MODEL_PATH + "Sign.obj", TEXTURE_PATH + "VanGoghSign.png", temp);
		
		temp = glm::translate(glm::mat4(1.0f), glm::vec3(-0.35f, 2.0f, 1.7f))*
			glm::rotate(glm::mat4(1.0f), glm::radians(-90.0f), glm::vec3(0.0f, 0.0f, 1.0f))*
			glm::rotate(glm::mat4(1.0f), glm::radians(-90.0f), glm::vec3(0.0f, 1.0f, 0.0f))*
			glm::rotate(glm::mat4(1.0f), glm::radians(-90.0f), glm::vec3(0.0f, 0.0f, 1.0f))*
			glm::rotate(glm::mat4(1.0f), glm::radians(180.0f), glm::vec3(1.0f, 0.0f, 0.0f))*
			glm::rotate(glm::mat4(1.0f), glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f))*
			glm::scale(glm::mat4(1.0f), glm::vec3(0.08f, 0.08f, 0.08f));
		VanGoghSign2.init(&DSL_museum, this, MODEL_PATH + "Sign.obj", TEXTURE_PATH + "VanGoghSign.png", temp);
		
		temp = glm::translate(glm::mat4(1.0f), glm::vec3(4.15f, 2.0f, 6.4f))*
			glm::rotate(glm::mat4(1.0f), glm::radians(-90.0f), glm::vec3(0.0f, 0.0f, 1.0f))*
			glm::rotate(glm::mat4(1.0f), glm::radians(-90.0f), glm::vec3(0.0f, 1.0f, 0.0f))*
			glm::rotate(glm::mat4(1.0f), glm::radians(-90.0f), glm::vec3(0.0f, 0.0f, 1.0f))*
			glm::scale(glm::mat4(1.0f), glm::vec3(0.08f, 0.08f, 0.08f));
		Expressionism1.init(&DSL_museum, this, MODEL_PATH + "Sign.obj", TEXTURE_PATH + "Expressionism.png", temp);
		
		temp = glm::translate(glm::mat4(1.0f), glm::vec3(4.15f, 2.0f, 1.7f))*
			glm::rotate(glm::mat4(1.0f), glm::radians(-90.0f), glm::vec3(0.0f, 0.0f, 1.0f))*
			glm::rotate(glm::mat4(1.0f), glm::radians(-90.0f), glm::vec3(0.0f, 1.0f, 0.0f))*
			glm::rotate(glm::mat4(1.0f), glm::radians(-90.0f), glm::vec3(0.0f, 0.0f, 1.0f))*
			glm::rotate(glm::mat4(1.0f), glm::radians(180.0f), glm::vec3(1.0f, 0.0f, 0.0f))*
			glm::rotate(glm::mat4(1.0f), glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f))*
			glm::scale(glm::mat4(1.0f), glm::vec3(0.08f, 0.08f, 0.08f));
		Expressionism2.init(&DSL_museum, this, MODEL_PATH + "Sign.obj", TEXTURE_PATH + "Expressionism.png", temp);
		
		temp = glm::translate(glm::mat4(1.0f), glm::vec3(-0.35f, 2.0f, 1.85f))*
			glm::rotate(glm::mat4(1.0f), glm::radians(-90.0f), glm::vec3(0.0f, 0.0f, 1.0f))*
			glm::rotate(glm::mat4(1.0f), glm::radians(-90.0f), glm::vec3(0.0f, 1.0f, 0.0f))*
			glm::rotate(glm::mat4(1.0f), glm::radians(-90.0f), glm::vec3(0.0f, 0.0f, 1.0f))*
			glm::scale(glm::mat4(1.0f), glm::vec3(0.08f, 0.08f, 0.08f));
		GuernicaSign1.init(&DSL_museum, this, MODEL_PATH + "Sign.obj", TEXTURE_PATH + "GuernicaSign.png", temp);
		
		temp = glm::translate(glm::mat4(1.0f), glm::vec3(2.6, 2.0f, -1.0f))*
			glm::rotate(glm::mat4(1.0f), glm::radians(-90.0f), glm::vec3(0.0f, 0.0f, 1.0f))*
			glm::rotate(glm::mat4(1.0f), glm::radians(-90.0f), glm::vec3(0.0f, 1.0f, 0.0f))*
			glm::scale(glm::mat4(1.0f), glm::vec3(0.08f, 0.08f, 0.08f));
		GuernicaSign2.init(&DSL_museum, this, MODEL_PATH + "Sign.obj", TEXTURE_PATH + "GuernicaSign.png", temp);
		
		temp = glm::translate(glm::mat4(1.0f), glm::vec3(4.15f, 2.0f, 1.85f))*
			glm::rotate(glm::mat4(1.0f), glm::radians(-90.0f), glm::vec3(0.0f, 0.0f, 1.0f))*
			glm::rotate(glm::mat4(1.0f), glm::radians(-90.0f), glm::vec3(0.0f, 1.0f, 0.0f))*
			glm::rotate(glm::mat4(1.0f), glm::radians(-90.0f), glm::vec3(0.0f, 0.0f, 1.0f))*
			glm::scale(glm::mat4(1.0f), glm::vec3(0.08f, 0.08f, 0.08f));
		Sculptures1.init(&DSL_museum, this, MODEL_PATH + "Sign.obj", TEXTURE_PATH + "Sculptures.png", temp);
		
		temp = glm::translate(glm::mat4(1.0f), glm::vec3(2.38, 2.0f, -1.0f))*
			glm::rotate(glm::mat4(1.0f), glm::radians(-90.0f), glm::vec3(0.0f, 0.0f, 1.0f))*
			glm::rotate(glm::mat4(1.0f), glm::radians(-90.0f), glm::vec3(0.0f, 1.0f, 0.0f))*
			glm::rotate(glm::mat4(1.0f), glm::radians(180.0f), glm::vec3(1.0f, 0.0f, 0.0f))*
			glm::rotate(glm::mat4(1.0f), glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f))*
			glm::scale(glm::mat4(1.0f), glm::vec3(0.08f, 0.08f, 0.08f));
		Sculptures2.init(&DSL_museum, this, MODEL_PATH + "Sign.obj", TEXTURE_PATH + "Sculptures.png", temp);
		
		temp = glm::translate(glm::mat4(1.0f), glm::vec3(-0.35f, 2.0f, 10.7f))*
			glm::rotate(glm::mat4(1.0f), glm::radians(-90.0f), glm::vec3(0.0f, 0.0f, 1.0f))*
			glm::rotate(glm::mat4(1.0f), glm::radians(-90.0f), glm::vec3(0.0f, 1.0f, 0.0f))*
			glm::rotate(glm::mat4(1.0f), glm::radians(-90.0f), glm::vec3(0.0f, 0.0f, 1.0f))*
			glm::rotate(glm::mat4(1.0f), glm::radians(180.0f), glm::vec3(1.0f, 0.0f, 0.0f))*
			glm::rotate(glm::mat4(1.0f), glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f))*
			glm::scale(glm::mat4(1.0f), glm::vec3(0.08f, 0.08f, 0.08f));
		Matisse1.init(&DSL_museum, this, MODEL_PATH + "Sign.obj", TEXTURE_PATH + "Matisse.png", temp);
		
		temp = glm::translate(glm::mat4(1.0f), glm::vec3(2.6, 2.0f, 13.6f))*
			glm::rotate(glm::mat4(1.0f), glm::radians(-90.0f), glm::vec3(0.0f, 0.0f, 1.0f))*
			glm::rotate(glm::mat4(1.0f), glm::radians(-90.0f), glm::vec3(0.0f, 1.0f, 0.0f))*
			glm::scale(glm::mat4(1.0f), glm::vec3(0.08f, 0.08f, 0.08f));
		Matisse2.init(&DSL_museum, this, MODEL_PATH + "Sign.obj", TEXTURE_PATH + "Matisse.png", temp);
		
		temp = glm::translate(glm::mat4(1.0f), glm::vec3(4.15f, 2.0f, 10.65f))*
			glm::rotate(glm::mat4(1.0f), glm::radians(-90.0f), glm::vec3(0.0f, 0.0f, 1.0f))*
			glm::rotate(glm::mat4(1.0f), glm::radians(-90.0f), glm::vec3(0.0f, 1.0f, 0.0f))*
			glm::rotate(glm::mat4(1.0f), glm::radians(-90.0f), glm::vec3(0.0f, 0.0f, 1.0f))*
			glm::rotate(glm::mat4(1.0f), glm::radians(180.0f), glm::vec3(1.0f, 0.0f, 0.0f))*
			glm::rotate(glm::mat4(1.0f), glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f))*
			glm::scale(glm::mat4(1.0f), glm::vec3(0.08f, 0.08f, 0.08f));
		Fourth1.init(&DSL_museum, this, MODEL_PATH + "Sign.obj", TEXTURE_PATH + "TheFourthSign.png", temp);
		
		temp = glm::translate(glm::mat4(1.0f), glm::vec3(2.38, 2.0f, 13.6f))*
			glm::rotate(glm::mat4(1.0f), glm::radians(-90.0f), glm::vec3(0.0f, 0.0f, 1.0f))*
			glm::rotate(glm::mat4(1.0f), glm::radians(-90.0f), glm::vec3(0.0f, 1.0f, 0.0f))*
			glm::rotate(glm::mat4(1.0f), glm::radians(180.0f), glm::vec3(1.0f, 0.0f, 0.0f))*
			glm::rotate(glm::mat4(1.0f), glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f))*
			glm::scale(glm::mat4(1.0f), glm::vec3(0.08f, 0.08f, 0.08f));
		Fourth2.init(&DSL_museum, this, MODEL_PATH + "Sign.obj", TEXTURE_PATH + "TheFourthSign.png", temp);

		for (int i = 0; i < Museum.model.indices.size() - 1; i += 3) {
			player.addTriangle(Triangle{
					Museum.pco.worldMat * glm::vec4(Museum.model.vertices[Museum.model.indices[i]].pos, 1.0f),
					Museum.pco.worldMat * glm::vec4(Museum.model.vertices[Museum.model.indices[i + 1]].pos, 1.0f),
					Museum.pco.worldMat * glm::vec4(Museum.model.vertices[Museum.model.indices[i + 2]].pos, 1.0f)
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
		Museum.cleanup();
		Floor.cleanup();
		Island.cleanup();

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
		Among_Us.cleanup();

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
		museumPipeline.cleanup();
		DSL_global.cleanup();
		DSL_museum.cleanup();
		skybox.cleanup();
	}
	
	// Here it is the creation of the command buffer:
	// You send to the GPU all the objects you want to draw,
	// with their buffers and textures
	void populateCommandBuffer(VkCommandBuffer commandBuffer, int currentImage) {
		
// ---------- Environment command buffer ----------
		
		vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
				museumPipeline.graphicsPipeline);

		vkCmdBindDescriptorSets(commandBuffer,
			VK_PIPELINE_BIND_POINT_GRAPHICS,
			museumPipeline.pipelineLayout, 0, 1, &DS_global.descriptorSets[currentImage],
			0, nullptr);

		Museum.populateCommandBuffer(commandBuffer, currentImage, museumPipeline);
		Floor.populateCommandBuffer(commandBuffer, currentImage, museumPipeline);
		Island.populateCommandBuffer(commandBuffer, currentImage, museumPipeline);


// --------------------------------------------

// ---------- picture command buffer ----------

		Sunday.populateCommandBuffer(commandBuffer, currentImage, museumPipeline);
		StarringNight.populateCommandBuffer(commandBuffer, currentImage, museumPipeline);
		VanGogh.populateCommandBuffer(commandBuffer, currentImage, museumPipeline);
		Munch_Scream.populateCommandBuffer(commandBuffer, currentImage, museumPipeline);
		Guernica.populateCommandBuffer(commandBuffer, currentImage, museumPipeline);
		Boulevard_monmarte.populateCommandBuffer(commandBuffer, currentImage, museumPipeline);
		Volpedo_FourthEstate.populateCommandBuffer(commandBuffer, currentImage, museumPipeline);
		Persistenza.populateCommandBuffer(commandBuffer, currentImage, museumPipeline);
		Impression_Sunrise.populateCommandBuffer(commandBuffer, currentImage, museumPipeline);
		TheDance.populateCommandBuffer(commandBuffer, currentImage, museumPipeline);
		Manet_Dejeuner.populateCommandBuffer(commandBuffer, currentImage, museumPipeline);
		Girasoli.populateCommandBuffer(commandBuffer, currentImage, museumPipeline);
		La_Camera.populateCommandBuffer(commandBuffer, currentImage, museumPipeline);
		Composizione_VI.populateCommandBuffer(commandBuffer, currentImage, museumPipeline);
		Cavalli.populateCommandBuffer(commandBuffer, currentImage, museumPipeline);
		Dream.populateCommandBuffer(commandBuffer, currentImage, museumPipeline);
		Cigni.populateCommandBuffer(commandBuffer, currentImage, museumPipeline);
		Donna_Cappello.populateCommandBuffer(commandBuffer, currentImage, museumPipeline);

		Dalì1.populateCommandBuffer(commandBuffer, currentImage, museumPipeline);
		Dalì2.populateCommandBuffer(commandBuffer, currentImage, museumPipeline);
		Impressionism1.populateCommandBuffer(commandBuffer, currentImage, museumPipeline);
		Impressionism2.populateCommandBuffer(commandBuffer, currentImage, museumPipeline);
		VanGoghSign1.populateCommandBuffer(commandBuffer, currentImage, museumPipeline);
		VanGoghSign2.populateCommandBuffer(commandBuffer, currentImage, museumPipeline);
		Expressionism1.populateCommandBuffer(commandBuffer, currentImage, museumPipeline);
		Expressionism2.populateCommandBuffer(commandBuffer, currentImage, museumPipeline);
		GuernicaSign1.populateCommandBuffer(commandBuffer, currentImage, museumPipeline);
		GuernicaSign2.populateCommandBuffer(commandBuffer, currentImage, museumPipeline);
		Sculptures1.populateCommandBuffer(commandBuffer, currentImage, museumPipeline);
		Sculptures2.populateCommandBuffer(commandBuffer, currentImage, museumPipeline);
		Matisse1.populateCommandBuffer(commandBuffer, currentImage, museumPipeline);
		Matisse2.populateCommandBuffer(commandBuffer, currentImage, museumPipeline);
		Fourth1.populateCommandBuffer(commandBuffer, currentImage, museumPipeline);
		Fourth2.populateCommandBuffer(commandBuffer, currentImage, museumPipeline);
		vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
			museumPipeline.graphicsPipeline);

		// ---------- statues command buffer ----------
		Venus_Milo.populateCommandBuffer(commandBuffer, currentImage, museumPipeline);
		David.populateCommandBuffer(commandBuffer, currentImage, museumPipeline);
		Discobolus.populateCommandBuffer(commandBuffer, currentImage, museumPipeline);
		Among_Us.populateCommandBuffer(commandBuffer, currentImage, museumPipeline);

		// skybox
		skybox.populateCommandBuffer(commandBuffer, currentImage, DS_global);
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
		gubo.view = CamMat;
		gubo.proj = Prj;
		gubo.lightPos[0] = glm::vec3(1.0f, 2.0f, 13.0f); //point lights
		gubo.lightPos[1] = glm::vec3(1.0f, 2.0f, 8.f);
		gubo.lightPos[2] = glm::vec3(1.0f, 2.0f, 3.7f);
		gubo.lightPos[3] = glm::vec3(1.0f, 2.0f, -1.0f);
		gubo.lightPos[4] = glm::vec3(5.0f, 2.05f, 13.0f);
		gubo.lightPos[5] = glm::vec3(5.0f, 2.0f, 8.4f);
		gubo.lightPos[6] = glm::vec3(5.0f, 2.0f, 3.7f);
		gubo.lightPos[7] = glm::vec3(5.0f, 2.0f, -1.0f);
		gubo.lightColor = glm::vec3(1.0f, 1.0f, 1.0f);
		gubo.sunLightDir = glm::vec3(cos(glm::radians(135.0f)), sin(glm::radians(135.0f)), 0.0f); //sun (direct) light
		gubo.sunLightColor = glm::vec3(0.99f,0.9f,0.44f);
		gubo.coneInOutDecayExp = glm::vec4(0.9f, 0.92f, 0.5f, 1.5f);

		// gubo
		vkMapMemory(device, DS_global.uniformBuffersMemory[0][currentImage], 0, sizeof(gubo), 0, &data);
		memcpy(data, &gubo, sizeof(gubo));
		vkUnmapMemory(device, DS_global.uniformBuffersMemory[0][currentImage]);

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