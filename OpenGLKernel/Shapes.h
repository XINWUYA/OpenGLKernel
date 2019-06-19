#pragma once
#include "Common.h"
#include <glm/glm.hpp>

NAMESPACE_BEGIN(gl_kernel)

void createACube();//´ý²¹³ä
void createASphere(std::vector<glm::vec3>& voPositionSet, std::vector<glm::vec3>& voNormalSet, std::vector<glm::vec2>& voTextureCoordsSet, std::vector<unsigned int>& voIndicesSet);

NAMESPACE_END(gl_kernel)