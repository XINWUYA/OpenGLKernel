#pragma once
#include <string>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "Common.h"

NAMESPACE_BEGIN(gl_kernel)

class CGLScreen
{
public:
	CGLScreen(const std::string& vWindowTitle, int vWindowWidth, int vWindowHeight, bool vIsFullScreen = false, int vSamples = 0, unsigned int vGLMajor = 4, unsigned int vGLMinor = 3);
	virtual ~CGLScreen();

	virtual void drawAllV();
	virtual void drawContentsV() {  /* To be overridden */ }

	GLFWwindow* fetchGLFWWindow() const { return m_pGLFWWindow; }
	bool isWindowShouldClosed() const;
	bool isVisible() const { return m_IsVisible; }

	void setVisible(bool vIsVisible);

	virtual void processCursorPosCallbackEventV(double vX, double vY);
	virtual void processMouseButtonCallbackEventV(int vButton, int vAction, int vModifiers);
	virtual void processKeyCallbackEventV(int vKey, int vScancode, int vAction, int vMods);
	virtual void processScrollCallbackEventV(double vX, double vY);
	virtual void processResizeCallbackEventV(int vWidth, int vHeight);

private:
	bool __initGLFWWindow();
	void __initCallbackFunc();

	GLFWwindow* m_pGLFWWindow = nullptr;
	std::string m_WindowTitle = {};
	int m_WindowWidth = 0;
	int m_WindowHeight = 0;
	int m_Samples = 0;
	bool m_IsFullScreen = false;
	bool m_IsVisible = true;
	unsigned int m_GLMajor = 4, m_GLMinor = 3;
};

NAMESPACE_END(gl_kernel)