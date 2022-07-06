// This has been adapted from the Vulkan tutorial

#include "MyProject.hpp"

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

struct Picture {
	Model model;
	Texture texture;
	DescriptorSet descSet;

	PushConstantObject pco;

	void cleanup();
	void init(DescriptorSetLayout *DSL, BaseProject *bs, std::string modelString, std::string textureString, glm::mat4 position);
	void populateCommandBuffer(VkCommandBuffer commandBuffer, int currentImage, Pipeline pipeline);
};

struct Statue {
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

void copyInMemory(Statue statue, int currentImage, UniformBufferObject ubo, void* data, VkDevice device) {
	vkMapMemory(device, statue.descSet.uniformBuffersMemory[0][currentImage], 0,
		sizeof(ubo), 0, &data);
	memcpy(data, &ubo, sizeof(ubo));
	vkUnmapMemory(device, statue.descSet.uniformBuffersMemory[0][currentImage]);
};

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



class MyProject : public BaseProject {

	protected:
	// Here you list all the Vulkan objects you need:
	// Camera
		glm::vec3 CamAng = glm::vec3(0.0f);
		glm::vec3 CamPos = glm::vec3(-0.2f, 0.95f, 14.5f);

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
		Manet_Dejeuner.init(&DSL_pic, this, MODEL_PATH + "pictures.obj", TEXTURE_PATH + "Manet_Dejeuner.png", temp);
		
		
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
		
		temp = glm::translate(glm::mat4(1.0f), glm::vec3(3.0f, 0.05f, 1.0f))*
			glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f))*
			glm::scale(glm::mat4(1.0f), glm::vec3(1.0f, 1.0f, 1.0f));
		Among_Us.init(&DSL_statue, this, MODEL_PATH + "Among_Us.obj", TEXTURE_PATH + "Among_Us.png", temp);

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


		//----Skybox---//
		skybox.init(this, &DSL_global);

		DS_global.init(this, &DSL_global, {
			{0, UNIFORM, sizeof(GlobalUniformBufferObject), nullptr}
		});

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
		Among_Us.populateCommandBuffer(commandBuffer, currentImage, statuePipeline);


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

		glm::vec3 oldCamPos = CamPos;

		glfwSetInputMode(window, GLFW_STICKY_MOUSE_BUTTONS, GLFW_TRUE);
		if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
			CamAng.y -= m_dx * ROT_SPEED / MOUSE_RES;
			CamAng.x -= m_dy * ROT_SPEED / MOUSE_RES;
		}
		/*
		if (hideMouse) {
			glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
			hideMouse = false; isMouseHidden = true;
		}
		if (showMouse) {
			glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
			showMouse = false; isMouseHidden = false;
		}
		CamAng.y += m_dx * ROT_SPEED / MOUSE_RES;
		CamAng.x += m_dy * ROT_SPEED / MOUSE_RES;
		*/

		if (glfwGetKey(window, GLFW_KEY_LEFT)) {
			CamAng.y += deltaT * ROT_SPEED;
		}
		if (glfwGetKey(window, GLFW_KEY_RIGHT)) {
			CamAng.y -= deltaT * ROT_SPEED;
		}
		if (glfwGetKey(window, GLFW_KEY_UP)) {
			CamAng.x += deltaT * ROT_SPEED;
		}
		if (glfwGetKey(window, GLFW_KEY_DOWN)) {
			CamAng.x -= deltaT * ROT_SPEED;
		}

		glm::mat3 CamDir = glm::mat3(glm::rotate(glm::mat4(1.0f), CamAng.y, glm::vec3(0.0f, 1.0f, 0.0f))) *
			glm::mat3(glm::rotate(glm::mat4(1.0f), CamAng.x, glm::vec3(1.0f, 0.0f, 0.0f))) *
			glm::mat3(glm::rotate(glm::mat4(1.0f), CamAng.z, glm::vec3(0.0f, 0.0f, 1.0f)));

		if (glfwGetKey(window, GLFW_KEY_A)) {
			CamPos -= MOVE_SPEED * glm::vec3(glm::rotate(glm::mat4(1.0f), CamAng.y,
				glm::vec3(0.0f, 1.0f, 0.0f)) * glm::vec4(1, 0, 0, 1)) * deltaT;
		}
		if (glfwGetKey(window, GLFW_KEY_D)) {
			CamPos += MOVE_SPEED * glm::vec3(glm::rotate(glm::mat4(1.0f), CamAng.y,
				glm::vec3(0.0f, 1.0f, 0.0f)) * glm::vec4(1, 0, 0, 1)) * deltaT;
		}
		if (glfwGetKey(window, GLFW_KEY_S)) {
			CamPos += MOVE_SPEED * glm::vec3(glm::rotate(glm::mat4(1.0f), CamAng.y,
				glm::vec3(0.0f, 1.0f, 0.0f)) * glm::vec4(0, 0, 1, 1)) * deltaT;
		}
		if (glfwGetKey(window, GLFW_KEY_W)) {
			CamPos -= MOVE_SPEED * glm::vec3(glm::rotate(glm::mat4(1.0f), CamAng.y,
				glm::vec3(0.0f, 1.0f, 0.0f)) * glm::vec4(0, 0, 1, 1)) * deltaT;
		}

		if (glfwGetKey(window, GLFW_KEY_F)) {
			CamPos -= MOVE_SPEED * glm::vec3(0, 1, 0) * deltaT;
		}
		if (glfwGetKey(window, GLFW_KEY_R)) {
			CamPos += MOVE_SPEED * glm::vec3(0, 1, 0) * deltaT;
		}

		/*
		if (glfwGetKey(window, GLFW_KEY_ESCAPE)) {
			if (isMouseHidden) showMouse = true;
			else hideMouse = true;
		}
		*/

		//---------------Limits-----------//
		/*
		if (CamPos.x > 5.0f || CamPos.x < -5.0f || CamPos.z < -2.0f || CamPos.z > 10.0f) {
			CamPos = oldCamPos;
		}*/

		/*
		printf("Dance x: %f\n", TheDance.pco.worldMat[3][0]);
		printf("CamPos: %f\n", (CamPos.x + 0.01f));
		printf("Position: %f\n", glm::abs(CamPos.x + 0.01f) - glm::abs(TheDance.pco.worldMat[3][0]));
		
		if (abs(CamPos.x) + 1.0f >= abs(TheDance.pco.worldMat[3][0])) {
			printf("Collision!\n");
			CamPos = oldCamPos;
		}
		else {
			printf("No Collision!\n");
		}
		*/


		glm::mat4 CamMat = glm::translate(glm::transpose(glm::mat4(CamDir)), -CamPos);

		glm::mat4 Prj = glm::perspective(glm::radians(60.0f),
			swapChainExtent.width / (float)swapChainExtent.height,
			0.1f, 100.0f);
		Prj[1][1] *= -1;

		void* data;

		//Update the Camera
		GlobalUniformBufferObject gubo{};
		gubo.view = CamMat;
		gubo.proj = Prj;

		// gubo
		vkMapMemory(device, DS_global.uniformBuffersMemory[0][currentImage], 0, sizeof(gubo), 0, &data);
		memcpy(data, &gubo, sizeof(gubo));
		vkUnmapMemory(device, DS_global.uniformBuffersMemory[0][currentImage]);

		// skybox -> ma se tanto è costante una volta che lo ho copiato non rimane li per sempre?
		skybox.updateVkMemory(device, currentImage, data);
	}

	void calculateIntersection() {

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