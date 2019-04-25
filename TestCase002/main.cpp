#include <iostream>
#include "Viewer.h"

int main()
{
	CViewer Viewer;
	while(!Viewer.isWindowShouldClosed())
		Viewer.drawAll();

	return 0;
}