#pragma once
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "Common.h"

struct ImGuiIO;

NAMESPACE_BEGIN(gl_kernel)

class CAssistGUI
{
public:
	CAssistGUI(const std::string& vTitle);
	~CAssistGUI();

	void initGUI(GLFWwindow* vGLFWWindow);
	void updateGUI();
	void drawGUI();

	void setIsShowDefaultGUI(bool vIsShowDefaultGUI) { m_IsShowDefaultGUI = vIsShowDefaultGUI; }

private:
	ImGuiIO* m_pImGuiIO = nullptr;
	std::string m_Title;
	bool m_IsShowDefaultGUI = true;
};

NAMESPACE_END(gl_kernel)