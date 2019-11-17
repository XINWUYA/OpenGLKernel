#pragma once
#include <GLM/glm.hpp>
#include <GL/glew.h>
#include "Common.h"

NAMESPACE_BEGIN(gl_kernel)

struct OPENGL_KERNEL_EXPORT SMeshVertex
{
	glm::vec3 m_Position;
	glm::vec3 m_Normal;
	glm::vec2 m_TexCoords;
	glm::vec3 m_Tangent; 
	glm::vec3 m_Bitangent;

	SMeshVertex() {}
};

struct OPENGL_KERNEL_EXPORT SMeshTexture
{
	GLuint m_TextureID = (unsigned int)-1;
	std::string m_TextureTypeName;
	std::string m_TexturePath;

	SMeshTexture() {}
};

class OPENGL_KERNEL_EXPORT CGLMesh
{
public:
	CGLMesh() = default;
	CGLMesh(const std::vector<SMeshVertex>& vMeshVerticesSet, const std::vector<GLuint> vMeshIndicesSet, const std::vector<SMeshTexture>& vMeshTextureSet);
	~CGLMesh() = default;

	void draw(unsigned int vTextureUnit = 0) const;

private:
	void __init();

	std::vector<SMeshVertex> m_MeshVerticesSet;
	std::vector<SMeshTexture> m_MeshTextureSet;
	std::vector<GLuint> m_MeshIndicesSet;
	GLuint m_VertexArrayObject;
};

NAMESPACE_END(gl_kernel)