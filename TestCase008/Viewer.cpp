#include "Viewer.h"
#include <iostream>
#include <functional>
#include <unordered_map>
#include <fstream>
#include <glm/gtc/type_ptr.hpp>
#include "../OpenGLKernel/Shapes.h"

CViewer::CViewer(const std::string& WindowTitle, int vWindowWidth, int vWindowHeight) : CGLScreen(WindowTitle, vWindowWidth, vWindowHeight, false, 8), m_WindowWidth(vWindowWidth), m_WindowHeight(vWindowHeight)
{
	m_LastMouseX = static_cast<float>(vWindowWidth) / 2.0f;
	m_LastMouseY = static_cast<float>(vWindowHeight) / 2.0f;

	m_pCamera = std::make_shared<CCamera>(glm::vec3(0.0f, 0.0f, 0.0f));
	
	m_pGLShader = std::make_shared<CGLShader>();
	m_pGLShader->initFromFiles("draw_cubemap", "shaders/draw_cubemap.vert", "shaders/draw_cubemap.frag");
	m_pCubeMapShader = std::make_shared<CGLShader>();
	m_pCubeMapShader->initFromFiles("hdr2cubemap", "shaders/hdr2cubemap.vert", "shaders/hdr2cubemap.frag", "shaders/hdr2cubemap.geom");
	
	//Create a Cube
	std::vector<glm::vec3> CubePosSet;
	std::vector<glm::vec3> CubeNormalSet;
	std::vector<glm::vec2> CubeTextureCoordsSet;
	gl_kernel::createACube(CubePosSet, CubeNormalSet, CubeTextureCoordsSet);
	m_DrawElementsCnt = CubePosSet.size();
	
	m_pGLShader->bind();
	m_pGLShader->uploadAttrib("Pos", CubePosSet, 3);

	m_pCubeMapShader->bind();
	m_pCubeMapShader->uploadAttrib("Pos", CubePosSet, 3);

	m_pHDRTexture = std::make_shared<CGLTexture>("../TextureSources/HDR/barcelona_rooftop.hdr");

	gl_kernel::STexture CubeMapTexture;
	CubeMapTexture.m_TextureType = gl_kernel::STexture::ETextureType::TEXTURE_CUBE_MAP;
	CubeMapTexture.m_Width = CubeMapTexture.m_Height = 512;
	CubeMapTexture.m_InternelFormat = GL_RGB16F;
	CubeMapTexture.m_ExternalFormat = GL_RGB;
	CubeMapTexture.m_DataType = GL_FLOAT;
	CubeMapTexture.m_Type4WrapS = CubeMapTexture.m_Type4WrapT = CubeMapTexture.m_Type4WrapR = GL_CLAMP_TO_EDGE;
	m_pCubeMapTexture = std::make_shared<CGLTexture>(CubeMapTexture);
	
	m_pFrameBuffer = std::make_shared<CGLFrameBuffer>();
	m_pFrameBuffer->init({ &(*m_pCubeMapTexture) });

	glm::mat4 ProjectionMat = glm::perspective(glm::radians(90.0f), 1.0f, 0.0f, 10.0f);
	glm::mat4 ViewMatArray[] = 
	{
		glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3( 1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
		glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(-1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
		glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3( 0.0f,  1.0f,  0.0f), glm::vec3(0.0f,  0.0f,  1.0f)),
		glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3( 0.0f, -1.0f,  0.0f), glm::vec3(0.0f,  0.0f, -1.0f)),
		glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3( 0.0f,  0.0f,  1.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
		glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3( 0.0f,  0.0f, -1.0f), glm::vec3(0.0f, -1.0f,  0.0f))
	};

	glViewport(0, 0, 512, 512);
	m_pFrameBuffer->bind();
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);

	m_pCubeMapShader->bind();
	m_pHDRTexture->bind(m_pHDRTexture->getTextureID());
	m_pCubeMapShader->setIntUniform("u_HDRTexture", m_pHDRTexture->getTextureID());
	m_pCubeMapShader->setMat4Uniform("u_ProjectionMat", glm::value_ptr(ProjectionMat));
	for (int i = 0; i < 6; ++i)
		m_pCubeMapShader->setMat4Uniform("u_ViewMat[" + std::to_string(i) + "]", glm::value_ptr(ViewMatArray[i]));
	m_pCubeMapShader->drawArray(GL_TRIANGLES, 0, m_DrawElementsCnt);
	glDisable(GL_DEPTH_TEST);
	m_pFrameBuffer->release();

	glViewport(0, 0, m_WindowWidth, m_WindowHeight);

	m_pGLShader->bind();
	m_pCubeMapTexture->bind(m_pCubeMapTexture->getTextureID());
	m_pGLShader->setIntUniform("u_CubeMapTexture", m_pCubeMapTexture->getTextureID());

	m_pAssistGUI = std::make_shared<CAssistGUI>("ControlPlane");
	m_pAssistGUI->initGUI(fetchGLFWWindow());
}

CViewer::~CViewer()
{
}

//***********************************************************************************************
//Function:
void CViewer::drawContentsV()
{
	glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	__calculateTime();
	__processInput();
	__setGUIComponents();

	glm::mat4 Projection = m_pCamera->computeProjectionMatrix(static_cast<float>(m_WindowWidth) / static_cast<float>(m_WindowHeight));
	glm::mat4 View = m_pCamera->getViewMatrix();
	glm::mat4 Model = glm::mat4(1.0f);
	Model = glm::rotate(Model, m_RotateSpeed * (float)glfwGetTime(), glm::vec3(0.0f, 1.0f, 0.0f));
	m_pGLShader->bind();
	m_pGLShader->setMat4Uniform("u_ProjectionMat", &Projection[0][0]);
	m_pGLShader->setMat4Uniform("u_ViewMat", &View[0][0]);
	m_pGLShader->setMat4Uniform("u_ModelMat", &Model[0][0]);

	m_pGLShader->drawArray(GL_TRIANGLES, 0, m_DrawElementsCnt);

	m_pAssistGUI->drawGUI();
}

//***********************************************************************************************
//Function:
void CViewer::processCursorPosCallbackEventV(double vX, double vY)
{
	if (m_IsFirstMouse)
	{
		m_LastMouseX = vX;
		m_LastMouseY = vY;
		m_IsFirstMouse = false;
	}

	float XOffset = vX - m_LastMouseX;
	float YOffset = m_LastMouseY - vY;

	m_LastMouseX = vX;
	m_LastMouseY = vY;
	m_pCamera->processCursorMovementEvent(XOffset, YOffset);
}

//***********************************************************************************************
//Function:
void CViewer::processMouseButtonCallbackEventV(int vButton, int vAction, int vModifiers)
{
	/*if (vAction == GLFW_PRESS)
	{
		if (vButton == GLFW_MOUSE_BUTTON_LEFT)
			m_pCamera->setMoveState(true);
	}
	else
		m_pCamera->setMoveState(false);*/
}

//***********************************************************************************************
//Function:
void CViewer::processKeyCallbackEventV(int vKey, int vScancode, int vAction, int vMods)
{
	if (vKey == GLFW_KEY_ESCAPE && vAction == GLFW_PRESS) 
		glfwSetWindowShouldClose(fetchGLFWWindow(), true);
}

//***********************************************************************************************
//Function:
void CViewer::processScrollCallbackEventV(double vX, double vY)
{
	m_pCamera->processMouseScrollEvent(vY);
}

//***********************************************************************************************
//Function:
void CViewer::__processInput()
{
	if (glfwGetKey(fetchGLFWWindow(), GLFW_KEY_W) == GLFW_PRESS) m_pCamera->processKeyEvent(gl_kernel::ECameraMovement::CAMERA_MOVE_FORWARD, m_DeltaTime);
	if (glfwGetKey(fetchGLFWWindow(), GLFW_KEY_S) == GLFW_PRESS) m_pCamera->processKeyEvent(gl_kernel::ECameraMovement::CAMERA_MOVE_BACKWARD, m_DeltaTime);
	if (glfwGetKey(fetchGLFWWindow(), GLFW_KEY_A) == GLFW_PRESS) m_pCamera->processKeyEvent(gl_kernel::ECameraMovement::CAMERA_MOVE_LEFT, m_DeltaTime);
	if (glfwGetKey(fetchGLFWWindow(), GLFW_KEY_D) == GLFW_PRESS) m_pCamera->processKeyEvent(gl_kernel::ECameraMovement::CAMERA_MOVE_RIGHT, m_DeltaTime);
	if (glfwGetKey(fetchGLFWWindow(), GLFW_KEY_Q) == GLFW_PRESS) m_pCamera->processKeyEvent(gl_kernel::ECameraMovement::CAMERA_MOVE_UP, m_DeltaTime);
	if (glfwGetKey(fetchGLFWWindow(), GLFW_KEY_E) == GLFW_PRESS) m_pCamera->processKeyEvent(gl_kernel::ECameraMovement::CAMERA_MOVE_DOWN, m_DeltaTime);
}

//***********************************************************************************************
//Function:
void CViewer::__setGUIComponents()
{
	m_pAssistGUI->updateGUI();
	m_pAssistGUI->text("Application average %.3f ms/frame (%.1f FPS)", __calculateFrameRateInMilliSecond(), __calcullateFPS());
	
	if (m_FrameDeque.size() >= 60)
		m_FrameDeque.pop_front();
	m_FrameDeque.push_back(__calcullateFPS());
	m_FrameRateSet.clear();
	for (auto Iter = m_FrameDeque.begin(); Iter != m_FrameDeque.end(); ++Iter)
		m_FrameRateSet.push_back(*Iter);
	m_pAssistGUI->plotLines("FrameRates", m_FrameRateSet);

	m_pAssistGUI->sliderFloat("RotateSpeed", m_RotateSpeed, 0.0f, 10.0f);
}

//***********************************************************************************************
//Function:
void CViewer::__calculateTime()
{
	double CurrentTime = glfwGetTime();
	m_DeltaTime = CurrentTime - m_LastGLFWTime;
	m_LastGLFWTime = CurrentTime;
}

//***********************************************************************************************
//Function:
double CViewer::__calculateFrameRateInMilliSecond()
{
	return m_DeltaTime * 1000;
}

//***********************************************************************************************
//Function:
double CViewer::__calcullateFPS()
{
	return 1.0 / m_DeltaTime;
}
