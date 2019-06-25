#include "Shapes.h"

const float PI = 3.14159265359f;

NAMESPACE_BEGIN(gl_kernel)

//***********************************************************************************************
//Function:
void createACube(std::vector<glm::vec3>& voPositionSet, std::vector<glm::vec3>& voNormalSet, std::vector<glm::vec2>& voTextureCoordsSet)
{
	voPositionSet = {
		glm::vec3(-1.0f, -1.0f, -1.0f), glm::vec3( 1.0f, -1.0f, -1.0f), glm::vec3( 1.0f,  1.0f, -1.0f), glm::vec3( 1.0f,  1.0f, -1.0f), glm::vec3(-1.0f,  1.0f, -1.0f), glm::vec3(-1.0f, -1.0f, -1.0f),
		glm::vec3(-1.0f, -1.0f,  1.0f), glm::vec3( 1.0f, -1.0f,  1.0f), glm::vec3( 1.0f,  1.0f,  1.0f), glm::vec3( 1.0f,  1.0f,  1.0f), glm::vec3(-1.0f,  1.0f,  1.0f), glm::vec3(-1.0f, -1.0f,  1.0f),
		glm::vec3(-1.0f,  1.0f,  1.0f), glm::vec3(-1.0f,  1.0f, -1.0f), glm::vec3(-1.0f, -1.0f, -1.0f), glm::vec3(-1.0f, -1.0f, -1.0f), glm::vec3(-1.0f, -1.0f,  1.0f), glm::vec3(-1.0f,  1.0f,  1.0f),
		glm::vec3( 1.0f,  1.0f,  1.0f), glm::vec3( 1.0f,  1.0f, -1.0f), glm::vec3( 1.0f, -1.0f, -1.0f), glm::vec3( 1.0f, -1.0f, -1.0f), glm::vec3( 1.0f, -1.0f,  1.0f), glm::vec3( 1.0f,  1.0f,  1.0f),
		glm::vec3(-1.0f, -1.0f, -1.0f), glm::vec3( 1.0f, -1.0f, -1.0f), glm::vec3( 1.0f, -1.0f,  1.0f), glm::vec3( 1.0f, -1.0f,  1.0f), glm::vec3(-1.0f, -1.0f,  1.0f), glm::vec3(-1.0f, -1.0f, -1.0f),	
		glm::vec3(-1.0f,  1.0f, -1.0f), glm::vec3( 1.0f,  1.0f, -1.0f), glm::vec3( 1.0f,  1.0f,  1.0f), glm::vec3( 1.0f,  1.0f,  1.0f), glm::vec3(-1.0f,  1.0f,  1.0f), glm::vec3(-1.0f,  1.0f, -1.0f)
	};
	voNormalSet = {
		glm::vec3( 0.0f,  0.0f, -1.0f), glm::vec3( 0.0f,  0.0f, -1.0f), glm::vec3( 0.0f,  0.0f, -1.0f), glm::vec3( 0.0f,  0.0f, -1.0f), glm::vec3( 0.0f,  0.0f, -1.0f), glm::vec3( 0.0f,  0.0f, -1.0f),
		glm::vec3( 0.0f,  0.0f,  1.0f), glm::vec3( 0.0f,  0.0f,  1.0f), glm::vec3( 0.0f,  0.0f,  1.0f), glm::vec3( 0.0f,  0.0f,  1.0f), glm::vec3( 0.0f,  0.0f,  1.0f), glm::vec3( 0.0f,  0.0f,  1.0f),
		glm::vec3(-1.0f,  0.0f,  0.0f), glm::vec3(-1.0f,  0.0f,  0.0f), glm::vec3(-1.0f,  0.0f,  0.0f), glm::vec3(-1.0f,  0.0f,  0.0f), glm::vec3(-1.0f,  0.0f,  0.0f), glm::vec3(-1.0f,  0.0f,  0.0f),
		glm::vec3( 1.0f,  0.0f,  0.0f), glm::vec3( 1.0f,  0.0f,  0.0f), glm::vec3( 1.0f,  0.0f,  0.0f), glm::vec3( 1.0f,  0.0f,  0.0f), glm::vec3( 1.0f,  0.0f,  0.0f), glm::vec3( 1.0f,  0.0f,  0.0f),
		glm::vec3( 0.0f, -1.0f,  0.0f), glm::vec3( 0.0f, -1.0f,  0.0f), glm::vec3( 0.0f, -1.0f,  0.0f), glm::vec3( 0.0f, -1.0f,  0.0f), glm::vec3( 0.0f, -1.0f,  0.0f), glm::vec3( 0.0f, -1.0f,  0.0f),		
		glm::vec3( 0.0f,  1.0f,  0.0f), glm::vec3( 0.0f,  1.0f,  0.0f), glm::vec3( 0.0f,  1.0f,  0.0f), glm::vec3( 0.0f,  1.0f,  0.0f), glm::vec3( 0.0f,  1.0f,  0.0f), glm::vec3( 0.0f,  1.0f,  0.0f),
	};
	voTextureCoordsSet = {
		glm::vec2(0.0f,  0.0f), glm::vec2(1.0f,  0.0f), glm::vec2(1.0f,  1.0f), glm::vec2(1.0f,  1.0f), glm::vec2(0.0f,  1.0f), glm::vec2(0.0f,  0.0f),
		glm::vec2(0.0f,  0.0f), glm::vec2(1.0f,  0.0f), glm::vec2(1.0f,  1.0f), glm::vec2(1.0f,  1.0f), glm::vec2(0.0f,  1.0f), glm::vec2(0.0f,  0.0f),
		glm::vec2(1.0f,  0.0f), glm::vec2(1.0f,  1.0f), glm::vec2(0.0f,  1.0f), glm::vec2(0.0f,  1.0f), glm::vec2(0.0f,  0.0f), glm::vec2(1.0f,  0.0f),
		glm::vec2(1.0f,  0.0f), glm::vec2(1.0f,  1.0f), glm::vec2(0.0f,  1.0f), glm::vec2(0.0f,  1.0f), glm::vec2(0.0f,  0.0f), glm::vec2(1.0f,  0.0f),
		glm::vec2(0.0f,  1.0f), glm::vec2(1.0f,  1.0f), glm::vec2(1.0f,  0.0f), glm::vec2(1.0f,  0.0f), glm::vec2(0.0f,  0.0f), glm::vec2(0.0f,  1.0f),
		glm::vec2(0.0f,  1.0f), glm::vec2(1.0f,  1.0f), glm::vec2(1.0f,  0.0f), glm::vec2(1.0f,  0.0f), glm::vec2(0.0f,  0.0f), glm::vec2(0.0f,  1.0f)
	};
}

//***********************************************************************************************
//Function:
void createASphere(std::vector<glm::vec3>& voPositionSet, std::vector<glm::vec3>& voNormalSet, std::vector<glm::vec2>& voTextureCoordsSet, std::vector<unsigned int>& voIndicesSet)
{
	unsigned int XSegments = 128, YSegments = 128;

	for (unsigned int y = 0; y <= YSegments; ++y)
	{
		for (unsigned int x = 0; x <= XSegments; ++x)
		{
			float TempXSegment = float(x) / float(XSegments);
			float TempYSegment = float(y) / float(YSegments);

			float PosX = std::cos(TempXSegment * 2.0f * PI) * std::sin(TempYSegment * PI);
			float PosY = std::cos(TempYSegment * PI);
			float PosZ = std::sin(TempXSegment * 2.0f * PI) * std::sin(TempYSegment * PI);

			voPositionSet.push_back(glm::vec3(PosX, PosY, PosZ));
			voNormalSet.push_back(glm::vec3(PosX, PosY, PosZ));
			voTextureCoordsSet.push_back(glm::vec2(TempXSegment, TempYSegment));
		}
	}

	bool IsOddRow = false;
	for (unsigned int y = 0; y < YSegments; ++y)
	{
		if (!IsOddRow) // even row: y == 2 * n
		{
			for (unsigned int x = 0; x <= XSegments; ++x)
			{
				voIndicesSet.push_back(y * (XSegments + 1) + x);
				voIndicesSet.push_back((y + 1) * (XSegments + 1) + x);
			}
		}
		else
		{
			for (int x = XSegments; x >= 0; --x)//不能用unsigned int，否则会无限循环
			{
				voIndicesSet.push_back((y + 1) * (XSegments + 1) + x);
				voIndicesSet.push_back(y * (XSegments + 1) + x);
			}
		}
		IsOddRow = !IsOddRow;
	}
}

NAMESPACE_END(gl_kernel)