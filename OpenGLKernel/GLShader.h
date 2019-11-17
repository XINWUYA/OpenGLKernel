#pragma once
#include <string>
#include <map>
#include <vector>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <GLM/glm.hpp>
#include "Common.h"

NAMESPACE_BEGIN(gl_kernel)

template <typename T> struct STypeTraits;
template <> struct STypeTraits<uint32_t>	{ enum { type = GL_UNSIGNED_INT, integral = 1 }; };
template <> struct STypeTraits<uint16_t>	{ enum { type = GL_UNSIGNED_SHORT, integral = 1 }; };
template <> struct STypeTraits<uint8_t>		{ enum { type = GL_UNSIGNED_BYTE, integral = 1 }; };
template <> struct STypeTraits<int32_t>		{ enum { type = GL_INT, integral = 1 }; };
template <> struct STypeTraits<int16_t>		{ enum { type = GL_SHORT, integral = 1 }; };
template <> struct STypeTraits<int8_t>		{ enum { type = GL_BYTE, integral = 1 }; };
template <> struct STypeTraits<double>		{ enum { type = GL_DOUBLE, integral = 0 }; };
template <> struct STypeTraits<float>		{ enum { type = GL_FLOAT, integral = 0 }; };
template <> struct STypeTraits<glm::vec4>	{ enum { type = GL_FLOAT, integral = 0 }; }; 
template <> struct STypeTraits<glm::vec3>	{ enum { type = GL_FLOAT, integral = 0 }; };
template <> struct STypeTraits<glm::vec2>	{ enum { type = GL_FLOAT, integral = 0 }; };
template <> struct STypeTraits<glm::mat4>	{ enum { type = GL_FLOAT, integral = 0 }; };
template <> struct STypeTraits<glm::ivec4>	{ enum { type = GL_INT, integral = 1 }; };
template <> struct STypeTraits<glm::ivec3>	{ enum { type = GL_INT, integral = 1 }; };
template <> struct STypeTraits<glm::ivec2>	{ enum { type = GL_INT, integral = 1 }; };

class OPENGL_KERNEL_EXPORT CGLShader
{
public:
	CGLShader();
	~CGLShader();

	bool initFromStr(const std::string& vShaderSig, const std::string& vVertexShaderStr, const std::string& vFragmentShaderStr,
		const std::string& vGeometryShaderShaderStr = "", const std::string& vTessellationControlShaderStr = "", const std::string& vTessellationEvaluationShaderStr = "");
	bool initFromFiles(const std::string& vShaderSig, const std::string& vVertexShaderFileName, const std::string& vFragmentShaderFileName,
		const std::string& vGeometryShaderShaderFileName = "", const std::string& vTessellationControlShaderFileName = "", const std::string& vTessellationEvaluationShaderFileName = "");

	const std::string& getShaderSig() const { return m_ShaderSig; }
	
	/// 在shader中预定义一些常量，如#define PI 3.1415926
	void define(const std::string& vKeySig, const std::string& vValue) { m_PreDefinitionsMap[vKeySig] = vValue; }
	
	void bind();

	template <typename T>//感觉这种设计不太好
	void uploadAttrib(const std::string& vAttribName, const std::vector<T>& vData, int vDim, int vVersion = -1)
	{
		uint32_t CompSize = sizeof(T);
		GLuint GLType = (GLuint)STypeTraits<T>::type;
		bool IsIntegral = (bool)STypeTraits<T>::integral;
		__uploadAttrib(vAttribName, (uint32_t)vData.size(), vDim, CompSize, GLType, IsIntegral, vData.data());
	}

	/// Draw a sequence of primitives
	void drawArray(int vType, uint32_t vOffset, uint32_t vCount);
	/// Draw a sequence of primitives using a previously uploaded index buffer
	void drawIndexed(int vType, uint32_t vOffset, uint32_t vCount);

	bool setIntUniform(const std::string& vUniformSig, int v0);
	bool setIntUniform(const std::string& vUniformSig, int v0, int v1, int v2, int v3) const;
	bool setFloatUniform(const std::string& vUniformSig, float v0) const;
	bool setFloatUniform(const std::string& vUniformSig, float v0, float v1) const;
	bool setFloatUniform(const std::string& vUniformSig, float v0, float v1, float v2) const;
	bool setFloatUniform(const std::string& vUniformSig, float v0, float v1, float v2, float v3) const;
	bool setMat4Uniform(const std::string& vUniformSig, const float vMat[16]) const;

private:
	GLuint __createShader( GLuint vType, const std::string& vDefinitionsStr, std::string vShaderStr);
	GLvoid __deleteShader(GLuint vShaderId) const;
	bool __compileShader(GLint vShaderId, GLuint vType);
	bool __linkProgram();
	void __uploadAttrib(const std::string& vAttribName, size_t vSize, int vDim, uint32_t vCompSize, GLuint vGLType, bool vIsIntegral, const void* vData, int vVersion = -1);
	GLint __getAttribLocation(const std::string& vAttribName) const;
	GLint __getUniformLocation(const std::string& vUniformName) const;

	struct SBuffer {
		GLuint m_ID;
		GLuint m_Type;
		GLuint m_Dim;
		GLuint m_Size;
		GLuint m_CompSize;
		int	   m_Version;
	};
	std::string m_ShaderSig;
	GLuint m_VertexArrayObject;
	GLuint m_ShaderProgram;
	GLuint m_VertexShader;
	GLuint m_FragmentShader;
	GLuint m_GeometryShader;
	std::map<std::string, SBuffer> m_BufferObjectsMap;
	std::map<std::string, std::string> m_PreDefinitionsMap;
};

NAMESPACE_END(gl_kernel)