#include "Interface.h"
#include "GLScreen.h"
#include <iostream>
#include <map>

NAMESPACE_BEGIN(gl_kernel)

static bool IS_MAINLOOP_ACTIVE = false;
extern std::map<GLFWwindow*, CGLScreen*> GL_SCREENS;

//***********************************************************************************************
//Function:
void init()
{
	glfwSetErrorCallback(
		[](int error, const char* descr) {
		if (error == GLFW_NOT_INITIALIZED)
			return; /* Ignore */
		std::cerr << "GLFW error " << error << ": " << descr << std::endl;
	}
	);

	if (!glfwInit())
		throw std::runtime_error("Could not initialize GLFW!");
}

//***********************************************************************************************
//Function:
void mainloop()
{
	if(IS_MAINLOOP_ACTIVE)
		throw std::runtime_error("Main loop is already running!");
	IS_MAINLOOP_ACTIVE = true;

	try
	{
		while (IS_MAINLOOP_ACTIVE)
		{
			int ScreenCnt = 0;
			for (auto& TempScreen : GL_SCREENS)
			{
				CGLScreen* pScreen = TempScreen.second;
				if (!pScreen->isVisible())
					continue;
				else if (glfwWindowShouldClose(pScreen->fetchGLFWWindow()))
				{
					pScreen->setVisible(false);
					continue;
				}

				pScreen->drawAllV();
				++ScreenCnt;
			}

			if (ScreenCnt == 0)
			{
				IS_MAINLOOP_ACTIVE = false;
				break;
			}
		}
	}
	catch (const std::exception& vExc)
	{
		std::cerr << "Caught exception in main loop: " << vExc.what() << std::endl;
		abort();
	}
}

//***********************************************************************************************
//Function:
void shutdown()
{
	glfwTerminate();
}

//***********************************************************************************************
//Function:
void exit()
{
	IS_MAINLOOP_ACTIVE = false;
}

NAMESPACE_END(gl_kernel)