#pragma once
#include "../OpenGLKernel/GLScreen.h"
#include "../OpenGLKernel/GLShader.h"
#include "../OpenGLKernel/GLUtils.h"
#include <GLM/glm.hpp>
#include <memory>

using gl_kernel::CGLScreen;
using gl_kernel::CGLShader;

using gl_utils::CGLTexture;

class CViewer : public CGLScreen
{
public:
	CViewer(const std::string& WindowTitle = "", int vWindowWidth = 800, int vWindowHeight = 600);
	~CViewer();

	void drawContents();

private:
	std::shared_ptr<CGLShader> m_pGLShader = nullptr;
	std::shared_ptr<CGLTexture> m_pTexture = nullptr;

	bool m_IsFirstMouse = true;
	int m_WindowWidth = 0, m_WindowHeight = 0;
};