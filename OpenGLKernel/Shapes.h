#pragma once
#include "Common.h"
#include <glm/glm.hpp>

NAMESPACE_BEGIN(gl_kernel)

OPENGL_KERNEL_EXPORT void createAQuad(std::vector<glm::vec3>& voPositionSet, std::vector<glm::vec3>& voTextureCoordsSet);
OPENGL_KERNEL_EXPORT void createAQuad(std::vector<glm::vec3>& voPositionSet, std::vector<glm::vec3>& voTextureCoordsSet, std::vector<glm::ivec3>& voIndicesSet);
OPENGL_KERNEL_EXPORT void createACube(std::vector<glm::vec3>& voPositionSet, std::vector<glm::vec3>& voNormalSet, std::vector<glm::vec2>& voTextureCoordsSet);
OPENGL_KERNEL_EXPORT void createASphere(std::vector<glm::vec3>& voPositionSet, std::vector<glm::vec3>& voNormalSet, std::vector<glm::vec2>& voTextureCoordsSet, std::vector<unsigned int>& voIndicesSet);

NAMESPACE_END(gl_kernel)