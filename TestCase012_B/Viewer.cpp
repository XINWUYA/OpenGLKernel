#include "Viewer.h"
#include <iostream>
#include <functional>
#include <unordered_map>
#include <fstream>
#include "../OpenGLKernel/Shapes.h"

CViewer::CViewer(const std::string& WindowTitle, int vWindowWidth, int vWindowHeight) : CGLScreen(WindowTitle, vWindowWidth, vWindowHeight), m_WindowWidth(vWindowWidth), m_WindowHeight(vWindowHeight)
{
	m_LastMouseX = static_cast<float>(vWindowWidth) / 2.0;
	m_LastMouseY = static_cast<float>(vWindowHeight) / 2.0;

	m_pAssistGUI = std::make_shared<CAssistGUI>("ControlPlane");
	m_pAssistGUI->initGUI(fetchGLFWWindow());

	m_pCamera = std::make_shared<CCamera>(glm::vec3(0.0f, 0.25f, 1.0f));
	m_pGLShader = std::make_shared<CGLShader>();
	m_pGLShader->initFromFiles("scene", "shaders/scene.vert", "shaders/scene.frag");
	m_pLightShader = std::make_shared<CGLShader>();
	m_pLightShader->initFromFiles("light", "shaders/light.vert", "shaders/light.frag");
	m_pGenerateShadowMapShader = std::make_shared<CGLShader>();
	m_pGenerateShadowMapShader->initFromFiles("generate_shadow_map", "shaders/shadow_map.vert", "shaders/shadow_map.frag");

	m_pModel = std::make_shared<CGLModel>("../ModelSources/SponzaPBR_dds2tga/SponzaPBR.obj");

	//Create a Sphere
	std::vector<glm::vec3> SpherePosSet;
	std::vector<glm::vec3> SphereNormalSet;
	std::vector<glm::vec2> SphereTextureCoordsSet;
	std::vector<unsigned int> SphereIndicesSet;
	gl_kernel::createASphere(SpherePosSet, SphereNormalSet, SphereTextureCoordsSet, SphereIndicesSet);
	m_LightDrawElementsCnt = SphereIndicesSet.size();
	m_pLightShader->bind();
	m_pLightShader->uploadAttrib("Pos", SpherePosSet, 3);
	m_pLightShader->uploadAttrib("Normal", SphereNormalSet, 3);
	m_pLightShader->uploadAttrib("TextureCoord", SphereTextureCoordsSet, 2);
	m_pLightShader->uploadAttrib("Indices", SphereIndicesSet, 1);

	gl_kernel::STexture TextureConfig;
	TextureConfig.m_Width = 1024;
	TextureConfig.m_Height = 1024;
	TextureConfig.m_InternelFormat = GL_DEPTH_COMPONENT;
	TextureConfig.m_ExternalFormat = GL_DEPTH_COMPONENT;
	TextureConfig.m_DataType = GL_FLOAT;
	TextureConfig.m_IsUseMipMap = GL_FALSE;
	TextureConfig.m_Type4MinFilter = GL_NEAREST;
	TextureConfig.m_Type4MagFilter = GL_NEAREST;
	TextureConfig.m_Type4WrapS = GL_CLAMP_TO_BORDER;
	TextureConfig.m_Type4WrapT = GL_CLAMP_TO_BORDER;
	TextureConfig.m_BorderColor = glm::vec4(1.0f);
	TextureConfig.m_AttachmentType = gl_kernel::STexture::ETextureAttachmentType::DEPTH_TEXTURE;
	m_pTexture = std::make_shared<CGLTexture>(TextureConfig);

	m_pShadowMapFBO = std::make_shared<CGLFrameBuffer>();
	m_pShadowMapFBO->init({ &(*m_pTexture) });

	m_pShadowMapFBO->bind();
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);
	m_pShadowMapFBO->release();
}

CViewer::~CViewer()
{
}

//***********************************************************************************************
//Function:
void CViewer::drawContentsV()
{
	glClearColor(0.2f, 0.3f, 0.3f, 1.0f);

	__calculateTime();
	__processInput();
	__setGUIComponents();

	//Draw Shadow Map
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glm::mat4 LightProjectionMat = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, 0.1f, 100.f);
	glm::mat4 LightViewMat = glm::lookAt(glm::vec3(m_LightPosX, m_LightPosY, m_LightPosZ), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	glm::mat4 LightModelMat = glm::mat4(1.0f); 
	m_pGenerateShadowMapShader->bind();
	m_pGenerateShadowMapShader->setMat4Uniform("u_ProjectionMat", &LightProjectionMat[0][0]);
	m_pGenerateShadowMapShader->setMat4Uniform("u_ViewMat", &LightViewMat[0][0]);
	m_pGenerateShadowMapShader->setMat4Uniform("u_ModelMat", &LightModelMat[0][0]);

	glViewport(0, 0, 1024, 1024);
	m_pShadowMapFBO->bind();
	glCullFace(GL_FRONT);
	glClear(GL_DEPTH_BUFFER_BIT);
	//m_pModel->init(*m_pGenerateShadowMapShader);
	m_pModel->draw();
	glCullFace(GL_BACK);
	m_pShadowMapFBO->release();

	//Draw Scene
	glViewport(0, 0, m_WindowWidth, m_WindowHeight);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glm::mat4 ProjectionMat = m_pCamera->computeProjectionMatrix(static_cast<float>(m_WindowWidth) / static_cast<float>(m_WindowHeight));
	glm::mat4 ViewMat = m_pCamera->getViewMatrix();
	glm::mat4 ModelMat = glm::mat4(1.0f);
	m_pGLShader->bind();
	m_pGLShader->setMat4Uniform("model", &ModelMat[0][0]);
	m_pGLShader->setMat4Uniform("projection", &ProjectionMat[0][0]);
	m_pGLShader->setMat4Uniform("view", &ViewMat[0][0]);
	m_pGLShader->setMat4Uniform("u_LightProjectionMat", &LightProjectionMat[0][0]);
	m_pGLShader->setMat4Uniform("u_LightViewMat", &LightViewMat[0][0]);
	m_pGLShader->setFloatUniform("u_LightPos", m_LightPosX, m_LightPosY, m_LightPosZ);
	m_pGLShader->setFloatUniform("u_CameraPos", m_pCamera->getCameraPos().x, m_pCamera->getCameraPos().y, m_pCamera->getCameraPos().z);
	m_pGLShader->setFloatUniform("u_LightColor", m_LightColor.r, m_LightColor.g, m_LightColor.b);
	m_pGLShader->setFloatUniform("u_LightIntensity", m_LightIntensity);
	m_pTexture->bind(m_pTexture->getTextureID());
	m_pGLShader->setIntUniform("u_ShadowMapTex", m_pTexture->getTextureID());
	m_pModel->init(*m_pGLShader);
	m_pModel->draw();

	//Light
	m_pLightShader->bind();
	ModelMat = glm::mat4(1.0f);
	ModelMat = glm::translate(ModelMat, glm::vec3(m_LightPosX, m_LightPosY, m_LightPosZ));
	ModelMat = glm::scale(ModelMat, glm::vec3(0.03f));
	m_pLightShader->setMat4Uniform("u_ProjectionMat", &ProjectionMat[0][0]);
	m_pLightShader->setMat4Uniform("u_ViewMat", &ViewMat[0][0]);
	m_pLightShader->setMat4Uniform("u_ModelMat", &ModelMat[0][0]);
	m_pLightShader->setFloatUniform("u_LightColor", m_LightColor.r, m_LightColor.g, m_LightColor.b);
	m_pLightShader->setFloatUniform("u_LightIntensity", m_LightIntensity);
	m_pLightShader->drawIndexed(GL_TRIANGLE_STRIP, 0, m_LightDrawElementsCnt);
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
		/*if (vButton == GLFW_MOUSE_BUTTON_LEFT)
			m_pCamera->setMoveState(true);*/
	}
	else
		m_pCamera->setMoveState(false);
}

//***********************************************************************************************
//Function:
void CViewer::processKeyCallbackEventV(int vKey, int vScancode, int vAction, int vMods)
{


	//if (vAction == GLFW_PRESS)
	{
		/*switch (vKey)
		{
		case GLFW_KEY_ESCAPE: glfwSetWindowShouldClose(m_pGLFWWindow, true); break;
		case GLFW_KEY_W: m_pCamera->processKeyEvent(gl_kernel::ECameraMovement::CAMERA_MOVE_FORWARD, DeltaTime); break;
		case GLFW_KEY_S: m_pCamera->processKeyEvent(gl_kernel::ECameraMovement::CAMERA_MOVE_BACKWARD, DeltaTime); break;
		case GLFW_KEY_A: m_pCamera->processKeyEvent(gl_kernel::ECameraMovement::CAMERA_MOVE_LEFT, DeltaTime); break;
		case GLFW_KEY_D: m_pCamera->processKeyEvent(gl_kernel::ECameraMovement::CAMERA_MOVE_RIGHT, DeltaTime); break;
		case GLFW_KEY_E: m_pCamera->processKeyEvent(gl_kernel::ECameraMovement::CAMERA_MOVE_UP, DeltaTime); break;
		case GLFW_KEY_Q: m_pCamera->processKeyEvent(gl_kernel::ECameraMovement::CAMERA_MOVE_DOWN, DeltaTime); break;
		default: break;
		}*/
		if (vKey == GLFW_KEY_ESCAPE && vAction == GLFW_PRESS) glfwSetWindowShouldClose(fetchGLFWWindow(), true);
		/*if (vKey == GLFW_KEY_W) m_pCamera->processKeyEvent(gl_kernel::ECameraMovement::CAMERA_MOVE_FORWARD, m_DeltaTime);
		if (vKey == GLFW_KEY_S) m_pCamera->processKeyEvent(gl_kernel::ECameraMovement::CAMERA_MOVE_BACKWARD, m_DeltaTime);
		if (vKey == GLFW_KEY_A) m_pCamera->processKeyEvent(gl_kernel::ECameraMovement::CAMERA_MOVE_LEFT, m_DeltaTime);
		if (vKey == GLFW_KEY_D) m_pCamera->processKeyEvent(gl_kernel::ECameraMovement::CAMERA_MOVE_RIGHT, m_DeltaTime);
		if (vKey == GLFW_KEY_Q) m_pCamera->processKeyEvent(gl_kernel::ECameraMovement::CAMERA_MOVE_UP, m_DeltaTime);
		if (vKey == GLFW_KEY_E) m_pCamera->processKeyEvent(gl_kernel::ECameraMovement::CAMERA_MOVE_DOWN, m_DeltaTime);*/
	}
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
	m_pAssistGUI->sliderFloat("LightIntensity", m_LightIntensity, 0.0f, 100.0f);
	m_pAssistGUI->colorEdit3("LightColor", &m_LightColor[0]);

	if (m_FrameDeque.size() >= 60)
		m_FrameDeque.pop_front();
	m_FrameDeque.push_back(__calcullateFPS());
	m_FrameRateSet.clear();
	for (auto Iter = m_FrameDeque.begin(); Iter != m_FrameDeque.end(); ++Iter)
		m_FrameRateSet.push_back(*Iter);

	m_pAssistGUI->plotLines("FrameRates", m_FrameRateSet);
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