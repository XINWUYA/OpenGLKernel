#pragma once
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "Common.h"

struct ImGuiIO;

NAMESPACE_BEGIN(gl_kernel)

class OPENGL_KERNEL_EXPORT CAssistGUI
{
public:
	CAssistGUI(const std::string& vTitle);
	~CAssistGUI();

	void initGUI(GLFWwindow* vGLFWWindow);
	void updateGUI();
	void drawGUI();

	void text(const char* vStr, ...);
	void sameLine();
	void plotLines(const std::string& vLabelName, const std::vector<float>& vDataSet);
	bool sliderFloat(const std::string& vLabelName, float& vioValue, float vMin, float vMax);
	bool colorEdit3(const std::string& vLabelName, float* vColor);
	bool checkBox(const std::string& vLabelName, bool& vioIsSelected);
	bool button(const std::string& vLabelName);
	bool combo(const std::string& vLabelName, const std::vector<std::string>& vLabelSet, std::string& vioSelectedLabel);

	void setIsShowDefaultGUI(bool vIsShowDefaultGUI) { m_IsShowDefaultGUI = vIsShowDefaultGUI; }

private:
	ImGuiIO* m_pImGuiIO = nullptr;
	std::string m_Title;
	bool m_IsShowDefaultGUI = false;
};

NAMESPACE_END(gl_kernel)