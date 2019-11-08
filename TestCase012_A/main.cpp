#include <iostream>
#include "Viewer.h"
#include "../OpenGLKernel/Interface.h"
#include <crtdbg.h>

//FUNCTION: detect the memory leak in DEBUG mode
void InstallMemoryLeakDetector()
{
	// Enable run-time memory check for debug builds.
#if defined(DEBUG) | defined(_DEBUG)
	//_CRTDBG_LEAK_CHECK_DF: Perform automatic leak checking at program exit through a call to _CrtDumpMemoryLeaks and generate an error 
	//report if the application failed to free all the memory it allocated. OFF: Do not automatically perform leak checking at program exit.
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

	//the following statement is used to trigger a breakpoint when memory leak happens
	//comment it out if there is no memory leak report;
	//_crtBreakAlloc = 755;
#endif
}

int main()
{
	InstallMemoryLeakDetector();

	try
	{
		gl_kernel::init();

		{
			CViewer Viewer("Shadow Map", 800, 800);
			Viewer.setVisible(true);

			gl_kernel::mainloop();
		}

		gl_kernel::shutdown();
	}
	catch (const std::runtime_error& vErr)
	{
		std::string ErrorMsg = std::string("Caught a fatal error: ") + std::string(vErr.what());
		std::cerr << ErrorMsg << std::endl;
		return -1;
	}

	return 0;
}