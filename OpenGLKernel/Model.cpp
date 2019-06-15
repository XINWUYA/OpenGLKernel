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
	vShader.setIntUniform("u_ModelMaterial.Metallic", vStartTextureUnit + 3);
	vShader.setIntUniform("u_ModelMaterial.Roughness", vStartTextureUnit + 4);
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
	const aiScene* pAiScene = Importer.ReadFile(vModelName, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_CalcTangentSpace);
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

	for (unsigned int i = 0; i < vNode->mNumMeshes; ++i)
	{
		aiMesh* pMesh = vScene->mMeshes[vNode->mMeshes[i]];
		_ASSERT(pMesh);
		m_MeshSet.push_back(__processMesh(pMesh, vScene));
	}

	for (unsigned int i = 0; i < vNode->mNumChildren; ++i)
		__processNodeRecursively(vNode->mChildren[i], vScene);
}

//***********************************************************************************************
//Function:
void CGLModel::__processVertices(const aiMesh* vMesh, std::vector<SMeshVertex>& voMeshVerticesSet)
{
	_ASSERT(vMesh);

	for (unsigned int i = 0; i < vMesh->mNumVertices; ++i)
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

	__loadMaterialTextures(pMaterial, aiTextureType_DIFFUSE, "u_ModelMaterial.Diffuse", voMeshTexturesSet);
	__loadMaterialTextures(pMaterial, aiTextureType_SPECULAR, "u_ModelMaterial.Specular", voMeshTexturesSet);
	//__loadMaterialTextures(pMaterial, aiTextureType_NORMALS, "u_ModelMaterial.Normal", voMeshTexturesSet);
	__loadMaterialTextures(pMaterial, aiTextureType_HEIGHT, "u_ModelMaterial.Normal", voMeshTexturesSet);//Normal	
	__loadMaterialTextures(pMaterial, aiTextureType_AMBIENT, "u_ModelMaterial.Metallic", voMeshTexturesSet);//Metallic
	__loadMaterialTextures(pMaterial, aiTextureType_SHININESS, "u_ModelMaterial.Roughness", voMeshTexturesSet);
}

//***********************************************************************************************
//Function:
void CGLModel::__loadMaterialTextures(const aiMaterial* vMaterial, aiTextureType vTextureType, const std::string& vTextureTypeName, std::vector<SMeshTexture>& voMeshTexturesSet)
{
	_ASSERT(vMaterial && !vTextureTypeName.empty());

	unsigned int TextureCnt = vMaterial->GetTextureCount(vTextureType);
	if (TextureCnt <= 0)
	{
		SMeshTexture MeshTexture;//Note: 防止由于缺失某个纹理导致纹理对应错误
		voMeshTexturesSet.push_back(MeshTexture);
		return;
	}

	int TextureIndex = -1;
	for (unsigned int i = 0; i < TextureCnt; ++i)
	{
		aiString TexturePath;
		vMaterial->GetTexture(vTextureType, i, &TexturePath);
		if (TexturePath.length == 0) 
			continue;
		std::string FilePath = m_Directory + '/' + TexturePath.C_Str();

		bool IsSkipFlag = false;
		for (size_t k = 0; k < m_LoadedMeshTextureSet.size(); ++k)
		{
			if (0 == std::strcmp(m_LoadedMeshTextureSet[k].m_TexturePath.data(), FilePath.c_str()))
			{
				m_LoadedMeshTextureSet[k].m_TextureTypeName = vTextureTypeName + std::to_string(++TextureCnt);
				voMeshTexturesSet.push_back(m_LoadedMeshTextureSet[k]);
				IsSkipFlag = true;
				break;
			}
		}
		if (!IsSkipFlag)
		{
			STexture Texture;
			Texture.m_IsFLipVertically = false;
			CGLTexture TempGLTexture(FilePath, Texture);
			
			SMeshTexture MeshTexture;
			MeshTexture.m_TextureID = TempGLTexture.getTextureID();
			MeshTexture.m_TextureTypeName = vTextureTypeName + std::to_string(++TextureCnt);
			MeshTexture.m_TexturePath = FilePath;
			voMeshTexturesSet.push_back(MeshTexture);
			m_LoadedMeshTextureSet.push_back(MeshTexture);
		}
	}
}

NAMESPACE_END(gl_kernel)