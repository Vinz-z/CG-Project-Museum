// This has been adapted from the Vulkan tutorial

#include "MyProject.hpp"

const std::string MODEL_PATH = "models/";
const std::string TEXTURE_PATH = "textures/";

// The uniform buffer object used in this example
struct GlobalUniformBufferObject {
	alignas(16) glm::mat4 view;
	alignas(16) glm::mat4 proj;
};

struct UniformBufferObject {
	alignas(16) glm::mat4 model;
};

struct Picture {
	Model model;
	Texture texture;
	DescriptorSet descSet;

	UniformBufferObject ubo;

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
	ubo.model = position;
}

void Picture::populateCommandBuffer(VkCommandBuffer commandBuffer, int currentImage, Pipeline pipeline) {
	VkBuffer vertexBuffers[] = { model.vertexBuffer };
	VkDeviceSize offsets[] = { 0 };
	vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);
	vkCmdBindIndexBuffer(commandBuffer, model.indexBuffer, 0, VK_INDEX_TYPE_UINT32);
	vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, 
		pipeline.pipelineLayout, 1, 1, &descSet.descriptorSets[currentImage], 
		0, nullptr);
	vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(model.indices.size()), 1, 0, 0, 0);
}

void copyInMemory(Picture picture, int currentImage, UniformBufferObject ubo, void* data, VkDevice device) {
	vkMapMemory(device, picture.descSet.uniformBuffersMemory[0][currentImage], 0,
		sizeof(ubo), 0, &data);
	memcpy(data, &ubo, sizeof(ubo));
	vkUnmapMemory(device, picture.descSet.uniformBuffersMemory[0][currentImage]);
};

struct Statue {
	Model model;
	Texture texture;
	DescriptorSet descSet;

	UniformBufferObject ubo;

	void cleanup();
	void init(DescriptorSetLayout *DSL, BaseProject *bs, std::string modelString, std::string textureString, glm::mat4 position);
	void populateCommandBuffer(VkCommandBuffer commandBuffer, int currentImage, Pipeline pipeline);
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
	ubo.model = position;
}

void Statue::populateCommandBuffer(VkCommandBuffer commandBuffer, int currentImage, Pipeline pipeline) {
	VkBuffer vertexBuffers[] = { model.vertexBuffer };
	VkDeviceSize offsets[] = { 0 };
	vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);
	vkCmdBindIndexBuffer(commandBuffer, model.indexBuffer, 0, VK_INDEX_TYPE_UINT32);
	vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
		pipeline.pipelineLayout, 1, 1, &descSet.descriptorSets[currentImage],
		0, nullptr);
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

	// Models, textures and Descriptors (values assigned to the uniforms)
	Model M_Museum;
	Texture T_Museum;
	DescriptorSet DS_Museum; //Instance of DSL_museum

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
	Picture TheBathers;
	Picture Impression_Sunrise;
	Picture TheDance;
	Picture Manet_Dejeuner;
	Picture Girasoli;
	Picture La_Camera;
	Picture Composizione_VI;
	Picture Cavalli;


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
		texturesInPool = 20;
		setsInPool = texturesInPool+2;
	}
	
	// Here you load and setup all your Vulkan objects
	void localInit() {
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
		

		//-----Pictures----//
		glm::mat4 temp = glm::translate(glm::mat4(1.0f), glm::vec3(-1.9f, 0.9f, 12.6f))*
			glm::scale(glm::mat4(1.0f), glm::vec3(0.009f, 0.009f, 0.009f))*
			glm::scale(glm::mat4(1.0f), glm::vec3(1.0f, 0.658f, 1.0f));
		TheDance.init(&DSL_pic, this, MODEL_PATH + "pictures.obj", TEXTURE_PATH + "TheDance.png", temp);
		
		//Van Gogh
		temp = glm::translate(glm::mat4(1.0f), glm::vec3(-1.9f, 0.90f, 4.2f))*
			glm::scale(glm::mat4(1.0f), glm::vec3(0.006f, 0.006f, 0.006f))*
			glm::scale(glm::mat4(1.0f), glm::vec3(1.0f, 0.658f, 1.0f));
		StarringNight.init(&DSL_pic, this, MODEL_PATH + "pictures.obj", TEXTURE_PATH + "starringNight.png", temp);

		temp = glm::translate(glm::mat4(1.0f), glm::vec3(-1.9f, 1.1f, 2.4f))*
			glm::scale(glm::mat4(1.0f), glm::vec3(0.002f, 0.002f, 0.002f))*
			glm::scale(glm::mat4(1.0f), glm::vec3(1.0f, 1.24f, 1.0f));
		VanGogh.init(&DSL_pic, this, MODEL_PATH + "pictures.obj", TEXTURE_PATH + "VanGogh.png", temp);
		
		temp = glm::translate(glm::mat4(1.0f), glm::vec3(2.3f, 1.0f, 3.5f))*
			glm::rotate(glm::mat4(1.0f), glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f))*
			glm::scale(glm::mat4(1.0f), glm::vec3(0.0032f, 0.0032f, 0.0032f))*
			glm::scale(glm::mat4(1.0f), glm::vec3(1.0f, 1.24f, 1.0f));
		Girasoli.init(&DSL_pic, this, MODEL_PATH + "pictures.obj", TEXTURE_PATH + "Girasoli.png", temp);

		temp = glm::translate(glm::mat4(1.0f), glm::vec3(2.3f, 1.0f, 5.5f))*
			glm::rotate(glm::mat4(1.0f), glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f))*
			glm::scale(glm::mat4(1.0f), glm::vec3(0.006f, 0.006f, 0.006f))*
			glm::scale(glm::mat4(1.0f), glm::vec3(1.0f, 0.658f, 1.0f));
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
			glm::scale(glm::mat4(1.0f), glm::vec3(1.0f, 0.658f, 1.0f));
		Boulevard_monmarte.init(&DSL_pic, this, MODEL_PATH + "pictures.obj", TEXTURE_PATH + "Boulevard_monmarte.png", temp);

		temp = glm::translate(glm::mat4(1.0f), glm::vec3(-1.9f, 0.9f, 8.9f))*
			glm::scale(glm::mat4(1.0f), glm::vec3(0.005f, 0.005f, 0.005f))*
			glm::scale(glm::mat4(1.0f), glm::vec3(1.0f, 0.658f, 1.0f));
		Impression_Sunrise.init(&DSL_pic, this, MODEL_PATH + "pictures.obj", TEXTURE_PATH + "Impression_Sunrise.png", temp);

		temp = glm::translate(glm::mat4(1.0f), glm::vec3(2.3f, 0.9f, 8.2f))*
			glm::rotate(glm::mat4(1.0f), glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f))*
			glm::scale(glm::mat4(1.0f), glm::vec3(0.007f, 0.007f, 0.007f))*
			glm::scale(glm::mat4(1.0f), glm::vec3(1.0f, 0.658f, 1.0f));
		Sunday.init(&DSL_pic, this, MODEL_PATH + "pictures.obj", TEXTURE_PATH + "a_sunday_afternoon.png", temp);

		temp = glm::translate(glm::mat4(1.0f), glm::vec3(2.3f, 0.9f, 10.2f))*
			glm::rotate(glm::mat4(1.0f), glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f))*
			glm::scale(glm::mat4(1.0f), glm::vec3(0.007f, 0.007f, 0.007f))*
			glm::scale(glm::mat4(1.0f), glm::vec3(1.0f, 0.658f, 1.0f));
		Manet_Dejeuner.init(&DSL_pic, this, MODEL_PATH + "pictures.obj", TEXTURE_PATH + "Manet_Dejeuner.jpg", temp);
		
		
		//Espressionismo
		temp = glm::translate(glm::mat4(1.0f), glm::vec3(6.8f, 0.85f, 3.2f))*
			glm::rotate(glm::mat4(1.0f), glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f))*
			glm::scale(glm::mat4(1.0f), glm::vec3(0.004f, 0.004f, 0.004f))*
			glm::scale(glm::mat4(1.0f), glm::vec3(1.0f, 1.24f, 1.0f));
		Munch_Scream.init(&DSL_pic, this, MODEL_PATH + "pictures.obj", TEXTURE_PATH + "Munch_Scream.png", temp);
		
		temp = glm::translate(glm::mat4(1.0f), glm::vec3(6.8f, 0.9f, 5.4f))*
			glm::rotate(glm::mat4(1.0f), glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f))*
			glm::scale(glm::mat4(1.0f), glm::vec3(0.007f, 0.007f, 0.007f))*
			glm::scale(glm::mat4(1.0f), glm::vec3(1.0f, 0.658f, 1.0f));
		Composizione_VI.init(&DSL_pic, this, MODEL_PATH + "pictures.obj", TEXTURE_PATH + "Composizione_VI.png", temp);
		
		temp = glm::translate(glm::mat4(1.0f), glm::vec3(2.6f, 0.90f, 3.4f))*
			glm::scale(glm::mat4(1.0f), glm::vec3(0.007f, 0.007f, 0.007f))*
			glm::scale(glm::mat4(1.0f), glm::vec3(1.0f, 0.658f, 1.0f));
		Cavalli.init(&DSL_pic, this, MODEL_PATH + "pictures.obj", TEXTURE_PATH + "Cavalli.png", temp);



		temp = glm::translate(glm::mat4(1.0f), glm::vec3(10.0f, 10.0f, 10.0f))*
			glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f))*
			glm::rotate(glm::mat4(1.0f), glm::radians(180.0f), glm::vec3(1.0f, 0.0f, 0.0f))*
			glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f))*
			glm::scale(glm::mat4(1.0f), glm::vec3(0.095f, 0.095f, 0.095f));
		Volpedo_FourthEstate.init(&DSL_pic, this, MODEL_PATH + "Volpedo_FourthEstate.obj", TEXTURE_PATH + "Volpedo_FourthEstate.png", temp);

		temp = glm::translate(glm::mat4(1.0f), glm::vec3(10.0f, 10.0f, 10.0f))*
			glm::scale(glm::mat4(1.0f), glm::vec3(0.005f, 0.005f, 0.005f))*
			glm::scale(glm::mat4(1.0f), glm::vec3(1.0f, 0.658f, 1.0f));
		TheBathers.init(&DSL_pic, this, MODEL_PATH + "pictures.obj", TEXTURE_PATH + "TheBathers.png", temp);

		//-----Statues-----//
		temp = glm::translate(glm::mat4(1.0f), glm::vec3(6.0f, 0.05f, 0.30f))*
			glm::rotate(glm::mat4(1.0f), glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f))*
			glm::scale(glm::mat4(1.0f), glm::vec3(0.007f, 0.007f, 0.007f));
		Venus_Milo.init(&DSL_statue, this, MODEL_PATH + "Venus_Milo.obj", TEXTURE_PATH + "Venus_Milo.jpg", temp);
		
		temp = glm::translate(glm::mat4(1.0f), glm::vec3(6.0f, 0.05f, 0.8f))*
			glm::rotate(glm::mat4(1.0f), glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f))*
			glm::scale(glm::mat4(1.0f), glm::vec3(0.003f, 0.003f, 0.003f));
		David.init(&DSL_statue, this, MODEL_PATH + "David.obj", TEXTURE_PATH + "David.jpg", temp);
		
		temp = glm::translate(glm::mat4(1.0f), glm::vec3(6.7f, 0.05f, 0.8f))*
			glm::rotate(glm::mat4(1.0f), glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f))*
			glm::scale(glm::mat4(1.0f), glm::vec3(0.007f, 0.007f, 0.007f));
		Discobolus.init(&DSL_statue, this, MODEL_PATH + "Discobolus.obj", TEXTURE_PATH + "Discobolus.jpg", temp);

		//----Skybox---//
		skybox.init(this, &DSL_global);

		DS_global.init(this, &DSL_global, {
			{0, UNIFORM, sizeof(GlobalUniformBufferObject), nullptr}
		});

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
		TheBathers.cleanup();
		Impression_Sunrise.cleanup();
		TheDance.cleanup();
		Manet_Dejeuner.cleanup();
		Girasoli.cleanup();
		La_Camera.cleanup();
		Composizione_VI.cleanup();
		Cavalli.cleanup();

		Venus_Milo.cleanup();
		David.cleanup();
		Discobolus.cleanup();

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
		TheBathers.populateCommandBuffer(commandBuffer, currentImage, picturePipeline);
		Impression_Sunrise.populateCommandBuffer(commandBuffer, currentImage, picturePipeline);
		TheDance.populateCommandBuffer(commandBuffer, currentImage, picturePipeline);
		Manet_Dejeuner.populateCommandBuffer(commandBuffer, currentImage, picturePipeline);
		Girasoli.populateCommandBuffer(commandBuffer, currentImage, picturePipeline);
		La_Camera.populateCommandBuffer(commandBuffer, currentImage, picturePipeline);
		Composizione_VI.populateCommandBuffer(commandBuffer, currentImage, picturePipeline);
		Cavalli.populateCommandBuffer(commandBuffer, currentImage, picturePipeline);

		// ---------- statues command buffer ----------
		Venus_Milo.populateCommandBuffer(commandBuffer, currentImage, statuePipeline);
		David.populateCommandBuffer(commandBuffer, currentImage, statuePipeline);
		Discobolus.populateCommandBuffer(commandBuffer, currentImage, statuePipeline);


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

		static double old_xpos = 0, old_ypos = 0;
		double xpos, ypos;
		glfwGetCursorPos(window, &xpos, &ypos);
		double m_dx = xpos - old_xpos;
		double m_dy = ypos - old_ypos;
		old_xpos = xpos; old_ypos = ypos;

		glm::vec3 oldCamPos = CamPos;

		glfwSetInputMode(window, GLFW_STICKY_MOUSE_BUTTONS, GLFW_TRUE);
		if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
			CamAng.y += m_dx * ROT_SPEED / MOUSE_RES;
			CamAng.x += m_dy * ROT_SPEED / MOUSE_RES;
		}

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
		
		//---------------Limits-----------//
		/*
		if (CamPos.x > 5.0f || CamPos.x < -5.0f || CamPos.z < -2.0f || CamPos.z > 10.0f) {
			CamPos = oldCamPos;
		}*/

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

		// meglio creare un ubo per ogni binding così non facciamo casino e li riconosciamo dai nomi
		UniformBufferObject ubo_museum{};
		ubo_museum.model = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -1.5f, 0.0f)) *
			glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f))*
			glm::scale(glm::mat4(1.0f),glm::vec3(2.2f,1.5f,2.2f));

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

	
		copyInMemory(Sunday, currentImage, Sunday.ubo, data, device);
		copyInMemory(StarringNight, currentImage, StarringNight.ubo, data, device);
		copyInMemory(VanGogh, currentImage, VanGogh.ubo, data, device);
		copyInMemory(Munch_Scream, currentImage, Munch_Scream.ubo, data, device);
		copyInMemory(Guernica, currentImage, Guernica.ubo, data, device);
		copyInMemory(Boulevard_monmarte, currentImage, Boulevard_monmarte.ubo, data, device);
		copyInMemory(Volpedo_FourthEstate, currentImage, Volpedo_FourthEstate.ubo, data, device);
		copyInMemory(TheBathers, currentImage, TheBathers.ubo, data, device);
		copyInMemory(Impression_Sunrise, currentImage, Impression_Sunrise.ubo, data, device);
		copyInMemory(TheDance, currentImage, TheDance.ubo, data, device);
		copyInMemory(Manet_Dejeuner, currentImage, Manet_Dejeuner.ubo, data, device);
		copyInMemory(Girasoli, currentImage, Girasoli.ubo, data, device);
		copyInMemory(La_Camera, currentImage, La_Camera.ubo, data, device);
		copyInMemory(Composizione_VI, currentImage, Composizione_VI.ubo, data, device);
		copyInMemory(Cavalli, currentImage, Cavalli.ubo, data, device);

		copyInMemory(Venus_Milo, currentImage, Venus_Milo.ubo, data, device);
		copyInMemory(David, currentImage, David.ubo, data, device);
		copyInMemory(Discobolus, currentImage, Discobolus.ubo, data, device);

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