#pragma once

#include "Model.hpp"
#include <string>
#include <vector>

namespace Assets
{
	struct Material;

	Model* LoadCIF(const std::string& filename, const Material& material);

	// TODO modify this to load multiple models and instances from within one cif file
	void LoadCIFAsScene(const std::string& filename, std::vector<Model>& models, std::vector<ModelInstance>& modelInstances);
}
