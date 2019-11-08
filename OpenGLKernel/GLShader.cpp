#include "GLShader.h"
#include <iostream>
#include <fstream>
#include <sstream>

NAMESPACE_BEGIN(gl_kernel)

CGLShader::CGLShader()
{
}


CGLShader::~CGLShader()
{
	for (auto& Buf : m_BufferObjectsMap)
		glDeleteBuffers(1, &Buf.second.m_ID);
	m_BufferObjectsMap.clear();

	if (m_VertexArrayObject) {
		glDeleteVertexArrays(1, &m_VertexArrayObject);
		m_VertexArrayObject = 0;
	}

	if (m_ShaderProgram) glDeleteProgram(m_ShaderProgram);
}

//***********************************************************************************************
//Function:
bool CGLShader::initFromStr(const std::string& vShaderSig, const std::string& vVertexShaderStr, const std::string& vFragmentShaderStr, const std::string& vGeometryShaderShaderStr, const std::string& vTessellationControlShaderStr, const std::string& vTessellationEvaluationShaderStr)
{
	m_ShaderSig = vShaderSig;
	glGenVertexArrays(1, &m_VertexArrayObject);
	m_ShaderProgram = glCreateProgram();

	std::string DefinesStr;
	for (auto Def : m_PreDefinitionsMap)
		DefinesStr += std::string("#define ") + Def.first + std::string(" ") + Def.second + "\n";

	GLuint VertexShader = __createShader(GL_VERTEX_SHADER, DefinesStr, vVertexShaderStr);
	GLuint FragmentShader = __createShader(GL_FRAGMENT_SHADER, DefinesStr, vFragmentShaderStr);
	GLuint GeometryShader = __createShader(GL_GEOMETRY_SHADER, DefinesStr, vGeometryShaderShaderStr);
	GLuint TessellationControlShader = __createShader(GL_TESS_CONTROL_SHADER, DefinesStr, vTessellationControlShaderStr);
	GLuint TessellationEvaluationShader = __createShader(GL_TESS_EVALUATION_SHADER, DefinesStr, vTessellationEvaluationShaderStr);

	if (!VertexShader || !FragmentShader) return false;
	if (!vGeometryShaderShaderStr.empty() && !GeometryShader) return false;
	if (!vTessellationControlShaderStr.empty() && !TessellationControlShader) return false;
	if (!vTessellationEvaluationShaderStr.empty() && !TessellationEvaluationShader) return false;

	bool IsLinkSuccess = __linkProgram();
	_ASSERT(IsLinkSuccess);

	__deleteShader(VertexShader);
	__deleteShader(FragmentShader);
	__deleteShader(GeometryShader);
	__deleteShader(TessellationControlShader);
	__deleteShader(TessellationEvaluationShader);

	return true;
}

//***********************************************************************************************
//Function:
bool CGLShader::initFromFiles(const std::string& vShaderSig, const std::string& vVertexShaderFileName, const std::string& vFragmentShaderFileName, const std::string& vGeometryShaderShaderFileName, const std::string& vTessellationControlShaderFileName, const std::string& vTessellationEvaluationShaderFileName)
{
	auto convertFile2Str = [](const std::string & vFileName) -> std::string {
		if (vFileName.empty())
			return "";

		std::ifstream InputStream(vFileName);
		std::string TempStr = std::string(std::istreambuf_iterator<char>(InputStream), std::istreambuf_iterator<char>());
		return TempStr;
	};
	return initFromStr(vShaderSig, convertFile2Str(vVertexShaderFileName), convertFile2Str(vFragmentShaderFileName), convertFile2Str(vGeometryShaderShaderFileName), convertFile2Str(vTessellationControlShaderFileName), convertFile2Str(vTessellationEvaluationShaderFileName));
}

//***********************************************************************************************
//Function:
void CGLShader::bind()
{
	glUseProgram(m_ShaderProgram);
	glBindVertexArray(m_VertexArrayObject);
}

//***********************************************************************************************
//Function:
void CGLShader::drawArray(int vType, uint32_t vOffset, uint32_t vCount)
{
	if (0 == vCount) return;

	glDrawArrays(vType, vOffset, vCount);
}

//***********************************************************************************************
//Function:
void CGLShader::drawIndexed(int vType, uint32_t vOffset, uint32_t vCount)
{
	if (0 == vCount) return;
	size_t Offset = vOffset;
	size_t Count = vCount;

	switch (vType)
	{
	case GL_TRIANGLES: Offset *= 3; Count *= 3; break;
	case GL_LINES: Offset *= 2; Count *= 2; break;
	}

	glDrawElements(vType, (GLsizei)Count, GL_UNSIGNED_INT, (const void*)(Offset * sizeof(uint32_t)));
}

//***********************************************************************************************
//Function:
bool CGLShader::setIntUniform(const std::string& vUniformSig, int v0)
{
	GLint Location = __getUniformLocation(vUniformSig);
	if (Location < 0) return false;
	_ASSERT(Location >= 0);
	glUniform1i(Location, v0);
	return true;
}

//***********************************************************************************************
//Function:
bool CGLShader::setIntUniform(const std::string& vUniformSig, int v0, int v1, int v2, int v3) const
{
	GLint Location = __getUniformLocation(vUniformSig);
	if (Location < 0) return false;
	_ASSERT(Location >= 0);
	glUniform4i(Location, v0, v1, v2, v3);
	return true;
}

//***********************************************************************************************
//Function:
bool CGLShader::setFloatUniform(const std::string& vUniformSig, float v0) const
{
	GLint Location = __getUniformLocation(vUniformSig);
	if (Location < 0) return false;
	_ASSERT(Location >= 0);
	glUniform1f(Location, v0);
	return true;
}

//***********************************************************************************************
//Function:
bool CGLShader::setFloatUniform(const std::string& vUniformSig, float v0, float v1) const
{
	GLint Location = __getUniformLocation(vUniformSig);
	if (Location < 0) return false;
	_ASSERT(Location >= 0);
	glUniform2f(Location, v0, v1);
	return true;
}

//***********************************************************************************************
//Function:
bool CGLShader::setFloatUniform(const std::string& vUniformSig, float v0, float v1, float v2) const
{
	GLint Location = __getUniformLocation(vUniformSig);
	if (Location < 0) return false;
	_ASSERT(Location >= 0);
	glUniform3f(Location, v0, v1, v2);
	return true;
}

//***********************************************************************************************
//Function:
bool CGLShader::setFloatUniform(const std::string& vUniformSig, float v0, float v1, float v2, float v3) const
{
	GLint Location = __getUniformLocation(vUniformSig);
	if (Location < 0) return false;
	_ASSERT(Location >= 0);
	glUniform4f(Location, v0, v1, v2, v3);
	return true;
}

//***********************************************************************************************
//Function:
bool CGLShader::setMat4Uniform(const std::string& vUniformSig, const float vMat[16]) const
{
	GLint Location = __getUniformLocation(vUniformSig);
	if (Location < 0) return false;
	_ASSERT(Location >= 0);
	glUniformMatrix4fv(Location, 1, GL_FALSE, vMat);
	return true;
}

//***********************************************************************************************
//Function:
GLuint CGLShader::__createShader(GLuint vType, const std::string& vDefinitionsStr, std::string vShaderStr)
{
	if (vShaderStr.empty()) return GLuint(0);

	if (!vDefinitionsStr.empty())
	{
		if (vShaderStr.length() > 8 && vShaderStr.substr(0, 8) == "#version")
		{
			std::istringstream InStringStream(vShaderStr);
			std::ostringstream OutStringStream;
			std::string LineStr;
			std::getline(InStringStream, LineStr);
			OutStringStream << LineStr << std::endl;
			OutStringStream << vDefinitionsStr << std::endl;
			while (std::getline(InStringStream, LineStr))
				OutStringStream << LineStr << std::endl;

			vShaderStr = OutStringStream.str();
		}
		else
			vShaderStr = vDefinitionsStr + vShaderStr;
	}

	GLuint ShaderId = glCreateShader(vType);
	const char* pShaderStr = vShaderStr.c_str();
	glShaderSource(ShaderId, 1, &pShaderStr, nullptr);

	bool IsCompileSuccess = __compileShader(ShaderId, vType);
	_ASSERT(IsCompileSuccess);

	_ASSERT(m_ShaderProgram);
	glAttachShader(m_ShaderProgram, ShaderId);

	return ShaderId;
}

//***********************************************************************************************
//Function:
GLvoid CGLShader::__deleteShader(GLuint vShaderId) const
{
	if (vShaderId) glDeleteShader(vShaderId);
}

//***********************************************************************************************
//Function:
bool CGLShader::__compileShader(GLint vShaderId, GLuint vType)
{
	_ASSERT(vShaderId);
	glCompileShader(vShaderId);
	const GLint MaxInfoLogLength = 512;
	GLint Status;
	glGetShaderiv(vShaderId, GL_COMPILE_STATUS, &Status);

	if (Status != GL_TRUE) {
#ifdef _DEBUG
		char Buffer[MaxInfoLogLength];
		std::cerr << "Error while compiling ";
		switch (vType)
		{
		case GL_VERTEX_SHADER:			std::cerr << "vertex shader"; break;
		case GL_FRAGMENT_SHADER:		std::cerr << "fragment shader"; break;
		case GL_GEOMETRY_SHADER:		std::cerr << "geometry shader"; break;
		case GL_TESS_CONTROL_SHADER:	std::cerr << "tessellation control shader"; break;
		case GL_TESS_EVALUATION_SHADER: std::cerr << "tessellation evaluation shader"; break;
		default: break;
		}
		std::cerr << " \"" << m_ShaderSig << "\":" << std::endl;
		//std::cerr << vShaderStr << std::endl << std::endl;
		glGetShaderInfoLog(vShaderId, MaxInfoLogLength, nullptr, Buffer);
		std::cerr << "Error: " << std::endl << Buffer << std::endl;
#endif // _DEBUG
		throw std::runtime_error("Shader compilation failed!");
		return false;
	}
	return true;
}

//***********************************************************************************************
//Function:
bool CGLShader::__linkProgram()
{
	_ASSERT(m_ShaderProgram);
	glLinkProgram(m_ShaderProgram);
	const GLint MaxInfoLogLength = 512;
	GLint Status;
	glGetProgramiv(m_ShaderProgram, GL_LINK_STATUS, &Status);

	if (Status != GL_TRUE) {
#ifdef _DEBUG
		char Buffer[MaxInfoLogLength];
		glGetProgramInfoLog(m_ShaderProgram, MaxInfoLogLength, nullptr, Buffer);
		std::cerr << "Linker error (" << m_ShaderSig << "): " << std::endl << Buffer << std::endl;
#endif // _DEBUG
		m_ShaderProgram = 0;
		throw std::runtime_error("Shader linking failed!");
		return false;
	}
	return true;
}

//***********************************************************************************************
//Function:
void CGLShader::__uploadAttrib(const std::string& vAttribName, size_t vSize, int vDim, uint32_t vCompSize, GLuint vGLType, bool vIsIntegral, const void* vData, int vVersion)
{
	int AttribId = 0;
	if (vAttribName != "Indices")
	{
		AttribId = __getAttribLocation(vAttribName);
		if (AttribId < 0) return;
	}

	GLuint BufferId;
	auto it = m_BufferObjectsMap.find(vAttribName);
	if (it != m_BufferObjectsMap.end())
	{
		SBuffer& Buffer = it->second;
		BufferId = it->second.m_ID;
		Buffer.m_Version = vVersion;
		Buffer.m_Size = vSize;
		Buffer.m_CompSize = vCompSize;
	}
	else
	{
		glGenBuffers(1, &BufferId);
		SBuffer Buffer;
		Buffer.m_ID = BufferId;
		Buffer.m_Type = vGLType;
		Buffer.m_Dim = vDim;
		Buffer.m_Version = vVersion;
		Buffer.m_Size = vSize;
		Buffer.m_CompSize = vCompSize;
		m_BufferObjectsMap[vAttribName] = Buffer;
	}

	size_t TotalSize = vSize * (size_t)vCompSize;
	if (vAttribName == "Indices")
	{
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, BufferId);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, TotalSize, vData, GL_DYNAMIC_DRAW);
	}
	else
	{
		glBindBuffer(GL_ARRAY_BUFFER, BufferId);
		glBufferData(GL_ARRAY_BUFFER, TotalSize, vData, GL_DYNAMIC_DRAW);
		if (0 == vSize)
		{
			glDisableVertexAttribArray(AttribId);
		}
		else
		{
			glEnableVertexAttribArray(AttribId);
			glVertexAttribPointer(AttribId, vDim, vGLType, vIsIntegral, 0, 0);
		}
	}
}

//***********************************************************************************************
//Function:
GLint CGLShader::__getAttribLocation(const std::string& vAttribName) const
{
	GLint Location = glGetAttribLocation(m_ShaderProgram, vAttribName.c_str());

#ifdef _DEBUG
	if (-1 == Location)
		std::cerr << m_ShaderSig << ": warning: did not find attrib " << vAttribName << std::endl;
#endif // _DEBUG

	return Location;
}

//***********************************************************************************************
//Function:
GLint CGLShader::__getUniformLocation(const std::string& vUniformName) const
{
	GLint Location = glGetUniformLocation(m_ShaderProgram, vUniformName.c_str());

#ifdef _DEBUG
	if (-1 == Location)
		std::cerr << m_ShaderSig << ": warning: did not find uniform " << vUniformName << std::endl;
#endif // _DEBUG

	return Location;
}

NAMESPACE_END(gl_kernel)