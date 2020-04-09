#include "Viewer.h"
#include <iostream>
#include <functional>
#include <unordered_map>
#include <fstream>

CViewer::CViewer(const std::string& WindowTitle, int vWindowWidth, int vWindowHeight) : CGLScreen(WindowTitle, vWindowWidth, vWindowHeight), m_WindowWidth(vWindowWidth), m_WindowHeight(vWindowHeight)
{
	m_LastMouseX = static_cast<float>(vWindowWidth) / 2.0;
	m_LastMouseY = static_cast<float>(vWindowHeight) / 2.0;

	m_pCamera = std::make_shared<CCamera>(glm::vec3(0.0f, -1.0f, 0.0f));
	m_pGLShader = std::make_shared<CGLShader>();
	m_pGLShader->initFromFiles("model", "shaders/model.vert", "shaders/model.frag");
	m_pModel = std::make_shared<CGLModel>("../ModelSources/sponza/sponza.obj");
	m_pModel->init(*m_pGLShader);

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

	glEnable(GL_DEPTH_TEST);

	glm::mat4 Projection = m_pCamera->computeProjectionMatrix(static_cast<float>(m_WindowWidth) / static_cast<float>(m_WindowHeight));
	glm::mat4 View = m_pCamera->getViewMatrix();
	glm::mat4 Model = glm::mat4(1.0f);
	Model = glm::scale(Model, glm::vec3(0.5f));
	m_pGLShader->bind();
	m_pGLShader->setMat4Uniform("projection", &Projection[0][0]);
	m_pGLShader->setMat4Uniform("view", &View[0][0]);
	m_pGLShader->setMat4Uniform("model", &Model[0][0]);
	m_pGLShader->setFloatUniform("u_CameraPos", m_pCamera->getCameraPos().x, m_pCamera->getCameraPos().y, m_pCamera->getCameraPos().z);
	m_pGLShader->setFloatUniform("u_LightInfo.Position", m_LightPosX, m_LightPosY, m_LightPosZ);
	m_pGLShader->setFloatUniform("u_LightInfo.Color", m_LightColor.r, m_LightColor.g, m_LightColor.b);
	
	m_pModel->draw();

	glDisable(GL_DEPTH_TEST);

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
	if (vAction == GLFW_PRESS)
	{
		if (vButton == GLFW_MOUSE_BUTTON_RIGHT)
			m_pCamera->setMoveState(true);
	}
	else
		m_pCamera->setMoveState(false);
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
	m_pAssistGUI->sliderFloat("LightPosX", m_LightPosX, -10.0f, 10.0f);
	m_pAssistGUI->sliderFloat("LightPosY", m_LightPosY, -10.0f, 10.0f);
	m_pAssistGUI->sliderFloat("LightPosZ", m_LightPosZ, -10.0f, 10.0f);
	m_pAssistGUI->colorEdit3("LightColor", &m_LightColor[0]);

	if (m_FrameDeque.size() >= 60)
		m_FrameDeque.pop_front();
	m_FrameDeque.push_back(__calcullateFPS());
	m_FrameRateSet.clear();
	for (auto Iter = m_FrameDeque.begin(); Iter != m_FrameDeque.end(); ++Iter)
		m_FrameRateSet.push_back(*Iter);

	m_pAssistGUI->plotLines("FrameRates", m_FrameRateSet);
	m_pAssistGUI->combo("SelectCombo", m_LabelSet, m_Selectedlabel);
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
