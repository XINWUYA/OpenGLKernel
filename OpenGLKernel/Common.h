#pragma once
#include <string>
#include <algorithm>
#include <vector>
#include <sstream>
#include <random>
#include "OPENGL_KERNEL_EXPORT.h"

#ifndef NAMESPACE_BEGIN
#define NAMESPACE_BEGIN(vName) namespace vName {
#endif

#ifndef NAMESPACE_END
#define NAMESPACE_END(vName) }
#endif

NAMESPACE_BEGIN(gl_kernel)

//***********************************************************************************************
//Function:
OPENGL_KERNEL_EXPORT inline std::string convertStr2Lower(std::string vStr)
{
	std::transform(vStr.begin(), vStr.end(), vStr.begin(), ::tolower);
	return vStr;
}

//***********************************************************************************************
//Function:
OPENGL_KERNEL_EXPORT inline uint32_t convertStr2uint32_t(const std::string& vStr)
{
	char* pEndPtr = nullptr;
	uint32_t Result = (uint32_t)std::strtoul(vStr.c_str(), &pEndPtr, 10);
	if (*pEndPtr != '\0')
		throw std::runtime_error("Could not parse unsigned integer \"" + vStr + "\"");
	return Result;
}

//***********************************************************************************************
//Function:
OPENGL_KERNEL_EXPORT inline int32_t convertStr2int32_t(const std::string& vStr)
{
	char* pEndPtr = nullptr;
	int32_t Result = (uint32_t)std::strtol(vStr.c_str(), &pEndPtr, 10);
	if (*pEndPtr != '\0')
		throw std::runtime_error("Could not parse signed integer \"" + vStr + "\"");
	return Result;
}

//***********************************************************************************************
//Function:
OPENGL_KERNEL_EXPORT inline float convertStr2Float(const std::string& vStr)
{
	char* pEndPtr = nullptr;
	float Result = (uint32_t)std::strtof(vStr.c_str(), &pEndPtr);
	if (*pEndPtr != '\0')
		throw std::runtime_error("Could not parse floating point value \"" + vStr + "\"");
	return Result;
}

//***********************************************************************************************
//Function:
OPENGL_KERNEL_EXPORT inline void tokenizeStr(const std::string& vStr, char vDelim, std::vector<std::string>& voElemSet, bool vIsIncludeEmpty = false)
{
	std::stringstream StrStream(vStr);
	std::string Elem;
	while (std::getline(StrStream, Elem, vDelim))
	{
		if (!Elem.empty() || vIsIncludeEmpty)
			voElemSet.emplace_back(Elem);
	}
}

//***********************************************************************************************
//Function:
OPENGL_KERNEL_EXPORT inline std::vector<std::string> tokenizeStr(const std::string& vStr, char vDelim, bool vIsIncludeEmpty)
{
	std::vector<std::string> ElemSet;
	tokenizeStr(vStr, vDelim, ElemSet, vIsIncludeEmpty);
	return ElemSet;
}

//***********************************************************************************************
//Function:
template<typename T = float>
OPENGL_KERNEL_EXPORT T generateRandomValue(std::default_random_engine& vioRangdomEngine, T vMin, T vMax)
{
	std::uniform_real_distribution<T> RandomValue(vMin, vMax);
	return RandomValue(vioRangdomEngine);
}

NAMESPACE_END(gl_kernel)