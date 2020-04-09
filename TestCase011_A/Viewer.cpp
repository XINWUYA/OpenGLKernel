#include "Viewer.h"
#include "..//OpenGLKernel/Shapes.h"

CViewer::CViewer(const std::string& WindowTitle, int vWindowWidth, int vWindowHeight) : CGLScreen(WindowTitle, vWindowWidth, vWindowHeight, false, 8), m_WindowWidth(vWindowWidth), m_WindowHeight(vWindowHeight)
{
	m_LastMouseX = static_cast<float>(vWindowWidth) / 2.0;
	m_LastMouseY = static_cast<float>(vWindowHeight) / 2.0;

	m_pCamera = std::make_shared<CCamera>(glm::vec3(0.0f, 3.0f, 20.0f), glm::vec3(0.0f, 0.0f, -1.0f));
	
	m_pLightShader = std::make_shared<CGLShader>();
	m_pLightShader->initFromFiles("area_light", "shaders/area_light.vert", "shaders/area_light.frag");
	m_pGroundShader = std::make_shared<CGLShader>();
	m_pGroundShader->initFromFiles("ground", "shaders/ground.vert", "shaders/ground.frag");
	
	std::vector<glm::vec3> LightQuadPosSet = { m_LightVertexPosSet[0], m_LightVertexPosSet[1], m_LightVertexPosSet[2], m_LightVertexPosSet[0], m_LightVertexPosSet[2], m_LightVertexPosSet[3] };
	m_pLightShader->bind();
	m_pLightShader->uploadAttrib("Pos", LightQuadPosSet, 3);

	//Create a Quad
	std::vector<glm::vec3> QuadPosSet;
	std::vector<glm::vec3> QuadTextureCoordsSet;
	gl_kernel::createAQuad(QuadPosSet, QuadTextureCoordsSet);

	m_pGroundShader->bind();
	m_pGroundShader->uploadAttrib("Pos", QuadPosSet, 3);

	gl_kernel::STexture Texture;
	Texture.m_Type4WrapS = GL_CLAMP_TO_EDGE;
	Texture.m_Type4WrapT = GL_CLAMP_TO_EDGE;
	Texture.m_IsUseMipMap = GL_FALSE;
	std::shared_ptr<CGLTexture> pLTCMatrixTexture = std::make_shared<CGLTexture>("textures/LTC/ltc_mat.dds", Texture);
	std::shared_ptr<CGLTexture> pLTCAmplifierTexture = std::make_shared<CGLTexture>("textures/LTC/ltc_amp.dds", Texture);

	m_pGroundShader->bind();
	pLTCMatrixTexture->bind(pLTCMatrixTexture->getTextureID());
	m_pGroundShader->setIntUniform("u_LTCMatTexture", pLTCMatrixTexture->getTextureID());
	pLTCAmplifierTexture->bind(pLTCAmplifierTexture->getTextureID());
	m_pGroundShader->setIntUniform("u_LTCAmpTexture", pLTCAmplifierTexture->getTextureID());

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
	Model = glm::translate(Model, m_LightTranslatePos);
	Model = glm::scale(Model, glm::vec3(m_LightWidth, m_LightHeight, 1.0f));
	Model = glm::rotate(Model, glm::radians(m_LightRotateAngle.x), glm::vec3(1.0f, 0.0f, 0.0f));
	Model = glm::rotate(Model, glm::radians(m_LightRotateAngle.y), glm::vec3(0.0f, 1.0f, 0.0f));
	Model = glm::rotate(Model, glm::radians(m_LightRotateAngle.z), glm::vec3(0.0f, 0.0f, 1.0f));
	m_pLightShader->bind();
	m_pLightShader->setMat4Uniform("projection", &Projection[0][0]);
	m_pLightShader->setMat4Uniform("view", &View[0][0]);
	m_pLightShader->setMat4Uniform("model", &Model[0][0]);
	m_pLightShader->setFloatUniform("u_LightIntensity", m_LightIntensity);
	m_pLightShader->setFloatUniform("u_LightColor", m_LightColor.x, m_LightColor.y, m_LightColor.z);
	m_pLightShader->drawArray(GL_TRIANGLES, 0, 6);

	std::vector<glm::vec3> CurrentLightVertexPosSet;
	for (unsigned int i = 0; i < m_LightVertexPosSet.size(); ++i)
		CurrentLightVertexPosSet.push_back(glm::vec3(Model * glm::vec4(m_LightVertexPosSet[i], 1.0f)));

	Model = glm::mat4(1.0f);
	Model = glm::translate(Model, glm::vec3(0.0f, -2.0f, 0.0f));
	Model = glm::scale(Model, glm::vec3(10.0f));
	Model = glm::rotate(Model, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	m_pGroundShader->bind();
	m_pGroundShader->setMat4Uniform("projection", &Projection[0][0]);
	m_pGroundShader->setMat4Uniform("view", &View[0][0]);
	m_pGroundShader->setMat4Uniform("model", &Model[0][0]);
	m_pGroundShader->setFloatUniform("u_CameraPos", m_pCamera->getCameraPos().x, m_pCamera->getCameraPos().y, m_pCamera->getCameraPos().z);
	m_pGroundShader->setIntUniform("u_IsTwoSided", m_IsTwoSide);
	m_pGroundShader->setFloatUniform("u_Roughness", m_Roughness);
	m_pGroundShader->setFloatUniform("u_LightIntensity", m_LightIntensity);
	m_pGroundShader->setFloatUniform("u_LightColor", m_LightColor.x, m_LightColor.y, m_LightColor.z);
	for (unsigned int i = 0; i < CurrentLightVertexPosSet.size(); ++i)
		m_pGroundShader->setFloatUniform("u_PolygonalLightVertexPos[" + std::to_string(i) + "]", CurrentLightVertexPosSet[i].x, CurrentLightVertexPosSet[i].y, CurrentLightVertexPosSet[i].z);
	m_pGroundShader->drawArray(GL_TRIANGLES, 0, 6);

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

	m_pAssistGUI->text("Area Light Info: ");
	m_pAssistGUI->colorEdit3("LightColor", &m_LightColor[0]);
	m_pAssistGUI->sliderFloat("Light Width", m_LightWidth, 0.01f, 10.0f);
	m_pAssistGUI->sliderFloat("Light Height", m_LightHeight, 0.01f, 10.0f);
	m_pAssistGUI->sliderFloat("Rotate Angle X", m_LightRotateAngle.x, -180.0f, 180.0f);
	m_pAssistGUI->sliderFloat("Rotate Angle Y", m_LightRotateAngle.y, -180.0f, 180.0f);
	m_pAssistGUI->sliderFloat("Rotate Angle Z", m_LightRotateAngle.z, -180.0f, 180.0f);
	m_pAssistGUI->sliderFloat("Translate Pos X", m_LightTranslatePos.x, -5.0f, 5.0f);
	m_pAssistGUI->sliderFloat("Translate Pos Y", m_LightTranslatePos.y, -5.0f, 5.0f);
	m_pAssistGUI->sliderFloat("Translate Pos Z", m_LightTranslatePos.z, -5.0f, 5.0f);
	m_pAssistGUI->sliderFloat("LightIntensity", m_LightIntensity, 0.0f, 100.0f);
	m_pAssistGUI->checkBox("Is Use Two Side", m_IsTwoSide);
	m_pAssistGUI->text("Ground Info: ");
	m_pAssistGUI->sliderFloat("Roughness", m_Roughness, 0.0f, 1.0f);
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