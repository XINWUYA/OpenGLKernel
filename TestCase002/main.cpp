#include <iostream>
#include "Viewer.h"

int main()
{
	CViewer Viewer("Camera");
	while(!Viewer.isWindowShouldClosed())
		Viewer.drawAll();

	return 0;
}