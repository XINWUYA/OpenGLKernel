#include "GLUtils.h"
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#include <gli/gli.hpp>
#include <iostream>

NAMESPACE_BEGIN(gl_kernel)

CGLTexture::CGLTexture(const STexture& vTexture) : m_Texture(vTexture)
{
	__generateTexture();
}

CGLTexture::CGLTexture(const std::string& vTextureFileName)
{
	__loadTeture(vTextureFileName);
}

CGLTexture::CGLTexture(const std::string& vTextureFileName, STexture& vTexture) : m_Texture(vTexture)
{
	__loadTeture(vTextureFileName);
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

	if (FileExtension == ".hdr")
		__loadHDRTexture(vTextureFileName);
	else if (FileExtension == ".dds")
		__loadDDSTexture(vTextureFileName);
	else/* if(FileExtension == "png" || FileExtension == "jpg" || FileExtension == "bmp")*/
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
		std::cerr << "Error: Common Texture Load Failed." << std::endl;
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
		m_Texture.m_Channels = TextureChannels;
		m_Texture.m_pDataSet.push_back(pImageData);

		__generateTexture();
		
		stbi_image_free(pImageData);
	}
}

//***********************************************************************************************
//Function:
void CGLTexture::__loadHDRTexture(const std::string& vTextureFileName)
{
	if (m_Texture.m_IsFLipVertically)
		stbi_set_flip_vertically_on_load(true);

	int TextureWidth = 0, TextureHeight = 0, TextureChannels = 0;
	GLvoid* pImageData = stbi_loadf(vTextureFileName.c_str(), &TextureWidth, &TextureHeight, &TextureChannels, 0);
	if (!pImageData)
	{
		std::cerr << "Error: HDR Texture Load Failed." << std::endl;
		return;
	}
	else
	{
		switch (TextureChannels)
		{
		case 1:
			m_Texture.m_InternelFormat = GL_R16F;
			m_Texture.m_ExternalFormat = GL_RED;
			break;
		case 2:
			m_Texture.m_InternelFormat = GL_RG16F;
			m_Texture.m_ExternalFormat = GL_RG;
			break;
		case 3:
			m_Texture.m_InternelFormat = GL_RGB16F;
			m_Texture.m_ExternalFormat = GL_RGB;
			break;
		case 4:
			m_Texture.m_InternelFormat = GL_RGBA16F;
			m_Texture.m_ExternalFormat = GL_RGBA;
			break;
		default:
			break;
		}
		m_Texture.m_DataType = GL_FLOAT;
		m_Texture.m_Width = TextureWidth;
		m_Texture.m_Height = TextureHeight;
		m_Texture.m_Channels = TextureChannels;
		m_Texture.m_Type4WrapS = m_Texture.m_Type4WrapT = m_Texture.m_Type4WrapR = GL_CLAMP_TO_EDGE;
		m_Texture.m_IsUseMipMap = GL_FALSE;
		m_Texture.m_pDataSet.push_back(pImageData);

		__generateTexture();

		stbi_image_free(pImageData);
	}
}

//***********************************************************************************************
//Function:
void CGLTexture::__loadDDSTexture(const std::string& vTextureFileName)
{
	gli::texture GLITexture = gli::load(vTextureFileName);
	if (GLITexture.empty())
	{
		std::cerr << "Error: DDS Texture Load Failed." << std::endl;
		return;
	}
	else
	{
		gli::gl GL(gli::gl::PROFILE_GL33);
		gli::gl::format const GLIFormat = GL.translate(GLITexture.format(), GLITexture.swizzles());
		m_Texture.m_InternelFormat = GLIFormat.Internal;
		m_Texture.m_ExternalFormat = GLIFormat.External;
		m_Texture.m_DataType = GLIFormat.Type;
		m_Texture.m_Width = GLITexture.extent().x;
		m_Texture.m_Height = GLITexture.extent().y;
		m_Texture.m_pDataSet.push_back(GLITexture.data());
		
		__generateTexture();
	}
}

//***********************************************************************************************
//Function:
void CGLTexture::__generateTexture()
{
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
		glBindTexture(TextureType, m_Texture.m_ID);
		glTexImage3D(TextureType, 0, m_Texture.m_InternelFormat, m_Texture.m_Width, m_Texture.m_Height, m_Texture.m_Depth, 0, m_Texture.m_ExternalFormat, m_Texture.m_DataType, m_Texture.m_pDataSet.empty() ? nullptr : m_Texture.m_pDataSet.data());
		break;
	case STexture::ETextureType::TEXTURE_CUBE_MAP:
		TextureType = GL_TEXTURE_CUBE_MAP;
		glBindTexture(TextureType, m_Texture.m_ID);
		for (size_t i = 0; i < 6; ++i)
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, m_Texture.m_InternelFormat, m_Texture.m_Width, m_Texture.m_Height, 0, m_Texture.m_ExternalFormat, m_Texture.m_DataType, m_Texture.m_pDataSet.size() > i ? m_Texture.m_pDataSet[i] : nullptr);
		break;
	default:
		break;
	}
	//glTexParameterfv(TextureType, GL_TEXTURE_BORDER_COLOR, m_Texture.m_BorderColor.data());
	glTexParameteri(TextureType, GL_TEXTURE_WRAP_S, m_Texture.m_Type4WrapS);
	glTexParameteri(TextureType, GL_TEXTURE_WRAP_T, m_Texture.m_Type4WrapT);
	if(m_Texture.m_Channels >= 3)
		glTexParameteri(TextureType, GL_TEXTURE_WRAP_R, m_Texture.m_Type4WrapR);

	if (m_Texture.m_IsUseMipMap && m_Texture.m_Type4MagFilter == GL_LINEAR)
		m_Texture.m_Type4MinFilter = GL_LINEAR_MIPMAP_LINEAR;
	glTexParameteri(TextureType, GL_TEXTURE_MIN_FILTER, m_Texture.m_Type4MinFilter);
	glTexParameteri(TextureType, GL_TEXTURE_MAG_FILTER, m_Texture.m_Type4MagFilter);
	if (m_Texture.m_IsUseMipMap)
		glGenerateMipmap(TextureType);
}

//***********************************************************************************************
//CGLFrameBuffer
//***********************************************************************************************

CGLFrameBuffer::CGLFrameBuffer()
{
	glGenFramebuffers(1, &m_FrameBuffer);
}

//***********************************************************************************************
//Function:
void CGLFrameBuffer::init(const std::initializer_list<CGLTexture*>& vTextureAttacments, int vSamples)
{
	m_Samples = vSamples;
	
	glBindFramebuffer(GL_FRAMEBUFFER, m_FrameBuffer);
	
	m_FrameSize = glm::ivec2((*vTextureAttacments.begin())->getTextureWidth(), (*vTextureAttacments.begin())->getTextureHeight());

	GLint i = -1;
	std::vector<GLenum> TextureAttachmentSet;
	TextureAttachmentSet.reserve(vTextureAttacments.size());
	for (const CGLTexture* pTempTexture : vTextureAttacments)
	{
		_ASSERT(pTempTexture->getTextureID() >= 0);

		switch (pTempTexture->getTextureAttachmentType())
		{
		case STexture::ETextureAttachmentType::DEPTH_TEXTURE:
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, pTempTexture->getTextureID(), 0);
			break;
		case STexture::ETextureAttachmentType::STENCIL_TEXTURE:
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_TEXTURE_2D, pTempTexture->getTextureID(), 0);
			break;
		case STexture::ETextureAttachmentType::DEPTH_AND_STENCIL_TEXTURE:
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, pTempTexture->getTextureID(), 0);
			break;
		default:
			switch (pTempTexture->getTextureType())
			{
			case STexture::ETextureType::TEXTURE_2D:
				glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + (++i), GL_TEXTURE_2D, pTempTexture->getTextureID(), 0);
				TextureAttachmentSet.push_back(GL_COLOR_ATTACHMENT0 + i);
				break;
			case STexture::ETextureType::TEXTURE_2D_ARRAY:
				for (int k = 0; k < pTempTexture->getTextureDepth(); ++k)
				{
					glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + (++i), pTempTexture->getTextureID(), 0, k);
					TextureAttachmentSet.push_back(GL_COLOR_ATTACHMENT0 + i);
				}
				break;
			case STexture::ETextureType::TEXTURE_CUBE_MAP:
				glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + (++i), pTempTexture->getTextureID(), 0);
				TextureAttachmentSet.push_back(GL_COLOR_ATTACHMENT0 + i);
				break;
			default:
				break;
			}
			break;
		}
	}
	if (TextureAttachmentSet.size())
		glDrawBuffers(TextureAttachmentSet.size(), &TextureAttachmentSet[0]);

	auto genRenderBuffer = [](GLuint vBufferID, GLenum vInternelFormat, GLenum vAttachmentType, int vSamples, glm::ivec2& vSize)
	{
		glGenRenderbuffers(1, &vBufferID);
		glBindRenderbuffer(GL_RENDERBUFFER, vBufferID);
		if (vSamples <= 1)
			glRenderbufferStorage(GL_RENDERBUFFER, vInternelFormat, vSize.x, vSize.y);
		else
			glRenderbufferStorageMultisample(GL_RENDERBUFFER, vSamples, vInternelFormat, vSize.x, vSize.y);

		glFramebufferRenderbuffer(GL_RENDERBUFFER, vAttachmentType, GL_RENDERBUFFER, vBufferID);
	};

	genRenderBuffer(m_ColorBuffer, GL_RGBA8, GL_COLOR_ATTACHMENT0, m_Samples, m_FrameSize);
	genRenderBuffer(m_DepthBuffer, GL_DEPTH24_STENCIL8, GL_DEPTH_STENCIL_ATTACHMENT, m_Samples, m_FrameSize);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		std::cerr << "Error: CGLFrameBuffer: Framebuffer is not completed." << std::endl;

	release();
}

//***********************************************************************************************
//Function:
void CGLFrameBuffer::bind()
{
	glBindFramebuffer(GL_FRAMEBUFFER, m_FrameBuffer);
	
	if (m_Samples > 1)
		glEnable(GL_MULTISAMPLE);
}

//***********************************************************************************************
//Function:
void CGLFrameBuffer::free()
{
	glDeleteRenderbuffers(1, &m_ColorBuffer);
	glDeleteRenderbuffers(1, &m_DepthBuffer);
	m_ColorBuffer = m_DepthBuffer = 0;
}

//***********************************************************************************************
//Function:
void CGLFrameBuffer::release()
{
	if (m_Samples > 1)
		glDisable(GL_MULTISAMPLE);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

//***********************************************************************************************
//Function:
void CGLFrameBuffer::blit()
{
	glBindFramebuffer(GL_READ_FRAMEBUFFER, m_FrameBuffer);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
	glDrawBuffer(GL_BACK);

	glBlitFramebuffer(0, 0, m_FrameSize.x, m_FrameSize.y, 0, 0, m_FrameSize.x, m_FrameSize.y, GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT, GL_NEAREST);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

NAMESPACE_END(gl_kernel)