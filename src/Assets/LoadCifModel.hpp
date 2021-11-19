#pragma once

#include "Model.hpp"
#include <string>

namespace Assets
{
	struct Material;

	Model LoadCIF(const std::string& filename, const Material& material);
}
