#include "Viewer.h"

CViewer::CViewer(const std::string& WindowTitle, int vWindowWidth, int vWindowHeight) : CGLScreen(WindowTitle, vWindowWidth, vWindowHeight), m_WindowWidth(vWindowWidth), m_WindowHeight(vWindowHeight)
{
	m_pTexture = std::make_shared<CGLTexture>("textures/container2.png");
	m_pGLShader = std::make_shared<CGLShader>();
	m_pGLShader->initFromFiles("triangle", "shaders/triangle.vert", "shaders/triangle.frag");
	
	std::vector<glm::vec3> Vertices = {
		glm::vec3(-0.5f, -0.5f, 0.0f),
		glm::vec3( 0.5f, -0.5f, 0.0f),
		glm::vec3(-0.5f,  0.5f, 0.0f),
		glm::vec3( 0.5f,  0.5f, 0.0f)
	};
	std::vector<glm::ivec3> Indices =
	{
		glm::ivec3(0, 1, 2),
		glm::ivec3(1, 2, 3)
	};
	std::vector<glm::vec2> TextureCoord = {
		glm::vec2(0.0f, 0.0f),
		glm::vec2(1.0f, 0.0f),
		glm::vec2(0.0f, 1.0f),
		glm::vec2(1.0f, 1.0f)
	};
	m_pGLShader->bind();
	m_pGLShader->uploadAttrib("Pos", Vertices, 3);
	m_pGLShader->uploadAttrib("Indices", Indices, 3);
	m_pGLShader->uploadAttrib("TextureCoord", TextureCoord, 2);

	m_pTexture->bind(m_pTexture->getTextureID());
	m_pGLShader->setIntUniform("u_Texture", m_pTexture->getTextureID());
}

CViewer::~CViewer()
{
}

//***********************************************************************************************
//Function:
void CViewer::drawContents()
{
	//m_pGLShader->drawArray(GL_TRIANGLES, 0, 6);
	m_pGLShader->drawIndexed(GL_TRIANGLES, 0, 2);
}