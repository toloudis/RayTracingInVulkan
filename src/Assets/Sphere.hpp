#pragma once

#include "Procedural.hpp"
#include "Utilities/Glm.hpp"

namespace Assets
{
	
	class Sphere final : public Procedural
	{
	public:

		Sphere(const glm::vec3& center, const float radius) :
			Center(center), Radius(radius)
		{
		}

		const glm::vec3 Center;
		const float Radius;

		size_t NumBoundingBoxes() const override { return 1; }
		std::pair<glm::vec3, glm::vec3> BoundingBox(size_t index = 0) const override
		{
			return std::make_pair(Center - Radius, Center + Radius);
		}

	};

	class SphereGroup final : public Procedural
	{
	public:
		SphereGroup(const std::vector<glm::vec3> centers, const std::vector<float> radii)
			: centers(centers), radii(radii)
		{

		}
		const 	std::vector<glm::vec3> centers;
		// one single radius? consider...
		const 	std::vector<float> radii;
		size_t NumBoundingBoxes() const override { return centers.size(); }

		std::pair<glm::vec3, glm::vec3> BoundingBox(size_t index = 0) const override
		{
			return std::make_pair(centers[index] - radii[index], centers[index] + radii[index]);
		}

	};

}