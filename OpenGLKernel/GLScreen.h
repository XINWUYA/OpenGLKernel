#pragma once
#include <string>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "Common.h"

NAMESPACE_BEGIN(gl_kernel)

class CGLScreen
{
public:
	CGLScreen(const std::string& vWindowTitle, int vWindowWidth, int vWindowHeight, bool vIsFullScreen = false, int vSamples = 0, unsigned int vGLMajor = 4, unsigned int vGLMinor = 0);
	virtual ~CGLScreen();

	virtual void drawAll();
	virtual void drawContents() { }

	GLFWwindow* fetchGLFWWindow() const { return m_pGLFWWindow; }
	bool isWindowShouldClosed() const;

	virtual void processCursorPosCallbackEvent(double vX, double vY);
	virtual void processMouseButtonCallbackEvent(int vButton, int vAction, int vModifiers);
	virtual void processKeyCallbackEvent(int vKey, int vScancode, int vAction, int vMods);
	virtual void processScrollCallbackEvent(double vX, double vY);
	virtual void processResizeCallbackEvent(int vWidth, int vHeight);

private:
	bool __initGLFWWindow();
	void __initCallbackFunc();

	GLFWwindow* m_pGLFWWindow = nullptr;
	std::string m_WindowTitle = {};
	int m_WindowWidth = 0;
	int m_WindowHeight = 0;
	int m_Samples = 0;
	bool m_IsFullScreen = false;
	unsigned int m_GLMajor = 4, m_GLMinor = 0;
};

NAMESPACE_END(gl_kernel)