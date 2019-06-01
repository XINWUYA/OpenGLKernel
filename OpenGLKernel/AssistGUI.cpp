#include "AssistGUI.h"
#include "imgui.h"
#include "examples/imgui_impl_glfw.h"
#include "examples/imgui_impl_opengl3.h"
#include "GLScreen.h"

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

	if (m_IsShowDefaultGUI)
	{
		ImGui::Begin(m_Title.c_str());
		ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
		ImGui::End();
	}
}

//***********************************************************************************************
//Function:
void CAssistGUI::drawGUI()
{
	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

NAMESPACE_END(gl_kernel)