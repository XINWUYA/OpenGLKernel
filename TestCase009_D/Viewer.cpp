#include "Viewer.h"
#include <iostream>
#include <functional>
#include <unordered_map>
#include <fstream>
#include <glm/gtc/type_ptr.hpp>
#include "../OpenGLKernel/Shapes.h"

CViewer::CViewer(const std::string& WindowTitle, int vWindowWidth, int vWindowHeight) : CGLScreen(WindowTitle, vWindowWidth, vWindowHeight, false, 8), m_WindowWidth(vWindowWidth), m_WindowHeight(vWindowHeight)
{
	m_LastMouseX = static_cast<float>(vWindowWidth) / 2.0;
	m_LastMouseY = static_cast<float>(vWindowHeight) / 2.0;
	setWindowIcon("../TextureSources/awesomeface.png");

	m_pCamera = std::make_shared<CCamera>(glm::vec3(0.0f, 0.0f, 0.0f));
	
	m_pDrawCubeMapShader = std::make_shared<CGLShader>();
	m_pDrawCubeMapShader->initFromFiles("draw_cubemap", "shaders/draw_cubemap.vert", "shaders/draw_cubemap.frag");
	m_pHDR2CubeMapShader = std::make_shared<CGLShader>();
	m_pHDR2CubeMapShader->initFromFiles("hdr2cubemap", "shaders/hdr2cubemap.vert", "shaders/hdr2cubemap.frag", "shaders/hdr2cubemap.geom");
	m_pPreComputeIrradianceMapShader = std::make_shared<CGLShader>();
	m_pPreComputeIrradianceMapShader->initFromFiles("precompute_irradiance_map", "shaders/precompute_irradiance_map.vert", "shaders/precompute_irradiance_map.frag", "shaders/precompute_irradiance_map.geom");
	m_pPreFilterEnvironmentMapShader = std::make_shared<CGLShader>();
	m_pPreFilterEnvironmentMapShader->initFromFiles("prefilter_environment_map", "shaders/prefilter_environment_map.vert", "shaders/prefilter_environment_map.frag", "shaders/prefilter_environment_map.geom");
	m_pPreComputeBRDFShader = std::make_shared<CGLShader>();
	m_pPreComputeBRDFShader->initFromFiles("precompute_brdf", "shaders/precompute_brdf.vert", "shaders/precompute_brdf.frag"); 
	m_pDrawModelShader = std::make_shared<CGLShader>();
	m_pDrawModelShader->initFromFiles("draw_model", "shaders/model.vert", "shaders/model.frag");

	//Create a Cube
	std::vector<glm::vec3> CubePosSet;
	std::vector<glm::vec3> CubeNormalSet;
	std::vector<glm::vec2> CubeTextureCoordsSet;
	gl_kernel::createACube(CubePosSet, CubeNormalSet, CubeTextureCoordsSet);
	
	m_pDrawCubeMapShader->bind();
	m_pDrawCubeMapShader->uploadAttrib("Pos", CubePosSet, 3);
	m_pHDR2CubeMapShader->bind();
	m_pHDR2CubeMapShader->uploadAttrib("Pos", CubePosSet, 3);
	m_pPreComputeIrradianceMapShader->bind();
	m_pPreComputeIrradianceMapShader->uploadAttrib("Pos", CubePosSet, 3);
	m_pPreFilterEnvironmentMapShader->bind();
	m_pPreFilterEnvironmentMapShader->uploadAttrib("Pos", CubePosSet, 3);

	//Create a Sphere
	std::vector<glm::vec3> SpherePosSet;
	std::vector<glm::vec3> SphereNormalSet;
	std::vector<glm::vec2> SphereTextureCoordsSet;
	std::vector<unsigned int> SphereIndicesSet;
	gl_kernel::createASphere(SpherePosSet, SphereNormalSet, SphereTextureCoordsSet, SphereIndicesSet);
	m_DrawElementsCnt = SphereIndicesSet.size();

	m_pDrawModelShader->bind();
	m_pDrawModelShader->uploadAttrib("Pos", SpherePosSet, 3);
	m_pDrawModelShader->uploadAttrib("Normal", SphereNormalSet, 3);
	//m_pDrawModelShader->uploadAttrib("TextureCoord", SphereTextureCoordsSet, 2);
	m_pDrawModelShader->uploadAttrib("Indices", SphereIndicesSet, 1);

	//Create a Quad
	std::vector<glm::vec3> QuadPosSet;
	std::vector<glm::vec3> QuadTextureCoordsSet;
	gl_kernel::createAQuad(QuadPosSet, QuadTextureCoordsSet);

	m_pPreComputeBRDFShader->bind();
	m_pPreComputeBRDFShader->uploadAttrib("Pos", QuadPosSet, 3);
	m_pPreComputeBRDFShader->uploadAttrib("TextureCoord", QuadTextureCoordsSet, 3);

	m_pHDRTexture = std::make_shared<CGLTexture>("../TextureSources/HDR/newport_loft.hdr");

	glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

	gl_kernel::STexture CubeMapTexture;
	CubeMapTexture.m_TextureType = gl_kernel::STexture::ETextureType::TEXTURE_CUBE_MAP;
	CubeMapTexture.m_Width = CubeMapTexture.m_Height = 512;
	CubeMapTexture.m_InternelFormat = GL_RGB16F;
	CubeMapTexture.m_ExternalFormat = GL_RGB;
	CubeMapTexture.m_DataType = GL_FLOAT;
	CubeMapTexture.m_Type4WrapS = CubeMapTexture.m_Type4WrapT = CubeMapTexture.m_Type4WrapR = GL_CLAMP_TO_EDGE;
	CubeMapTexture.m_IsUseMipMap = GL_FALSE;
	m_pCubeMapTexture = std::make_shared<CGLTexture>(CubeMapTexture);

	gl_kernel::STexture IrradianceMapTexture;
	IrradianceMapTexture.m_TextureType = gl_kernel::STexture::ETextureType::TEXTURE_CUBE_MAP;
	IrradianceMapTexture.m_Width = IrradianceMapTexture.m_Height = 32;
	IrradianceMapTexture.m_InternelFormat = GL_RGB16F;
	IrradianceMapTexture.m_ExternalFormat = GL_RGB;
	IrradianceMapTexture.m_DataType = GL_FLOAT;
	IrradianceMapTexture.m_Type4WrapS = IrradianceMapTexture.m_Type4WrapT = IrradianceMapTexture.m_Type4WrapR = GL_CLAMP_TO_EDGE;
	IrradianceMapTexture.m_IsUseMipMap = GL_FALSE;
	m_pIrradianceMapTexture = std::make_shared<CGLTexture>(IrradianceMapTexture);

	gl_kernel::STexture EnvironmentMapTexture;
	EnvironmentMapTexture.m_TextureType = gl_kernel::STexture::ETextureType::TEXTURE_CUBE_MAP;
	EnvironmentMapTexture.m_Width = EnvironmentMapTexture.m_Height = 128;
	EnvironmentMapTexture.m_InternelFormat = GL_RGB16F;
	EnvironmentMapTexture.m_ExternalFormat = GL_RGB;
	EnvironmentMapTexture.m_DataType = GL_FLOAT;
	EnvironmentMapTexture.m_Type4WrapS = EnvironmentMapTexture.m_Type4WrapT = EnvironmentMapTexture.m_Type4WrapR = GL_CLAMP_TO_EDGE;
	EnvironmentMapTexture.m_IsUseMipMap = GL_TRUE;
	m_pEnvironmentMapTexture = std::make_shared<CGLTexture>(EnvironmentMapTexture);

	gl_kernel::STexture BRDFTexture;
	BRDFTexture.m_TextureType = gl_kernel::STexture::ETextureType::TEXTURE_2D;
	BRDFTexture.m_Width = BRDFTexture.m_Height = 512;
	BRDFTexture.m_InternelFormat = GL_RG16F;
	BRDFTexture.m_ExternalFormat = GL_RG;
	BRDFTexture.m_DataType = GL_FLOAT;
	BRDFTexture.m_Type4WrapS = BRDFTexture.m_Type4WrapT = GL_CLAMP_TO_EDGE;
	BRDFTexture.m_IsUseMipMap = GL_FALSE;
	m_pBRDFTexture = std::make_shared<CGLTexture>(BRDFTexture);

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

	m_pFrameBuffer = std::make_shared<CGLFrameBuffer>();

	//Convert HDR to CubeMap
	m_pFrameBuffer->init({ &(*m_pCubeMapTexture) });
	m_pFrameBuffer->bind();
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glViewport(0, 0, m_pCubeMapTexture->getTextureWidth(), m_pCubeMapTexture->getTextureWidth());

	glEnable(GL_DEPTH_TEST);

	m_pHDR2CubeMapShader->bind();
	m_pHDRTexture->bind(m_pHDRTexture->getTextureID());
	m_pHDR2CubeMapShader->setIntUniform("u_HDRTexture", m_pHDRTexture->getTextureID());
	m_pHDR2CubeMapShader->setMat4Uniform("u_ProjectionMat", glm::value_ptr(ProjectionMat));
	for (int i = 0; i < 6; ++i)
		m_pHDR2CubeMapShader->setMat4Uniform("u_ViewMat[" + std::to_string(i) + "]", glm::value_ptr(ViewMatArray[i]));
	m_pHDR2CubeMapShader->drawArray(GL_TRIANGLES, 0, 36);
	
	glDisable(GL_DEPTH_TEST);
	m_pFrameBuffer->release();

	//Precompute Iradiance Map
	m_pFrameBuffer->init({ &(*m_pIrradianceMapTexture) });
	m_pFrameBuffer->bind();
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glViewport(0, 0, m_pIrradianceMapTexture->getTextureWidth(), m_pIrradianceMapTexture->getTextureHeight());

	m_pPreComputeIrradianceMapShader->bind();
	m_pCubeMapTexture->bind(m_pCubeMapTexture->getTextureID());
	m_pPreComputeIrradianceMapShader->setIntUniform("u_CubeMapTexture", m_pCubeMapTexture->getTextureID());
	/*m_pHDRTexture->bind(m_pHDRTexture->getTextureID());
	m_pPreComputeIrradianceMapShader->setIntUniform("u_CubeMapTexture", m_pHDRTexture->getTextureID());*/
	m_pPreComputeIrradianceMapShader->setMat4Uniform("u_ProjectionMat", glm::value_ptr(ProjectionMat));
	for (int i = 0; i < 6; ++i)
		m_pPreComputeIrradianceMapShader->setMat4Uniform("u_ViewMat[" + std::to_string(i) + "]", glm::value_ptr(ViewMatArray[i]));
	m_pPreComputeIrradianceMapShader->drawArray(GL_TRIANGLES, 0, 36);
	
	m_pFrameBuffer->release();

	//Prefilter Environment Map
	m_pFrameBuffer->init({ &(*m_pEnvironmentMapTexture) });
	m_pFrameBuffer->bind();
	//glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	//glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	m_pPreFilterEnvironmentMapShader->bind();
	m_pCubeMapTexture->bind(m_pCubeMapTexture->getTextureID());
	m_pPreFilterEnvironmentMapShader->setIntUniform("u_CubeMapTexture", m_pCubeMapTexture->getTextureID());
	/*m_pHDRTexture->bind(m_pHDRTexture->getTextureID());
	m_pPreFilterEnvironmentMapShader->setIntUniform("u_CubeMapTexture", m_pHDRTexture->getTextureID());*/
	m_pPreFilterEnvironmentMapShader->setMat4Uniform("u_ProjectionMat", glm::value_ptr(ProjectionMat));
	for (int i = 0; i < 6; ++i)
		m_pPreFilterEnvironmentMapShader->setMat4Uniform("u_ViewMat[" + std::to_string(i) + "]", glm::value_ptr(ViewMatArray[i]));

	int MaxLevels = 5;
	for (int i = 0; i < MaxLevels; ++i)
	{
		GLuint LevelWidth = m_pEnvironmentMapTexture->getTextureWidth() * std::pow(0.5, i);
		GLuint LevelHeight = m_pEnvironmentMapTexture->getTextureHeight() * std::pow(0.5, i);

		glViewport(0, 0, LevelWidth, LevelHeight);
		float Roughness = float(i) / float(MaxLevels - 1);
		m_pPreFilterEnvironmentMapShader->setFloatUniform("u_Roughness", Roughness);
		glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, m_pEnvironmentMapTexture->getTextureID(), i);
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		m_pPreFilterEnvironmentMapShader->drawArray(GL_TRIANGLES, 0, 36);
	}

	m_pFrameBuffer->release();

	//Precompute BRDF
	m_pFrameBuffer->init({ &(*m_pBRDFTexture) });
	m_pFrameBuffer->bind();
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glViewport(0, 0, m_pBRDFTexture->getTextureWidth(), m_pBRDFTexture->getTextureWidth());

	m_pPreComputeBRDFShader->bind();
	m_pPreComputeBRDFShader->drawArray(GL_TRIANGLES, 0, 6);

	m_pFrameBuffer->release();

	//Configure Needed Resources
	glViewport(0, 0, m_WindowWidth, m_WindowHeight);

	m_pDrawCubeMapShader->bind();
	m_pCubeMapTexture->bind(m_pCubeMapTexture->getTextureID());
	m_pDrawCubeMapShader->setIntUniform("u_CubeMapTexture", m_pCubeMapTexture->getTextureID());
	/*m_pEnvironmentMapTexture->bind(m_pEnvironmentMapTexture->getTextureID());
	m_pDrawCubeMapShader->setIntUniform("u_CubeMapTexture", m_pEnvironmentMapTexture->getTextureID());*/
	/*m_pIrradianceMapTexture->bind(m_pIrradianceMapTexture->getTextureID());
	m_pDrawCubeMapShader->setIntUniform("u_CubeMapTexture", m_pIrradianceMapTexture->getTextureID());*/

	m_pDrawModelShader->bind();
	m_pIrradianceMapTexture->bind(m_pIrradianceMapTexture->getTextureID());
	m_pDrawModelShader->setIntUniform("u_IrradianceMap", m_pIrradianceMapTexture->getTextureID());
	m_pEnvironmentMapTexture->bind(m_pEnvironmentMapTexture->getTextureID());
	m_pDrawModelShader->setIntUniform("u_EnvironmentMap", m_pEnvironmentMapTexture->getTextureID());
	m_pBRDFTexture->bind(m_pBRDFTexture->getTextureID());
	m_pDrawModelShader->setIntUniform("u_BRDFTexture", m_pBRDFTexture->getTextureID());
	
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

	//Pass 0
	glm::mat4 Projection = m_pCamera->computeProjectionMatrix(static_cast<float>(m_WindowWidth) / static_cast<float>(m_WindowHeight));
	glm::mat4 View = m_pCamera->getViewMatrix();
	glm::mat4 Model = glm::mat4(1.0f);
	//Model = glm::rotate(Model, m_RotateSpeed * (float)glfwGetTime(), glm::vec3(1.0f, 0.0f, 0.0f));
	m_pDrawCubeMapShader->bind();
	m_pDrawCubeMapShader->setMat4Uniform("u_ProjectionMat", &Projection[0][0]);
	m_pDrawCubeMapShader->setMat4Uniform("u_ViewMat", &View[0][0]);
	m_pDrawCubeMapShader->setMat4Uniform("u_ModelMat", &Model[0][0]);
	m_pDrawCubeMapShader->drawArray(GL_TRIANGLES, 0, 36);

	//Pass 1
	glEnable(GL_DEPTH_TEST);

	m_pDrawModelShader->bind();
	m_pDrawModelShader->setMat4Uniform("u_ProjectionMat", &Projection[0][0]);
	m_pDrawModelShader->setMat4Uniform("u_ViewMat", &View[0][0]);
	Model = glm::mat4(1.0f); 
	Model = glm::scale(Model, glm::vec3(0.5f));
	Model = glm::translate(Model, glm::vec3(0.0f, 0.0f, -3.0f));
	m_pDrawModelShader->setMat4Uniform("u_ModelMat", &Model[0][0]);
	m_pDrawModelShader->setFloatUniform("u_CameraPos", m_pCamera->getCameraPos().x, m_pCamera->getCameraPos().y, m_pCamera->getCameraPos().z);
	m_pDrawModelShader->setFloatUniform("u_LightInfo.Position", m_LightPosX, m_LightPosY, m_LightPosZ);
	m_pDrawModelShader->setFloatUniform("u_LightInfo.Color", m_LightColor.r, m_LightColor.g, m_LightColor.b);
	m_pDrawModelShader->setFloatUniform("u_LightInfo.Intensity", m_LightIntensity);
	m_pDrawModelShader->setFloatUniform("u_Roughness", m_ModelRoughness);
	m_pDrawModelShader->setFloatUniform("u_Metallic", m_ModelMetallic);
	m_pDrawModelShader->drawIndexed(GL_TRIANGLE_STRIP, 0, m_DrawElementsCnt);

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
	
	if (m_FrameDeque.size() >= 60)
		m_FrameDeque.pop_front();
	m_FrameDeque.push_back(__calcullateFPS());
	m_FrameRateSet.clear();
	for (auto Iter = m_FrameDeque.begin(); Iter != m_FrameDeque.end(); ++Iter)
		m_FrameRateSet.push_back(*Iter);
	m_pAssistGUI->plotLines("FrameRates", m_FrameRateSet);

	m_pAssistGUI->sliderFloat("RotateSpeed", m_RotateSpeed, 0.0f, 10.0f);
	m_pAssistGUI->sliderFloat("LightPosX", m_LightPosX, -10.0f, 10.0f);
	m_pAssistGUI->sliderFloat("LightPosY", m_LightPosY, -10.0f, 10.0f);
	m_pAssistGUI->sliderFloat("LightPosZ", m_LightPosZ, -10.0f, 10.0f);
	m_pAssistGUI->sliderFloat("LightIntensity", m_LightIntensity, 0.0f, 100.0f);
	m_pAssistGUI->sliderFloat("ModelRoughness", m_ModelRoughness, 0.0f, 1.0f);
	m_pAssistGUI->sliderFloat("ModelMetallic", m_ModelMetallic, 0.0f, 1.0f);
	m_pAssistGUI->colorEdit3("LightColor", &m_LightColor[0]);
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
