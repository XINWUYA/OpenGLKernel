#include "GLScreen.h"
#include <iostream>
#include <map>
#include "Camera.h"

NAMESPACE_BEGIN(gl_kernel)

std::map<GLFWwindow*, CGLScreen*> GL_SCREENS;

CGLScreen::CGLScreen(const std::string& vWindowTitle, int vWindowWidth, int vWindowHeight, bool vIsFullScreen, int vSamples, unsigned int vGLMajor, unsigned int vGLMinor)
	: m_WindowTitle(vWindowTitle), m_WindowWidth(vWindowWidth), m_WindowHeight(vWindowHeight), m_Samples(vSamples), m_IsFullScreen(vIsFullScreen), m_GLMajor(vGLMajor), m_GLMinor(vGLMinor)
{
	__initGLFWWindow();
	__initCallbackFunc();
	GL_SCREENS[m_pGLFWWindow] = this;
}

CGLScreen::~CGLScreen()
{
	GL_SCREENS.erase(m_pGLFWWindow);

	if (m_pGLFWWindow)
		glfwDestroyWindow(m_pGLFWWindow);

	glfwTerminate();
}

//***********************************************************************************************
//Function:
void CGLScreen::drawAllV()
{
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

	drawContentsV();

	glfwPollEvents();
	glfwSwapBuffers(m_pGLFWWindow);
}

//***********************************************************************************************
//Function:
bool CGLScreen::isWindowShouldClosed() const
{
	return glfwWindowShouldClose(m_pGLFWWindow);
}

//***********************************************************************************************
//Function:
void CGLScreen::setVisible(bool vIsVisible)
{
	if (m_IsVisible != vIsVisible)
	{
		m_IsVisible = vIsVisible;

		if (vIsVisible)
			glfwShowWindow(m_pGLFWWindow);
		else
			glfwHideWindow(m_pGLFWWindow);
	}
}

//***********************************************************************************************
//Function:
void CGLScreen::processCursorPosCallbackEventV(double vX, double vY)
{	
}

//***********************************************************************************************
//Function:
void CGLScreen::processMouseButtonCallbackEventV(int vButton, int vAction, int vModifiers)
{
}

//***********************************************************************************************
//Function:
void CGLScreen::processKeyCallbackEventV(int vKey, int vScancode, int vAction, int vMods)
{
	if (vAction == GLFW_PRESS)
	{
		if (vKey == GLFW_KEY_ESCAPE) glfwSetWindowShouldClose(m_pGLFWWindow, true);
	}
}

//***********************************************************************************************
//Function:
void CGLScreen::processScrollCallbackEventV(double vX, double vY)
{
}

//***********************************************************************************************
//Function:
void CGLScreen::processResizeCallbackEventV(int vWidth, int vHeight)
{
	glViewport(0, 0, vWidth, vHeight);
}

//***********************************************************************************************
//Function:
bool CGLScreen::__initGLFWWindow()
{
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, m_GLMajor);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, m_GLMinor);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	glfwWindowHint(GLFW_SAMPLES, m_Samples);
	
	if (m_IsFullScreen)
	{
		GLFWmonitor* pMonitor = glfwGetPrimaryMonitor();
		const GLFWvidmode* pMode = glfwGetVideoMode(pMonitor);
		m_pGLFWWindow = glfwCreateWindow(pMode->width, pMode->height, m_WindowTitle.c_str(), pMonitor, nullptr);
	}
	else
		m_pGLFWWindow = glfwCreateWindow(m_WindowWidth, m_WindowHeight, m_WindowTitle.c_str(), nullptr, nullptr);

	if (!m_pGLFWWindow)
	{
		std::cerr << "Could not create an OpenGL context!" << std::endl;
		glfwTerminate();
		return false;
	}

	glfwMakeContextCurrent(m_pGLFWWindow);

	glewExperimental = GL_TRUE;
	if (glewInit() != GLEW_OK)
	{
		std::cerr << "Error::Window:: GLEW Init Failure" << std::endl;
		return false;
	}
	int FramebufferWidth = 0, FramebufferHeight = 0;
	glfwGetFramebufferSize(m_pGLFWWindow, &FramebufferWidth, &FramebufferHeight);
	glViewport(0, 0, FramebufferWidth, FramebufferHeight);
	return false;
}

//***********************************************************************************************
//Function:
void CGLScreen::__initCallbackFunc()
{
	glfwSetCursorPosCallback(m_pGLFWWindow, 
		[](GLFWwindow * vWindow, double vX, double vY)
		{
			auto it = GL_SCREENS.find(vWindow);
			if (it == GL_SCREENS.end()) return;

			CGLScreen* pScreen = it->second;
			pScreen->processCursorPosCallbackEventV(vX, vY);
		}
	);

	glfwSetMouseButtonCallback(m_pGLFWWindow,
		[](GLFWwindow * vWindow, int vButton, int vAction, int vModifiers)
		{
			auto it = GL_SCREENS.find(vWindow);
			if (it == GL_SCREENS.end()) return;

			CGLScreen* pScreen = it->second;
			pScreen->processMouseButtonCallbackEventV(vButton, vAction, vModifiers);
		}
	);

	glfwSetKeyCallback(m_pGLFWWindow,
		[](GLFWwindow * vWindow, int vKey, int vScancode, int vAction, int vMods)
		{
			auto it = GL_SCREENS.find(vWindow);
			if (it == GL_SCREENS.end()) return;

			CGLScreen * pScreen = it->second;
			pScreen->processKeyCallbackEventV(vKey, vScancode, vAction, vMods);
		}
	);

	glfwSetScrollCallback(m_pGLFWWindow,
		[](GLFWwindow * vWindow, double vX, double vY)
		{
			auto it = GL_SCREENS.find(vWindow);
			if (it == GL_SCREENS.end()) return;

			CGLScreen * pScreen = it->second;
			pScreen->processScrollCallbackEventV(vX, vY);
		}
	);

	glfwSetFramebufferSizeCallback(m_pGLFWWindow,
		[](GLFWwindow * vWindow, int vWidth, int vHeight)
		{
			auto it = GL_SCREENS.find(vWindow);
			if (it == GL_SCREENS.end()) return;

			CGLScreen * pScreen = it->second;
			pScreen->processResizeCallbackEventV(vWidth, vHeight);
		}
	);
}

NAMESPACE_END(gl_kernel)