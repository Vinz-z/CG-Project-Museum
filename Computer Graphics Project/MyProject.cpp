#include "MyProject.hpp";
#include <list>
#include <json.hpp>


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
	alignas(16) glm::vec2 coneInOutDecayExp; //(g, beta)
};

struct UniformBufferObject {
	alignas(16) glm::mat4 model;
	alignas(16) glm::mat4 view;
	alignas(16) glm::mat4 proj;
};

struct Character {
	unsigned int TextureID;  // ID handle of the glyph texture
	glm::ivec2   Size;       // Size of glyph
	glm::ivec2   Bearing;    // Offset from baseline to left/top of glyph
	unsigned int Advance;    // Offset to advance to next glyph
};

std::map<char, Character> Characters;

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

struct Ray {
	glm::vec3 origin;
	glm::vec3 direction;
};

struct Triangle {
	glm::vec3 A;
	glm::vec3 B;
	glm::vec3 C;
	glm::vec3 AB;
	glm::vec3 AC;
	glm::vec3  norm;

	Triangle(glm::vec3 a, glm::vec3 b, glm::vec3 c) {
		this->A = a;
		this->B = b;
		this->C = c;
		this->AB = B - A;
		this->AC = C - A;
		norm = glm::cross(AB, AC);
	}

	std::optional<glm::vec3> lineIntersectInside(Ray ray) {
		float d = glm::dot(norm, A);
		float perp = glm::dot(ray.direction, norm);
		if (perp == 0) return {};
		float t = (d - glm::dot(norm, ray.origin)) / perp;

		glm::vec3 intersection = ray.origin + (ray.direction * t);

		Triangle a{ A, B, intersection };
		Triangle b{ B, C, intersection };
		Triangle c{ C, A, intersection };

		if (glm::dot(glm::normalize(a.norm), glm::normalize(b.norm)) == 1.0f &&
			glm::dot(glm::normalize(b.norm), glm::normalize(c.norm)) == 1.0f
			) {
			return intersection;
		}

		return {};
	}

	void print() {
		printVec("A", A);
		printVec("B", B);
		printVec("C", C);
		printVec("norm", norm);
	}
};

struct Clickable {
	virtual void handleClick() = 0;

	void addTriangle(Triangle t) {
		body.push_front(t);
	}

	bool isClicked(Ray ray) {
		for (Triangle& t : body) {
			if (t.lineIntersectInside(ray)) return true;
		}

		return false;

	}

private:
	std::list<Triangle> body;

};

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

	Ray getRay() {
		return { position, getViewDirection() };
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

struct Player {
	std::list<Triangle> boundaries;
	Camera camera;

	const float movementSpeed = 3.0f;

	void init(float aspectRatio, glm::vec3 initPos) {
		position = initPos;
		camera.init(glm::vec3(0.0f, 0.0f, 0.0f), position, 0.1f, 200.0f, 70.0f, aspectRatio);
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
		for (Triangle& t : boundaries) {
			auto intersec = t.lineIntersectInside(Ray{ position, dir });
			if (intersec && glm::length(*intersec - position) <= (glm::length(dir) + 0.1f)) { return; }
		}

		position += dir;
		camera.move(dir);
	}
};

// -------------------- end Player --------------------



struct Artwork : Clickable {
	std::string title;
	std::string textureName;
	std::string modelName;
	std::string description;
	std::vector<float> translate;
	std::vector<float> rotate;
	std::vector<float> scale;

	Model model;
	Texture texture;
	DescriptorSet descSet;

	PushConstantObject pco;

	void cleanup();
	void init(DescriptorSetLayout *DSL, BaseProject *bs);
	void populateCommandBuffer(VkCommandBuffer commandBuffer, int currentImage, Pipeline pipeline);
	void loadClickArea(std::string file);
	void handleClick();
};

void from_json(const nlohmann::json& j, Artwork& o) {
	j.at("title").get_to(o.title);
	j.at("src").get_to(o.textureName);
	j.at("model").get_to(o.modelName);
	j.at("description").get_to(o.description);
	j.at("translate").get_to(o.translate);
	j.at("scale").get_to(o.scale);
	j.at("rotate").get_to(o.rotate);
}


void Artwork::loadClickArea(std::string file) {
	tinyobj::attrib_t attrib;
	std::vector<tinyobj::shape_t> shapes;
	std::vector<tinyobj::material_t> materials;
	std::string warn, err;

	if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err,
		file.c_str())) {
		throw std::runtime_error(warn + err);
	}

	for (const auto& shape : shapes) {
		for (int i = 0; i < shape.mesh.indices.size(); i += 3) {
			addTriangle(Triangle{
					pco.worldMat * glm::vec4(
						attrib.vertices[3 * shape.mesh.indices[i].vertex_index + 0],
						attrib.vertices[3 * shape.mesh.indices[i].vertex_index + 1],
						attrib.vertices[3 * shape.mesh.indices[i].vertex_index + 2], 1.0f),
					pco.worldMat * glm::vec4(
						attrib.vertices[3 * shape.mesh.indices[i + 1].vertex_index + 0],
						attrib.vertices[3 * shape.mesh.indices[i + 1].vertex_index + 1],
						attrib.vertices[3 * shape.mesh.indices[i + 1].vertex_index + 2], 1.0f),
					pco.worldMat * glm::vec4(
						attrib.vertices[3 * shape.mesh.indices[i + 2].vertex_index + 0],
						attrib.vertices[3 * shape.mesh.indices[i + 2].vertex_index + 1],
						attrib.vertices[3 * shape.mesh.indices[i + 2].vertex_index + 2], 1.0f),
				});
		}
	}
}

void Artwork::handleClick() {
	std::cout << "----------- click ------------" << std::endl;
	std::cout << title << std::endl;
	std::cout << "------------------------------" << std::endl;
	std::cout << description << std::endl;
	std::cout << "-----------   x   ------------" << std::endl;
}

void Artwork::cleanup(){
	descSet.cleanup();
	texture.cleanup();
	model.cleanup();
}

void Artwork::init(DescriptorSetLayout *DSL, BaseProject *bs) {
	model.init(bs, MODEL_PATH + modelName);
	texture.init(bs, TEXTURE_PATH + textureName);
	descSet.init(bs, DSL, {
		{0, UNIFORM, sizeof(UniformBufferObject), nullptr},
		{1, TEXTURE, 0, &texture}
	});

	pco.worldMat = glm::translate(glm::mat4(1), { translate[0], translate[1], translate[2] }) *
		glm::rotate(glm::mat4(1), glm::radians(rotate[1]), glm::vec3(0, 1, 0)) *
		glm::rotate(glm::mat4(1), glm::radians(rotate[0]), glm::vec3(1, 0, 0)) *
		glm::rotate(glm::mat4(1), glm::radians(rotate[2]), glm::vec3(0, 0, 1)) *
		glm::scale(glm::mat4(1), { scale[0], scale[1], scale[2] });

	loadClickArea(MODEL_PATH + "planePicture.obj");
}

void Artwork::populateCommandBuffer(VkCommandBuffer commandBuffer, int currentImage, Pipeline pipeline) {
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

struct Sofa {
	std::string textureName;
	std::vector<float> translate;
	std::vector<float> rotate;
	std::vector<float> scale;

	Model model;
	Texture texture;
	DescriptorSet descSet;

	PushConstantObject pco;

	void cleanup();
	void init(DescriptorSetLayout *DSL, BaseProject *bs);
	void populateCommandBuffer(VkCommandBuffer commandBuffer, int currentImage, Pipeline pipeline);
};

void from_json(const nlohmann::json& j, Sofa& o) {
	j.at("src").get_to(o.textureName);
	j.at("translate").get_to(o.translate);
	j.at("scale").get_to(o.scale);
	j.at("rotate").get_to(o.rotate);
}

void Sofa::init(DescriptorSetLayout *DSL, BaseProject *bs) {
	model.init(bs, MODEL_PATH + "Ottoman.obj");
	texture.init(bs, TEXTURE_PATH + textureName);
	descSet.init(bs, DSL, {
		{0, UNIFORM, sizeof(UniformBufferObject), nullptr},
		{1, TEXTURE, 0, &texture}
		});

	pco.worldMat = glm::translate(glm::mat4(1), { translate[0], translate[1], translate[2] }) *
		glm::rotate(glm::mat4(1), glm::radians(rotate[1]), glm::vec3(0, 1, 0)) *
		glm::rotate(glm::mat4(1), glm::radians(rotate[0]), glm::vec3(1, 0, 0)) *
		glm::rotate(glm::mat4(1), glm::radians(rotate[2]), glm::vec3(0, 0, 1)) *
		glm::scale(glm::mat4(1), { scale[0], scale[1], scale[2] });
}

void Sofa::cleanup() {
	descSet.cleanup();
	texture.cleanup();
	model.cleanup();
}

void Sofa::populateCommandBuffer(VkCommandBuffer commandBuffer, int currentImage, Pipeline pipeline) {
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

struct Sign {
	std::string textureName;
	std::vector<float> translate;
	std::vector<float> rotate;
	std::vector<float> scale;

	Model model;
	Texture texture;
	DescriptorSet descSet;

	PushConstantObject pco;

	void cleanup();
	void init(DescriptorSetLayout *DSL, BaseProject *bs);
	void populateCommandBuffer(VkCommandBuffer commandBuffer, int currentImage, Pipeline pipeline);
};

void from_json(const nlohmann::json& j, Sign& o) {
	j.at("src").get_to(o.textureName);
	j.at("translate").get_to(o.translate);
	j.at("scale").get_to(o.scale);
	j.at("rotate").get_to(o.rotate);
}

void Sign::init(DescriptorSetLayout *DSL, BaseProject *bs) {
	model.init(bs, MODEL_PATH + "Sign.obj");
	texture.init(bs, TEXTURE_PATH + textureName);
	descSet.init(bs, DSL, {
		{0, UNIFORM, sizeof(UniformBufferObject), nullptr},
		{1, TEXTURE, 0, &texture}
		});

	pco.worldMat = glm::translate(glm::mat4(1), { translate[0], translate[1], translate[2] }) *
		glm::rotate(glm::mat4(1), glm::radians(rotate[1]), glm::vec3(0, 1, 0)) *
		glm::rotate(glm::mat4(1), glm::radians(rotate[0]), glm::vec3(1, 0, 0)) *
		glm::rotate(glm::mat4(1), glm::radians(rotate[2]), glm::vec3(0, 0, 1)) *
		glm::scale(glm::mat4(1), { scale[0], scale[1], scale[2] });
}

void Sign::cleanup() {
	descSet.cleanup();
	texture.cleanup();
	model.cleanup();
}

void Sign::populateCommandBuffer(VkCommandBuffer commandBuffer, int currentImage, Pipeline pipeline) {
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

struct Environment {
	Model model;
	Texture texture;
	DescriptorSet descSet;

	PushConstantObject pco;

	void cleanup();
	void init(DescriptorSetLayout *DSL, BaseProject *bs, std::string modelString, std::string textureString, glm::mat4 position);
	void populateCommandBuffer(VkCommandBuffer commandBuffer, int currentImage, Pipeline pipeline);
};

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

void copyInMemory(Artwork picture, int currentImage, UniformBufferObject ubo, void* data, VkDevice device) {
	vkMapMemory(device, picture.descSet.uniformBuffersMemory[0][currentImage], 0,
		sizeof(ubo), 0, &data);
	memcpy(data, &ubo, sizeof(ubo));
	vkUnmapMemory(device, picture.descSet.uniformBuffersMemory[0][currentImage]);
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
	texture.init(bp, TEXTURE_PATH + "skybox toon.png");

	DS.init(bp, &DSL, {
		{0, UNIFORM, sizeof(UniformBufferObject), nullptr},
		{1, TEXTURE, 0, &texture}
		});

	ubo.model = glm::scale(glm::mat4(1.0f), glm::vec3(100, 100, 100));
}

// -------------------- end Skybox --------------------

struct Square {
	Model2D model;
	Texture texture;
	DescriptorSet descSet;
	std::vector<Vertex> ver;
	std::vector<uint32_t> index;

	void init(DescriptorSetLayout *DSL, BaseProject *bp, std::string textureString);
	void populateCommandBuffer(VkCommandBuffer commandBuffer, int currentImage, Pipeline pipeline);
	void cleanup();
};


void Square::init(DescriptorSetLayout *DSL, BaseProject *bp, std::string textureString) {
	int i = 0;
	ver.push_back(Vertex{ {-0.9f, -0.9f, 0.0f}, {0, 0, 1}, {1, 1} }); index.push_back(i++);
	ver.push_back(Vertex{ {0.9f, -0.9f, 0.0f}, {0, 0, 1}, {0, 1} }); index.push_back(i++);
	ver.push_back(Vertex{ {-0.9f, 0.9f, 0.0f}, {0, 0, 1}, {1, 0} }); index.push_back(i++);

	ver.push_back(Vertex{ {0.9f, -0.9f, 0.0f}, {0, 0, 1}, {0, 1} }); index.push_back(i++);
	ver.push_back(Vertex{ {0.9f, 0.9f, 0.0f}, {0, 0, 1}, {0,0} }); index.push_back(i++);
	ver.push_back(Vertex{ {-0.9f, 0.9f, 0.0f}, {0, 0, 1}, {1, 0} }); index.push_back(i++);

	model.init(bp, ver, index);
	texture.init(bp, TEXTURE_PATH + textureString);
	descSet.init(bp, DSL, {
		{0, UNIFORM, sizeof(UniformBufferObject), nullptr},
		{1, TEXTURE, 0, &texture}
		});
}

void Square::cleanup() {
	descSet.cleanup();
	texture.cleanup();
	model.cleanup();
}

void Square::populateCommandBuffer(VkCommandBuffer commandBuffer, int currentImage, Pipeline pipeline) {
	VkBuffer vertexBuffers[] = { model.vertexBuffer };
	VkDeviceSize offsets[] = { 0 };
	vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);
	vkCmdBindIndexBuffer(commandBuffer, model.indexBuffer, 0, VK_INDEX_TYPE_UINT32);
	vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
		pipeline.pipelineLayout, 1, 1, &descSet.descriptorSets[currentImage],
		0, nullptr);

	// draw the picture
	vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(model.indices.size()), 1, 0, 0, 0);
}





class MyProject : public BaseProject {
	Player player;
	std::list<Artwork> artworks;
	std::list<Sign> signs;
	std::list<Sofa> sofas;

	// time
	std::chrono::time_point<std::chrono::steady_clock> startTime;
	float lastTime = 0;

	// mouse
	double old_xpos = 0, old_ypos = 0;
	float MOUSE_RES = 600.0f;
	float ROT_SPEED = glm::radians(100.0f); //rotation speed of the camera

	// Descriptor Layouts [what will be passed to the shaders]
	DescriptorSetLayout DSL_global;
	DescriptorSetLayout DSL_museum; //Object descriptor
	DescriptorSetLayout DSL_skybox;

	DescriptorSetLayout DSL_text;

	// Pipelines
	Pipeline museumPipeline;
	Pipeline textPipeline;

	Environment Museum;
	Environment Floor;
	Environment Island;

	Square descBackground;


	DescriptorSet DS_global; // used for cam and light points

	Skybox skybox;
	
	// Here you set the main application parameters
	void setWindowParameters() {
		// window size, titile and initial background
		windowWidth = 1280;
		windowHeight = 720;
		windowTitle = "Museum Project";
		initialBackgroundColor = {0.0f, 0.0f, 0.0f, 1.0f};
		
		// Descriptor pool sizes  !!!! -> ????? sono cazzo giusti ?????
		/*
		uniformBlocksInPool = 2;
		texturesInPool = 34 + 1;
		setsInPool = texturesInPool+10+6;
		*/
		texturesInPool = 48 + 1;
		uniformBlocksInPool = texturesInPool + 1;
		setsInPool = texturesInPool + 2;
	}

	// Here you load and setup all your Vulkan objects
	void localInit() {

		
		
		// setting things for glfw
		//double halfw = windowWidth * 1.0 / 2;
		//double halfh = windowHeight * 1.0 / 2;
		//glfwGetCursorPos(window, &halfw, &halfh);
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

		DSL_text.init(this, {
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
		textPipeline.init(this, "shaders/textVert.spv", "shaders/textFrag.spv", {&DSL_global, &DSL_text});


		descBackground.init(&DSL_text, this, "descriptions/BlueHorses.png");
		

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

		std::ifstream f_artworks("config/artworks.json");
		nlohmann::json j_artworks;
		f_artworks >> j_artworks;

		for (auto& artwork : j_artworks["pictures"].get<std::vector<Artwork>>()) {
			artwork.init(&DSL_museum, this);
			artworks.push_front(artwork);
		}

		for (auto& artwork : j_artworks["statues"].get<std::vector<Artwork>>()) {
			artwork.init(&DSL_museum, this);
			artworks.push_front(artwork);
		}

		for (auto& sign : j_artworks["signs"].get<std::vector<Sign>>()) {
			sign.init(&DSL_museum, this);
			signs.push_front(sign);
		}

		for (auto& sofa : j_artworks["sofas"].get<std::vector<Sofa>>()) {
			sofa.init(&DSL_museum, this);
			sofas.push_front(sofa);
		}

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

		// time initialization
		startTime = std::chrono::high_resolution_clock::now();

		// mouse initialization
		glfwGetCursorPos(window, &old_xpos, &old_ypos);
	}

	// Here you destroy all the objects you created!		
	void localCleanup() {
		Museum.cleanup();
		Floor.cleanup();
		Island.cleanup();

		descBackground.cleanup();

		for (Artwork& pic : artworks) {
			pic.cleanup();
		}

		for (Sign& s : signs) {
			s.cleanup();
		}

		for (Sofa& s : sofas) {
			s.cleanup();
		}

		DS_global.cleanup();

		museumPipeline.cleanup();
		DSL_global.cleanup();
		DSL_museum.cleanup();
		skybox.cleanup();

		DSL_text.cleanup();
		textPipeline.cleanup();
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


// ---------- picture command buffer ----------

		for (Artwork& pic : artworks) {
			pic.populateCommandBuffer(commandBuffer, currentImage, museumPipeline);
		}

		for (Sign& s : signs) {
			s.populateCommandBuffer(commandBuffer, currentImage, museumPipeline);
		}

		for (Sofa& s : sofas) {
			s.populateCommandBuffer(commandBuffer, currentImage, museumPipeline);
		}

		// ---------- statues command buffer ----------

		// skybox
		skybox.populateCommandBuffer(commandBuffer, currentImage, DS_global);

		
		//Text
		vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
			textPipeline.graphicsPipeline);

		vkCmdBindDescriptorSets(commandBuffer,
			VK_PIPELINE_BIND_POINT_GRAPHICS,
			textPipeline.pipelineLayout, 0, 1, &DS_global.descriptorSets[currentImage],
			0, nullptr);

		descBackground.populateCommandBuffer(commandBuffer, currentImage, textPipeline);
		
	}

	// Here is where you update the uniforms.
	// Very likely this will be where you will be writing the logic of your application.
	void updateUniformBuffer(uint32_t currentImage) {
		auto currentTime = std::chrono::high_resolution_clock::now();
		float time = std::chrono::duration<float, std::chrono::seconds::period>
			(currentTime - startTime).count();
		float deltaT = time - lastTime;
		lastTime = time;

		double xpos, ypos;
		glfwGetCursorPos(window, &xpos, &ypos);
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

		if (glfwGetKey(window, GLFW_KEY_SPACE)) {
			for (Artwork& clb : artworks) {
				if (clb.isClicked(player.camera.getRay())) {
					clb.handleClick();
				}
			}
		}

		// ------ copying data into buffers ------

		void* data;

		

		//Update the Camera
		GlobalUniformBufferObject gubo{};
		gubo.proj = player.camera.getProjectionMatrix();
		gubo.view = player.camera.getCameraMatrix();
		gubo.lightPos[0] = glm::vec3(1.0f, 2.0f, 13.0f); //point lights
		gubo.lightPos[1] = glm::vec3(1.0f, 2.0f, 8.4f);
		gubo.lightPos[2] = glm::vec3(1.0f, 2.0f, 3.7f);
		gubo.lightPos[3] = glm::vec3(1.0f, 2.0f, -1.0f);
		gubo.lightPos[4] = glm::vec3(5.0f, 2.05f, 13.0f);
		gubo.lightPos[5] = glm::vec3(5.0f, 2.0f, 8.4f);
		gubo.lightPos[6] = glm::vec3(5.0f, 2.0f, 3.7f);
		gubo.lightPos[7] = glm::vec3(5.0f, 2.0f, -1.0f);
		gubo.lightColor = glm::vec3(1.0f, 0.96f, 0.934f);
		gubo.sunLightDir = glm::vec3(cos(glm::radians(time * 5)), sin(glm::radians(time * 5)), 0.0f); //sun (direct) light
		gubo.sunLightColor = glm::vec3(0.99f,0.9f,0.44f) * glm::clamp(sin(glm::radians(time * 5)), 0.0f, 1.0f);
		gubo.coneInOutDecayExp = glm::vec2(0.5f, 1.5f);

		// gubo
		vkMapMemory(device, DS_global.uniformBuffersMemory[0][currentImage], 0, sizeof(gubo), 0, &data);
		memcpy(data, &gubo, sizeof(gubo));
		vkUnmapMemory(device, DS_global.uniformBuffersMemory[0][currentImage]);
		

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