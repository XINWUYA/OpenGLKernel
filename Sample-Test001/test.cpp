#include "pch.h"
#include "../OpenGLKernel/GLScreen.h"
#include "../OpenGLKernel/GLShader.h"

using namespace gl_kernel;

TEST(TestCaseName, TestName) {
	CGLScreen GLScreen("SampleTest001", 800, 600);
	CGLShader Shader;

	EXPECT_TRUE(Shader.initFromFiles("Test", "E:/Projects/LearnOpenGL/UnitTest001/vertShader.glsl", "E:/Projects/LearnOpenGL/UnitTest001/fragShader.glsl"));
}