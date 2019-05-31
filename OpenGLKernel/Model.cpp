#include "Model.h"
#include "GLShader.h"
#include "GLUtils.h"
#include "Mesh.h"
#include <iostream>

NAMESPACE_BEGIN(gl_kernel)

CGLModel::CGLModel(const std::string& vModelName, bool vIsUseGammaCorrection) : m_IsUseGammaCorrection(vIsUseGammaCorrection)
{
	_ASSERT(!vModelName.empty());
	__loadModel(vModelName);
}

//***********************************************************************************************
//Function:
void CGLModel::init(CGLShader& vShader, unsigned int vStartTextureUnit)
{
	m_StartTextureUnit = vStartTextureUnit;
	vShader.bind();
	vShader.setIntUniform("u_ModelMaterial.Diffuse", vStartTextureUnit);
	vShader.setIntUniform("u_ModelMaterial.Specular", vStartTextureUnit + 1);
	vShader.setIntUniform("u_ModelMaterial.Normal", vStartTextureUnit + 2);
	vShader.setIntUniform("u_ModelMaterial.Ambient", vStartTextureUnit + 3);
}

//***********************************************************************************************
//Function:
void CGLModel::draw() const
{
	for (auto& Mesh : m_MeshSet)
		Mesh.draw(m_StartTextureUnit);
}

//***********************************************************************************************
//Function:
void CGLModel::__loadModel(const std::string& vModelName)
{
	Assimp::Importer Importer;
	const aiScene* pAiScene = Importer.ReadFile(vModelName, aiProcess_Triangulate | aiProcess_FlipUVs);
	if (!pAiScene || pAiScene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !pAiScene->mRootNode)
	{
		std::cout << "Error: Assimp: " << Importer.GetErrorString() << std::endl;
		return;
	}
	m_Directory = vModelName.substr(0, vModelName.find_last_of('/'));
	__processNodeRecursively(pAiScene->mRootNode, pAiScene);
}

//***********************************************************************************************
//Function:
CGLMesh CGLModel::__processMesh(const aiMesh* vMesh, const aiScene* vScene)
{
	_ASSERT(vMesh);

	std::vector<SMeshVertex> MeshVerticesSet;
	std::vector<GLuint> MeshIndicesSet;
	std::vector<SMeshTexture> MeshTexturesSet;
	__processVertices(vMesh, MeshVerticesSet);
	__processIndices(vMesh, MeshIndicesSet);
	__processTextures(vMesh, vScene, MeshTexturesSet);

	return CGLMesh(MeshVerticesSet, MeshIndicesSet, MeshTexturesSet);
}

//***********************************************************************************************
//Function:
void CGLModel::__processNodeRecursively(const aiNode* vNode, const aiScene* vScene)
{
	_ASSERT(vNode && vScene);

	for (auto i = 0; i < vNode->mNumMeshes; ++i)
	{
		aiMesh* pMesh = vScene->mMeshes[vNode->mMeshes[i]];
		_ASSERT(pMesh);
		m_MeshSet.push_back(__processMesh(pMesh, vScene));
	}

	for (auto i = 0; i < vNode->mNumChildren; ++i)
		__processNodeRecursively(vNode->mChildren[i], vScene);
}

//***********************************************************************************************
//Function:
void CGLModel::__processVertices(const aiMesh* vMesh, std::vector<SMeshVertex>& voMeshVerticesSet)
{
	_ASSERT(vMesh);

	for (auto i = 0; i < vMesh->mNumVertices; ++i)
	{
		SMeshVertex Vertex;
		Vertex.m_Position = glm::vec3(vMesh->mVertices[i].x, vMesh->mVertices[i].y, vMesh->mVertices[i].z);
		Vertex.m_Normal = glm::vec3(vMesh->mNormals[i].x, vMesh->mNormals[i].y, vMesh->mNormals[i].z);

		if (vMesh->mTextureCoords[0])
			Vertex.m_TexCoords = glm::vec2(vMesh->mTextureCoords[0][i].x, vMesh->mTextureCoords[0][i].y);
		else
			Vertex.m_TexCoords = glm::vec2(0.0f, 0.0f);
		if (vMesh->mTangents)
			Vertex.m_Tangent = glm::vec3(vMesh->mTangents[i].x, vMesh->mTangents[i].y, vMesh->mTangents[i].z);
		if (vMesh->mBitangents)
			Vertex.m_Bitangent = glm::vec3(vMesh->mBitangents[i].x, vMesh->mBitangents[i].y, vMesh->mBitangents[i].z);

		voMeshVerticesSet.push_back(Vertex);
	}
}

//***********************************************************************************************
//Function:
void CGLModel::__processIndices(const aiMesh* vMesh, std::vector<GLuint>& voMeshIndicesSet)
{
	_ASSERT(vMesh);

	for (unsigned int i = 0; i < vMesh->mNumFaces; ++i)
	{
		aiFace Face = vMesh->mFaces[i];
		for (unsigned int k = 0; k < Face.mNumIndices; k++)
			voMeshIndicesSet.push_back(Face.mIndices[k]);
	}
}

//***********************************************************************************************
//Function:
void CGLModel::__processTextures(const aiMesh* vMesh, const aiScene* vScene, std::vector<SMeshTexture>& voMeshTexturesSet)
{
	_ASSERT(vMesh && vScene);

	aiMaterial* pMaterial = vScene->mMaterials[vMesh->mMaterialIndex];
	_ASSERT(pMaterial);

	__loadMaterialTextures(pMaterial, aiTextureType_DIFFUSE, "u_DiffuseTexture", voMeshTexturesSet);
	__loadMaterialTextures(pMaterial, aiTextureType_SPECULAR, "u_SpecularTexture", voMeshTexturesSet);
	__loadMaterialTextures(pMaterial, aiTextureType_NORMALS, "u_NormalTexture", voMeshTexturesSet);
	__loadMaterialTextures(pMaterial, aiTextureType_AMBIENT, "u_AmbientTexture", voMeshTexturesSet);
}

//***********************************************************************************************
//Function:
void CGLModel::__loadMaterialTextures(const aiMaterial* vMaterial, aiTextureType vTextureType, const std::string& vTextureUniformName, std::vector<SMeshTexture>& voMeshTexturesSet)
{
	_ASSERT(vMaterial && !vTextureUniformName.empty());

	for (auto i = 0; i < vMaterial->GetTextureCount(vTextureType); ++i)
	{
		aiString TexturePath;
		vMaterial->GetTexture(vTextureType, i, &TexturePath);
		bool IsSkipFlag = false;
		for (auto k = 0; k < m_LoadedMeshTextureSet.size(); ++k)
		{
			if (0 == std::strcmp(m_LoadedMeshTextureSet[k].m_TexturePath.data(), TexturePath.C_Str()))
			{
				voMeshTexturesSet.push_back(m_LoadedMeshTextureSet[k]);
				IsSkipFlag = true;
				break;
			}
		}
		if (!IsSkipFlag)
		{
			std::string FileName = m_Directory + '/' + TexturePath.C_Str();
			STexture Texture;
			Texture.m_IsFLipVertically = false;
			CGLTexture TempGLTexture(FileName, Texture);
			
			SMeshTexture MeshTexture;
			MeshTexture.m_TextureID = TempGLTexture.getTextureID();
			MeshTexture.m_TextureType = vTextureType;
			MeshTexture.m_TexturePath = TexturePath.C_Str();
			voMeshTexturesSet.push_back(MeshTexture);
			m_LoadedMeshTextureSet.push_back(MeshTexture);
		}
	}
}

NAMESPACE_END(gl_kernel)