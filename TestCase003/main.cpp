#include <iostream>
#include "Viewer.h"
#include "../OpenGLKernel/Interface.h"
int main()
{
	try
	{
		gl_kernel::init();

		{
			CViewer Viewer("Texture");
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