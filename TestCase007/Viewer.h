#pragma once
#include "../OpenGLKernel/GLScreen.h"
#include "../OpenGLKernel/GLShader.h"
#include "../OpenGLKernel/Camera.h"
#include "../OpenGLKernel/GLUtils.h"
#include "../OpenGLKernel/Model.h"
#include "../OpenGLKernel/AssistGUI.h"
#include <GLM/glm.hpp>
#include <memory>
#include <deque>

using gl_kernel::CGLScreen;
using gl_kernel::CGLShader;
using gl_kernel::CCamera;
using gl_kernel::CGLModel;
using gl_kernel::CGLTexture;
using gl_kernel::CAssistGUI;

class CViewer : public CGLScreen
{
public:
	CViewer(const std::string& WindowTitle = "", int vWindowWidth = 1280, int vWindowHeight = 720);
	~CViewer();

	virtual void drawContentsV() override;

	virtual void processCursorPosCallbackEventV(double vX, double vY) override;
	virtual void processMouseButtonCallbackEventV(int vButton, int vAction, int vModifiers) override;
	virtual void processKeyCallbackEventV(int vKey, int vScancode, int vAction, int vMods) override;
	virtual void processScrollCallbackEventV(double vX, double vY) override;
	//virtual void processResizeCallbackEventV(int vWidth, int vHeight) override {};

private:
	void __processInput();
	void __setGUIComponents();
	void __calculateTime();
	double __calculateFrameRateInMilliSecond();
	double __calcullateFPS();
	void __updateLightPos();

	std::shared_ptr<CGLShader> m_pGLShader = nullptr;
	std::shared_ptr<CCamera> m_pCamera = nullptr;
	std::shared_ptr<CGLTexture> m_pTexture = nullptr;
	std::shared_ptr<CGLModel> m_pModel = nullptr;
	std::shared_ptr<CAssistGUI> m_pAssistGUI = nullptr;
	bool m_IsFirstMouse = true;
	bool m_IsDynamicLight = false;
	bool m_IsUsePBR = true;
	int m_WindowWidth = 0, m_WindowHeight = 0;
	float m_LastMouseX = 0.0f, m_LastMouseY = 0.0f;
	double m_LastGLFWTime = 0.0f, m_DeltaTime = 0.0f;
	float m_LightPosX = 0.0f, m_LightPosY = 0.0f, m_LightPosZ = 0.0f;
	float m_LightMovingStepLength = 0.01f;
	float m_LightIntensity = 1.0f;
	glm::vec3 m_LightColor = glm::vec3(1.0f);
	glm::vec3 m_LightMovingDir = glm::vec3(0.0f);
	std::deque<float> m_FrameDeque;
	std::vector<float> m_FrameRateSet;
	std::vector<std::string> m_LabelSet = { "PBR", "PHONG" };
	std::string m_Selectedlabel;
	std::default_random_engine m_RandomEngine;
};