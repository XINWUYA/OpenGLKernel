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
using gl_kernel::CGLTexture;
using gl_kernel::CGLFrameBuffer;
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

	std::shared_ptr<CGLShader> m_pDrawCubeMapShader = nullptr;
	std::shared_ptr<CGLShader> m_pHDR2CubeMapShader = nullptr;
	std::shared_ptr<CGLShader> m_pDrawModelShader = nullptr;
	std::shared_ptr<CGLShader> m_pPreComputeIrradianceMapShader = nullptr;
	std::shared_ptr<CGLShader> m_pPreFilterEnvironmentMapShader = nullptr;
	std::shared_ptr<CGLShader> m_pPreComputeBRDFShader = nullptr;
	std::shared_ptr<CCamera> m_pCamera = nullptr;
	std::shared_ptr<CGLTexture> m_pHDRTexture = nullptr;
	std::shared_ptr<CGLTexture> m_pCubeMapTexture = nullptr;
	std::shared_ptr<CGLTexture> m_pIrradianceMapTexture = nullptr;
	std::shared_ptr<CGLTexture> m_pEnvironmentMapTexture = nullptr;
	std::shared_ptr<CGLTexture> m_pBRDFTexture = nullptr;
	std::shared_ptr<CAssistGUI> m_pAssistGUI = nullptr;
	std::shared_ptr<CGLFrameBuffer> m_pFrameBuffer = nullptr;

	bool m_IsFirstMouse = true;
	int m_DrawElementsCnt = 0;
	int m_WindowWidth = 0, m_WindowHeight = 0;
	int m_FBO = -1;
	float m_LastMouseX = 0.0f, m_LastMouseY = 0.0f;
	double m_LastGLFWTime = 0.0f, m_DeltaTime = 0.0f;
	float m_RotateSpeed = 1.0f;
	float m_LightPosX = 4.0f, m_LightPosY = 1.0f, m_LightPosZ = 2.0f;
	float m_LightIntensity = 1.0f;
	float m_ModelRoughness = 0.0f;
	float m_ModelMetallic = 0.0f;
	float m_ClearCoatRoughness = 0.0f;
	float m_ClearCoatThickness = 0.0f;
	glm::vec3 m_LightColor = glm::vec3(1.0f);
	std::deque<float> m_FrameDeque;
	std::vector<float> m_FrameRateSet;
};