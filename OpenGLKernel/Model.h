#pragma once
#include <Assimp/Importer.hpp>
#include <Assimp/scene.h>
#include "Common.h"
#include "Mesh.h"

NAMESPACE_BEGIN(gl_kernel)

class CGLShader;
class CGLMesh;
class CGLModel
{
public:
	CGLModel() = default;
	CGLModel(const std::string& vModelName, bool vIsUseGammaCorrection = false);
	~CGLModel() = default;

	void draw(const CGLShader& vShader, unsigned int vTextureUnit = 0);

private:
	void __init(const std::string& vModelName);
	void __loadMaterialTextures(const aiMaterial* vMaterial, aiTextureType vTextureType, const std::string& vTextureUniformName);
	CGLMesh __processMesh(const aiMesh* vMesh, const aiScene* vScene);
	void __processNodeRecursively(const aiNode* vNode, const aiScene* vScene);
	void __processVertices(const aiMesh* vMesh, std::vector<SMeshVertex>& voMeshVerticesSet);
	void __processTextures(const aiMesh* vMesh, std::vector<SMeshTexture>& voMeshTexturesSet);
	void __processIndices(const aiMesh* vMesh, std::vector<GLuint>& voMeshIndicesSet);

	std::vector<CGLMesh> m_MeshSet;
	std::vector<SMeshTexture> m_MeshTexture;
	bool m_IsUseGammaCorrection = false;
};

NAMESPACE_END(gl_kernel)