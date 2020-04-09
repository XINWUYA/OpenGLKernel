#include "AssistGUI.h"
#include <imgui.h>
#include <examples/imgui_impl_glfw.h>
#include <examples/imgui_impl_opengl3.h>
#include <iostream>
#include <cstdarg>
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
	
	//ImGui::StyleColorsDark();//Set ImGUI Style: Dark/Light/Classic
	ImGui::StyleColorsClassic();//Set ImGUI Style: Dark/Light/Classic
	//ImGui::StyleColorsLight();//Set ImGUI Style: Dark/Light/Classic
	
	ImGui_ImplGlfw_InitForOpenGL(vGLFWWindow, true);
	ImGui_ImplOpenGL3_Init("#version 430");

	//Load Fonts
	//m_pImGuiIO->Fonts->AddFontDefault();
	m_pImGuiIO->Fonts->AddFontFromFileTTF("../Libs/imgui/misc/fonts/Karla-Regular.ttf", 16.0f, nullptr, m_pImGuiIO->Fonts->GetGlyphRangesChineseSimplifiedCommon());
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
void CAssistGUI::sameLine()
{
	ImGui::SameLine();
}

//***********************************************************************************************
//Function:
void CAssistGUI::plotLines(const std::string& vLabelName, const std::vector<float>& vDataSet)
{
	ImGui::PlotLines(vLabelName.c_str(), vDataSet.data(), static_cast<int>(vDataSet.size()), 0, "", FLT_MAX, FLT_MAX, ImVec2(0, 40));
}

//***********************************************************************************************
//Function:
bool CAssistGUI::sliderFloat(const std::string& vLabelName, float& vioValue, float vMin, float vMax)
{
	return ImGui::SliderFloat(vLabelName.c_str(), &vioValue, vMin, vMax);
}

//***********************************************************************************************
//Function:
bool CAssistGUI::colorEdit3(const std::string& vLabelName, float* vColor)
{
	return ImGui::ColorEdit3(vLabelName.c_str(), vColor);
}

//***********************************************************************************************
//Function:
bool CAssistGUI::checkBox(const std::string& vLabelName, bool& vioIsSelected)
{
	return ImGui::Checkbox(vLabelName.c_str(), &vioIsSelected);
}

//***********************************************************************************************
//Function:
bool CAssistGUI::button(const std::string& vLabelName)
{
	return ImGui::Button(vLabelName.c_str());
}

//***********************************************************************************************
//Function:
bool CAssistGUI::combo(const std::string& vLabelName, const std::vector<std::string>& vLabelSet, std::string& vioSelectedLabel)
{
	static ImGuiComboFlags flags = 0;
	if(vioSelectedLabel.empty())
		vioSelectedLabel = vLabelSet.front();
	if (ImGui::BeginCombo(vLabelName.c_str(), vioSelectedLabel.c_str()))
	{
		for (auto& Label : vLabelSet)
		{
			bool IsSelected = (Label == vioSelectedLabel);
			if (ImGui::Selectable(Label.c_str(), IsSelected))
				vioSelectedLabel = Label;
			if (IsSelected)
				ImGui::SetItemDefaultFocus();
		}
		ImGui::EndCombo();
	}
	return true;
}

NAMESPACE_END(gl_kernel)