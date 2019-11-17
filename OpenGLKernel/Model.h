#pragma once
#include <Assimp/Importer.hpp>
#include <Assimp/scene.h>
#include <Assimp/postprocess.h>
#include "Common.h"
#include "Mesh.h"

NAMESPACE_BEGIN(gl_kernel)

class CGLShader;
class CGLMesh;

class OPENGL_KERNEL_EXPORT CGLModel
{
public:
	CGLModel() = default;
	CGLModel(const std::string& vModelName, bool vIsUseGammaCorrection = false);
	~CGLModel() = default;

	void init(CGLShader& vShader, unsigned int vStartTextureUnit = 0);
	void draw() const;

private:
	void __loadModel(const std::string& vModelName);
	CGLMesh __processMesh(const aiMesh* vMesh, const aiScene* vScene);
	void __processNodeRecursively(const aiNode* vNode, const aiScene* vScene);
	void __processVertices(const aiMesh* vMesh, std::vector<SMeshVertex>& voMeshVerticesSet);
	void __processIndices(const aiMesh* vMesh, std::vector<GLuint>& voMeshIndicesSet);
	void __processTextures(const aiMesh* vMesh, const aiScene* vScene, std::vector<SMeshTexture>& voMeshTexturesSet);
	void __loadMaterialTextures(const aiMaterial* vMaterial, aiTextureType vTextureType, const std::string& vTextureTypeName, std::vector<SMeshTexture>& voMeshTexturesSet);

	std::vector<CGLMesh> m_MeshSet;
	std::vector<SMeshTexture> m_LoadedMeshTextureSet;
	std::string m_Directory;
	unsigned int m_StartTextureUnit = 0;
	bool m_IsUseGammaCorrection = false;
};

NAMESPACE_END(gl_kernel)