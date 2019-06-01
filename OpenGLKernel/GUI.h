#pragma once
#include "Common.h"

NAMESPACE_BEGIN(gl_kernel)

class IGUI
{
public:
	IGUI(const std::string& vTitle = "");
	virtual ~IGUI();

	virtual void creatGUIV() {}
	virtual void drawGUIV() {}

protected:
	std::string m_Title;
};

NAMESPACE_END(gl_kernel)