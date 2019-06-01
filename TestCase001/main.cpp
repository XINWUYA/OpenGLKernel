#include <iostream>
#include <memory>
#include "../OpenGLKernel/GLScreen.h"
#include "../OpenGLKernel/GLShader.h"
#include "../OpenGLKernel/Interface.h"
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

	virtual void drawContentsV() override
	{
		m_pGLShader->bind();
		m_pGLShader->drawArray(GL_TRIANGLES, 0, 3);
	}


private:
	std::shared_ptr<CGLShader> m_pGLShader = nullptr;
};

int main()
{
	try
	{
		gl_kernel::init();

		{
			CViewer Viewer;
			Viewer.setVisible(true);

			gl_kernel::mainloop();
		}

		gl_kernel::shutdown();
	}
	catch (const std::runtime_error& vErr)
	{
		std::string ErrorMsg = std::string("Caught a fatal error: ") + std::string(vErr.what());
		std::cerr << ErrorMsg << std::endl;
		return -1;
	}

	return 0;
}