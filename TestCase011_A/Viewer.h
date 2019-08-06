#pragma once
#include "../OpenGLKernel/GLScreen.h"
#include "../OpenGLKernel/GLShader.h"
#include "../OpenGLKernel/Camera.h"
#include "../OpenGLKernel/GLUtils.h"
#include "../OpenGLKernel/AssistGUI.h"
#include <GLM/glm.hpp>
#include <memory>
#include <deque>

using gl_kernel::CGLScreen;
using gl_kernel::CGLShader;
using gl_kernel::CCamera;
using gl_kernel::CAssistGUI;
using gl_kernel::CGLTexture;

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
	//virtual void processResizeCallbackEvent(int vWidth, int vHeight) override {};

private:
	void __processInput();
	void __setGUIComponents();
	void __calculateTime();
	double __calculateFrameRateInMilliSecond();
	double __calcullateFPS();

	std::shared_ptr<CGLShader> m_pLightShader = nullptr;
	std::shared_ptr<CGLShader> m_pGroundShader = nullptr;
	std::shared_ptr<CCamera> m_pCamera = nullptr;
	std::shared_ptr<CAssistGUI> m_pAssistGUI = nullptr;

	bool m_IsFirstMouse = true;
	int m_WindowWidth = 0, m_WindowHeight = 0;
	float m_LastMouseX = 0.0f, m_LastMouseY = 0.0f;
	double m_LastGLFWTime = 0.0f, m_DeltaTime = 0.0f;
	int m_DrawElementsCnt = 0;
	bool m_IsTwoSide = false;
	float m_Roughness = 0.0f;
	float m_LightWidth = 1.0f;
	float m_LightHeight = 1.0f;
	float m_LightIntensity = 1.0f;
	glm::vec3 m_LightRotateAngle = glm::vec3(180.0f, 0.0f, 0.0f);
	glm::vec3 m_LightTranslatePos = glm::vec3(0.0f);
	glm::vec3 m_LightColor = glm::vec3(1.0f);
	std::vector<glm::vec3> m_LightVertexPosSet = { glm::vec3(-1.0f, -1.0f, 0.0f), glm::vec3(1.0f, -1.0f, 0.0f), glm::vec3(1.0f, 1.0f, 0.0f), glm::vec3(-1.0f, 1.0f, 0.0f) };
	std::deque<float> m_FrameDeque;
	std::vector<float> m_FrameRateSet;
};