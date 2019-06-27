#pragma once
#include "../OpenGLKernel/GLScreen.h"
#include "../OpenGLKernel/GLShader.h"
#include "../OpenGLKernel/Camera.h"
#include "../OpenGLKernel/GLUtils.h"
#include <GLM/glm.hpp>
#include <memory>

using gl_kernel::CGLScreen;
using gl_kernel::CGLShader;
using gl_kernel::CCamera;
using gl_kernel::CGLTexture;
using gl_kernel::CGLFrameBuffer;

class CViewer : public CGLScreen
{
public:
	CViewer(const std::string& WindowTitle = "", int vWindowWidth = 800, int vWindowHeight = 600);
	~CViewer();

	virtual void drawContentsV() override;

	virtual void processCursorPosCallbackEventV(double vX, double vY) override;
	virtual void processMouseButtonCallbackEventV(int vButton, int vAction, int vModifiers) override;
	virtual void processKeyCallbackEventV(int vKey, int vScancode, int vAction, int vMods) override;
	virtual void processScrollCallbackEventV(double vX, double vY) override;
	//virtual void processResizeCallbackEvent(int vWidth, int vHeight) override {};

private:
	void __processInput();

	std::shared_ptr<CGLShader> m_pGLShader = nullptr;
	std::shared_ptr<CGLShader> m_pPreComputeBRDFShader = nullptr;
	std::shared_ptr<CCamera> m_pCamera = nullptr;
	std::shared_ptr<CGLTexture> m_pTexture = nullptr;
	std::shared_ptr<CGLFrameBuffer> m_pFrameBuffer = nullptr;

	bool m_IsFirstMouse = true;
	int m_WindowWidth = 0, m_WindowHeight = 0;
	float m_LastMouseX = 0.0f, m_LastMouseY = 0.0f;
	double m_LastGLFWTime = 0.0f, m_DeltaTime = 0.0f;
};