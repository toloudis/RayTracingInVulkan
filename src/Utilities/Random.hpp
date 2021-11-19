#pragma once
#include "Glm.hpp"
#include <random>

float frand();

glm::vec3 randomInBox(float x, float y, float z);

glm::vec3 randomInSphere(float r = 1.0f);
