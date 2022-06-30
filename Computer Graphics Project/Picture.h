#pragma once

#include "MyProject.cpp"

struct Picture {
	Model model;
	Texture texture;
	DescriptorSetLayout DSL;
	DescriptorSet DS;

	// xml representation from file

	BaseProject *bs;

	void initialize(BaseProject *bs, DescriptorSet *ds, DescriptorSetLayout *dsl, std::string *texture, std::string *model);
	void handleClick();
};

void Picture::handleClick() {
	std::cout << "vinz ha cercato di rubarmi" << std::endl;
	return;
}
