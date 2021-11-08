#include "Random.hpp"

float frand() {
	return (float(rand()) / float(RAND_MAX));
}

glm::vec3 randomInBox(float x, float y, float z) {
	return glm::vec3(
		(float(rand()) / float(RAND_MAX)) * x - 0.5f * x,
		(float(rand()) / float(RAND_MAX)) * y - 0.5f * y,
		(float(rand()) / float(RAND_MAX)) * z - 0.5f * z
	);
}

glm::vec3 randomInSphere(float r) {
	float theta = (float(rand()) / float(RAND_MAX)) * 3.14159265f;
	float phi = (float(rand()) / float(RAND_MAX)) * 3.14159265f * 2.0f;
	float rr = r * (float(rand()) / float(RAND_MAX));
	return glm::vec3(
		rr * sin(theta) * sin(phi),
		rr * sin(theta) * cos(phi),
		rr * cos(theta)
	);
}
