#include <iostream>
#include "Viewer.h"

int main()
{
	CViewer Viewer("Texture");
	while(!Viewer.isWindowShouldClosed())
		Viewer.drawAll();

	return 0;
}