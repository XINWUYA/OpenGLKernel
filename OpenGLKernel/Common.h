#pragma once
#include <string>
#include <algorithm>

#ifndef NAMESPACE_BEGIN
#define NAMESPACE_BEGIN(vName) namespace vName {
#endif

#ifndef NAMESPACE_END
#define NAMESPACE_END(vName) }
#endif

inline std::string convertStr2Lower(std::string vStr)
{
	std::transform(vStr.begin(), vStr.end(), vStr.begin(), ::tolower);
	return vStr;
}