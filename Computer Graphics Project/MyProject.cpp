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

void copyPicInMemory(Picture picture, int currentImage, UniformBufferObject ubo, void* data, VkDevice device) {
	vkMapMemory(device, picture.descSet.uniformBuffersMemory[0][currentImage], 0,
		sizeof(ubo), 0, &data);
	memcpy(data, &ubo, sizeof(ubo));
	vkUnmapMemory(device, picture.descSet.uniformBuffersMemory[0][currentImage]);
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
		glm::vec3 CamPos = glm::vec3(-0.1f, 0.75f, 7.5f);

	// Descriptor Layouts [what will be passed to the shaders]
	DescriptorSetLayout DSL_global;
	DescriptorSetLayout DSL_museum; //Object descriptor
	DescriptorSetLayout DSL_skybox;

	DescriptorSetLayout DSL_pic;

	// Pipelines
	Pipeline museumPipeline;
	Pipeline picturePipeline;

	// Models, textures and Descriptors (values assigned to the uniforms)
	Model M_Museum;
	Texture T_Museum;
	DescriptorSet DS_Museum; //Instance of DSL_museum

	DescriptorSet DS_global; // used for cam and light points

	Picture Sunday;
	Picture StarringNight;
	Picture VanGogh;
	Picture Munch_Scream;
	Picture Guernica;
	Picture Statue;

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
		texturesInPool = 7;
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

		DSL_global.init(this, {
			{0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS},
			});

		// Pipelines [Shader couples]
		// The last array, is a vector of pointer to the layouts of the sets that will
		// be used in this pipeline. The first element will be set 0, and so on..
		picturePipeline.init(this, "shaders/picture_vert.spv", "shaders/picture_frag.spv", { &DSL_global, &DSL_pic });
		museumPipeline.init(this, "shaders/vert.spv", "shaders/frag.spv", {&DSL_global, &DSL_museum});

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

		glm::mat4 temp = glm::translate(glm::mat4(1.0f), glm::vec3(-0.86f, 0.90f, 6.1f))*
			glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f))*
			glm::rotate(glm::mat4(1.0f), glm::radians(180.0f), glm::vec3(1.0f, 0.0f, 0.0f))*
			glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f))*
			glm::scale(glm::mat4(1.0f), glm::vec3(0.06f, 0.06f, 0.06f));
		Sunday.init(&DSL_pic, this, MODEL_PATH + "a_sunday_afternoon.obj", TEXTURE_PATH + "a_sunday_afternoon.png", temp);

		temp = glm::translate(glm::mat4(1.0f), glm::vec3(-0.86f, 0.90f, 3.4f))*
			glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f))*
			glm::rotate(glm::mat4(1.0f), glm::radians(180.0f), glm::vec3(1.0f, 0.0f, 0.0f))*
			glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f))*
			glm::scale(glm::mat4(1.0f), glm::vec3(0.04f, 0.04f, 0.04f));
		StarringNight.init(&DSL_pic, this, MODEL_PATH + "starringNight.obj", TEXTURE_PATH + "starringNight.png", temp);

		temp = glm::translate(glm::mat4(1.0f), glm::vec3(-0.86f, 0.88f, 2.18f))*
			glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f))*
			glm::rotate(glm::mat4(1.0f), glm::radians(180.0f), glm::vec3(1.0f, 0.0f, 0.0f))*
			glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f))*
			glm::scale(glm::mat4(1.0f), glm::vec3(0.02f, 0.02f, 0.02f));
		VanGogh.init(&DSL_pic, this, MODEL_PATH + "VanGogh.obj", TEXTURE_PATH + "VanGogh.png", temp);

		temp = glm::translate(glm::mat4(1.0f), glm::vec3(-0.86f, 0.88f, 1.175f))*
			glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f))*
			glm::rotate(glm::mat4(1.0f), glm::radians(180.0f), glm::vec3(1.0f, 0.0f, 0.0f))*
			glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f))*
			glm::scale(glm::mat4(1.0f), glm::vec3(0.023f, 0.023f, 0.023f));
		Munch_Scream.init(&DSL_pic, this, MODEL_PATH + "Munch_Scream.obj", TEXTURE_PATH + "Munch_Scream.png", temp);

		temp = glm::translate(glm::mat4(1.0f), glm::vec3(-0.86f, 0.88f, -0.22f))*
			glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f))*
			glm::rotate(glm::mat4(1.0f), glm::radians(180.0f), glm::vec3(1.0f, 0.0f, 0.0f))*
			glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f))*
			glm::scale(glm::mat4(1.0f), glm::vec3(0.095f, 0.095f, 0.095f));
		Guernica.init(&DSL_pic, this, MODEL_PATH + "Guernica.obj", TEXTURE_PATH + "Guernica.png", temp);

		temp = glm::translate(glm::mat4(1.0f), glm::vec3(2.0f, 0.05f, -0.30f))*
			glm::rotate(glm::mat4(1.0f), glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f))*
			glm::scale(glm::mat4(1.0f), glm::vec3(0.005f, 0.005f, 0.005f));
		Statue.init(&DSL_pic, this, MODEL_PATH + "Statue.obj", TEXTURE_PATH + "Statue.jpg", temp);

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

		Sunday.cleanup();
		StarringNight.cleanup();
		VanGogh.cleanup();
		Munch_Scream.cleanup();
		Guernica.cleanup();
		Statue.cleanup();

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
		Statue.populateCommandBuffer(commandBuffer, currentImage, picturePipeline);

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
		float MOVE_SPEED = 1.5f; //speed of camera movements
		float MOUSE_RES = 500.0f;

		static double old_xpos = 0, old_ypos = 0;
		double xpos, ypos;
		glfwGetCursorPos(window, &xpos, &ypos);
		double m_dx = xpos - old_xpos;
		double m_dy = ypos - old_ypos;
		old_xpos = xpos; old_ypos = ypos;

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

		// meglio creare un ubo per ogni binding cos� non facciamo casino e li riconosciamo dai nomi
		UniformBufferObject ubo_museum{};
		ubo_museum.model = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -1.0f, 0.0f)) *
			glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));

		// museum ubo
		vkMapMemory(device, DS_Museum.uniformBuffersMemory[0][currentImage], 0, sizeof(ubo_museum), 0, &data);
		memcpy(data, &ubo_museum, sizeof(ubo_museum));
		vkUnmapMemory(device, DS_Museum.uniformBuffersMemory[0][currentImage]);


		// � rimasto dal merge ma non so cosa sia
		ubo_museum.model = glm::translate(glm::mat4(1.0f), glm::vec3(-0.86f, 0.90f, 6.0f))*
			glm::scale(glm::mat4(1.0f), glm::vec3(0.04f, 0.04f, 0.04f));


		copyPicInMemory(Sunday, currentImage, Sunday.ubo, data, device);
		copyPicInMemory(StarringNight, currentImage, StarringNight.ubo, data, device);
		copyPicInMemory(VanGogh, currentImage, VanGogh.ubo, data, device);
		copyPicInMemory(Munch_Scream, currentImage, Munch_Scream.ubo, data, device);
		copyPicInMemory(Guernica, currentImage, Guernica.ubo, data, device);
		copyPicInMemory(Statue, currentImage, Statue.ubo, data, device);

		// skybox -> ma se tanto � costante una volta che lo ho copiato non rimane li per sempre?
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