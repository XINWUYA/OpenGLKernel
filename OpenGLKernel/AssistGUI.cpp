#include "AssistGUI.h"
#include "imgui.h"
#include "examples/imgui_impl_glfw.h"
#include "examples/imgui_impl_opengl3.h"
#include "GLScreen.h"
#include <iostream>
#include <cstdarg>

NAMESPACE_BEGIN(gl_kernel)


CAssistGUI::CAssistGUI(const std::string& vTitle) : m_Title(vTitle)
{
}

CAssistGUI::~CAssistGUI()
{
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
}

//***********************************************************************************************
//Function:
void CAssistGUI::initGUI(GLFWwindow* vGLFWWindow)
{
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	m_pImGuiIO = &ImGui::GetIO();
	
	ImGui::StyleColorsDark();//Set ImGUI Style: Dark/Light/Classic
	
	ImGui_ImplGlfw_InitForOpenGL(vGLFWWindow, true);
	ImGui_ImplOpenGL3_Init("#version 400");
}

//***********************************************************************************************
//Function:
void CAssistGUI::updateGUI()
{
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();
	
	ImGui::Begin(m_Title.c_str());

	if (m_IsShowDefaultGUI)
	{
		static float f = 0.0f;
		ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
		ImGui::SliderFloat("float", &f, 0.0f, 1.0f);
	}
}

//***********************************************************************************************
//Function:
void CAssistGUI::drawGUI()
{
	ImGui::End();

	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

//***********************************************************************************************
//Function:
void CAssistGUI::text(const char* vStr, ...)
{
	va_list args;
	va_start(args, vStr);
	ImGui::TextV(vStr, args);
	va_end(args);
}

//***********************************************************************************************
//Function:
bool CAssistGUI::sliderFloat(const std::string& vName, float& vioValue, float vMin, float vMax)
{
	return ImGui::SliderFloat(vName.c_str(), &vioValue, vMin, vMax);
}

//***********************************************************************************************
//Function:
bool CAssistGUI::colorEdit3(const std::string& vName, float* vColor)
{
	return ImGui::ColorEdit3(vName.c_str(), vColor);
}

NAMESPACE_END(gl_kernel)