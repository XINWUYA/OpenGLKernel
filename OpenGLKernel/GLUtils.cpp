#include "GLUtils.h"
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#include <iostream>

NAMESPACE_BEGIN(gl_kernel)

CGLTexture::CGLTexture(const std::string& vTextureFileName)
{
	__loadTeture(vTextureFileName);
}

CGLTexture::CGLTexture(const std::string& vTextureFileName, STexture& vTexture) : m_Texture(vTexture)
{
	__loadTeture(vTextureFileName);
}

CGLTexture::~CGLTexture()
{
}

//***********************************************************************************************
//Function:
void CGLTexture::bind(int vIndex)
{
	glActiveTexture(GL_TEXTURE0 + vIndex);
	switch (m_Texture.m_TextureType)
	{
	case STexture::ETextureType::TEXTURE_2D:		
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, m_Texture.m_ID);
		break;
	case STexture::ETextureType::TEXTURE_2D_ARRAY:
		glEnable(GL_TEXTURE_2D_ARRAY);
		glBindTexture(GL_TEXTURE_2D_ARRAY, m_Texture.m_ID);
		break;
	case STexture::ETextureType::TEXTURE_3D:
		glEnable(GL_TEXTURE_3D);
		glBindTexture(GL_TEXTURE_3D, m_Texture.m_ID);
		break;
	case STexture::ETextureType::TEXTURE_CUBE_MAP:
		glEnable(GL_TEXTURE_CUBE_MAP);
		glBindTexture(GL_TEXTURE_CUBE_MAP, m_Texture.m_ID);
		break;
	default:
		break;
	}
}

//***********************************************************************************************
//Function:
void CGLTexture::__loadTeture(const std::string& vTextureFileName)
{
	std::string FileExtension;
	if (vTextureFileName.size() > 4)
		FileExtension = convertStr2Lower(vTextureFileName.substr(vTextureFileName.size() - 4));

	if (FileExtension == "hdr")
		__loadHDRTexture(vTextureFileName);
	else/* if(FileExtension == "png" || FileExtension == "jpg" || FileExtension == "bmp")*///未完成
		__loadCommonTexture(vTextureFileName);
}

//***********************************************************************************************
//Function:
void CGLTexture::__loadCommonTexture(const std::string& vTextureFileName)
{
	if(m_Texture.m_IsFLipVertically)
		stbi_set_flip_vertically_on_load(true);

	int TextureWidth = 0, TextureHeight = 0, TextureChannels = 0;
	GLvoid* pImageData = stbi_load(vTextureFileName.c_str(), &TextureWidth, &TextureHeight, &TextureChannels, 0);
	if (!pImageData)
	{
		std::cerr << "Error: Texture Load Failed." << std::endl;
		return;
	}
	else
	{
		switch (TextureChannels)
		{
		case 1: 
			m_Texture.m_InternelFormat = m_Texture.m_ExternalFormat = GL_RED;
			break;
		case 2: 
			m_Texture.m_InternelFormat = m_Texture.m_ExternalFormat = GL_RG;
			break;
		case 3: 
			m_Texture.m_InternelFormat = m_Texture.m_IsSRGBSpace ? GL_SRGB : GL_RGB;
			m_Texture.m_ExternalFormat = GL_RGB;
			break;
		case 4: 
			m_Texture.m_InternelFormat = m_Texture.m_IsSRGBSpace ? GL_SRGB_ALPHA : GL_RGBA;
			m_Texture.m_ExternalFormat = GL_RGBA;
			break;
		default: 
			break;
		}
		m_Texture.m_DataType = GL_UNSIGNED_BYTE;
		m_Texture.m_Width = TextureWidth;
		m_Texture.m_Height = TextureHeight;
		m_Texture.m_pDataSet.push_back(pImageData);

		__generateTexture();
		
		stbi_image_free(pImageData);
	}
}

//***********************************************************************************************
//Function:
void CGLTexture::__loadHDRTexture(const std::string& vTextureFileName)
{//未完成
}

//***********************************************************************************************
//Function:
void CGLTexture::__generateTexture()
{
	//GLuint TextureID = 0;
	glGenTextures(1, &(GLuint&)m_Texture.m_ID);

	GLuint TextureType = 0;
	switch (m_Texture.m_TextureType)
	{
	case STexture::ETextureType::TEXTURE_2D:
		TextureType = GL_TEXTURE_2D;
		glBindTexture(TextureType, m_Texture.m_ID);
		glTexImage2D(TextureType, 0, m_Texture.m_InternelFormat, m_Texture.m_Width, m_Texture.m_Height, 0, m_Texture.m_ExternalFormat, m_Texture.m_DataType, m_Texture.m_pDataSet.empty() ? nullptr : m_Texture.m_pDataSet[0]);
		break;
	case STexture::ETextureType::TEXTURE_2D_ARRAY:
		TextureType = GL_TEXTURE_2D_ARRAY;
		glBindTexture(TextureType, m_Texture.m_ID);
		glTexImage3D(TextureType, 0, m_Texture.m_InternelFormat, m_Texture.m_Width, m_Texture.m_Height, m_Texture.m_Depth, 0, m_Texture.m_ExternalFormat, m_Texture.m_DataType, nullptr);
		for (size_t i = 0; i < m_Texture.m_pDataSet.size(); ++i)
			glTexSubImage3D(TextureType, 0, 0, 0, i, m_Texture.m_Width, m_Texture.m_Height, 1, m_Texture.m_ExternalFormat, m_Texture.m_DataType, m_Texture.m_pDataSet[i]);
		break;
	case STexture::ETextureType::TEXTURE_3D://未完成
		TextureType = GL_TEXTURE_3D;
		break;
	case STexture::ETextureType::TEXTURE_CUBE_MAP://未完成
		break;
	default:
		break;
	}
	//glTexParameterfv(TextureType, GL_TEXTURE_BORDER_COLOR, m_Texture.m_BorderColor.data());
	glTexParameteri(TextureType, GL_TEXTURE_WRAP_S, m_Texture.m_Type4WrapS);
	glTexParameteri(TextureType, GL_TEXTURE_WRAP_T, m_Texture.m_Type4WrapT);
	glTexParameteri(TextureType, GL_TEXTURE_WRAP_R, m_Texture.m_Type4WrapR);

	if (m_Texture.m_IsUseMipMap && m_Texture.m_Type4MinFilter == GL_LINEAR)
		m_Texture.m_Type4MagFilter = GL_LINEAR_MIPMAP_LINEAR;
	glTexParameteri(TextureType, GL_TEXTURE_MIN_FILTER, m_Texture.m_Type4MinFilter);
	glTexParameteri(TextureType, GL_TEXTURE_MAG_FILTER, m_Texture.m_Type4MagFilter);
	if (m_Texture.m_IsUseMipMap)
		glGenerateMipmap(TextureType);

	//m_Texture.m_ID = TextureID;
}

NAMESPACE_END(gl_kernel)