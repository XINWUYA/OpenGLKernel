#pragma once
#include <GL/glew.h>
#include <GLM/glm.hpp>
#include <vector>
#include "Common.h"

NAMESPACE_BEGIN(gl_utils)

struct STexture
{
	enum class ETextureType
	{
		TEXTURE_2D = 0,
		TEXTURE_3D,
		TEXTURE_2D_ARRAY,
		TEXTURE_CUBE_MAP
	};
	enum class ETextureAttachmentType
	{
		COLOR_TEXTURE,
		DEPTH_TEXTURE,
		STENCIL_TEXTURE,
		DEPTH_AND_STENCIL_TEXTURE
	};

	GLint m_ID;
	GLint m_Type4WrapS;
	GLint m_Type4WrapT;
	GLint m_Type4WrapR;
	GLint m_Type4MinFilter;
	GLint m_Type4MagFilter;
	GLint m_InternelFormat;
	GLenum m_ExternalFormat;
	GLenum m_DataType;
	GLsizei m_Width;
	GLsizei m_Height;
	GLsizei m_Depth;
	GLboolean m_IsUseMipMap;
	GLboolean m_IsSRGBSpace;
	glm::vec4 m_BorderColor;
	std::string m_TextureName;
	std::vector<void*> m_pDataSet;
	ETextureType m_TextureType;
	ETextureAttachmentType m_AttachmentType;

	STexture() : m_ID(-1), m_Type4WrapS(GL_REPEAT), m_Type4WrapT(GL_REPEAT), m_Type4WrapR(GL_REPEAT), m_Type4MinFilter(GL_LINEAR), m_Type4MagFilter(GL_LINEAR),
		m_InternelFormat(GL_RGBA), m_ExternalFormat(GL_RGBA), m_DataType(GL_UNSIGNED_BYTE), m_Width(0), m_Height(0), m_Depth(1), m_IsUseMipMap(GL_FALSE),
		m_IsSRGBSpace(GL_FALSE), m_BorderColor(0.0, 0.0, 0.0, 1.0), m_TextureName(""), m_pDataSet({}), m_TextureType(ETextureType::TEXTURE_2D), m_AttachmentType(ETextureAttachmentType::COLOR_TEXTURE)
	{ }
};

class CGLTexture
{
public:
	CGLTexture(const std::string& vTextureFileName);
	CGLTexture(const std::string& vTextureFileName, STexture& vTexture);
	~CGLTexture();

	void bind(int vIndex);

	GLuint getTextureID() const { return m_Texture.m_ID; }
	int getTextureWidth() const { return m_Texture.m_Width; }
	int getTextureHeight() const { return m_Texture.m_Height; }

private:
	void __loadTeture(const std::string& vTextureFileName);
	void __loadCommonTexture(const std::string& vTextureFileName);
	void __loadHDRTexture(const std::string& vTextureFileName);
	void __generateTexture();

	STexture m_Texture = {};
};


NAMESPACE_END(gl_utils)