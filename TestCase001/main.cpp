#include <iostream>
#include <memory>
#include "../OpenGLKernel/GLScreen.h"
#include "../OpenGLKernel/GLShader.h"
#include <GLM/glm.hpp>

using gl_kernel::CGLScreen;
using gl_kernel::CGLShader;

class CViewer : public CGLScreen
{
public:
	CViewer() : CGLScreen("Triangle", 800, 600)
	{
		m_pGLShader = std::make_shared<CGLShader>();
		m_pGLShader->initFromFiles("triangle", "shaders/triangle.vert", "shaders/triangle.frag");
		std::vector<glm::vec3> Vertices = {
			glm::vec3(-0.5f, -0.5f, 0.0f),
			glm::vec3( 0.5f, -0.5f, 0.0f),
			glm::vec3(-0.5f,  0.5f, 0.0f)
		};
		m_pGLShader->bind();
		m_pGLShader->uploadAttrib("Pos", Vertices, 3);
	}
	~CViewer() = default;

	void drawContents()
	{
		m_pGLShader->bind();
		m_pGLShader->drawArray(GL_TRIANGLES, 0, 3);
	}


private:
	std::shared_ptr<CGLShader> m_pGLShader = nullptr;
};

int main()
{
	CViewer Viewer;
	while(!Viewer.isWindowShouldClosed())
		Viewer.drawAll();

	return 0;
}