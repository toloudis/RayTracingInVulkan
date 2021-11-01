#pragma once

#include "Utilities/Glm.hpp"
#include <utility>

namespace Assets
{

	class Procedural
	{
	public:

		Procedural(const Procedural&) = delete;
		Procedural(Procedural&&) = delete;
		Procedural& operator = (const Procedural&) = delete;
		Procedural& operator = (Procedural&&) = delete;

		Procedural() = default;
		virtual ~Procedural() = default;;
		virtual std::pair<glm::vec3, glm::vec3> BoundingBox(size_t index = 0) const = 0;
		virtual size_t NumBoundingBoxes() const = 0;
	};
}
