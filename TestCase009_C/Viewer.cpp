#include "Viewer.h"
#include "..//OpenGLKernel/Shapes.h"

CViewer::CViewer(const std::string& WindowTitle, int vWindowWidth, int vWindowHeight) : CGLScreen(WindowTitle, vWindowWidth, vWindowHeight), m_WindowWidth(vWindowWidth), m_WindowHeight(vWindowHeight)
{
	m_LastMouseX = static_cast<float>(vWindowWidth) / 2.0;
	m_LastMouseY = static_cast<float>(vWindowHeight) / 2.0;

	m_pCamera = std::make_shared<CCamera>();

	m_pGLShader = std::make_shared<CGLShader>();
	m_pGLShader->initFromFiles("show_brdf", "shaders/show_brdf.vert", "shaders/show_brdf.frag");
	m_pPreComputeBRDFShader = std::make_shared<CGLShader>();
	m_pPreComputeBRDFShader->initFromFiles("precompute_brdf", "shaders/precompute_brdf.vert", "shaders/precompute_brdf.frag");
	
	std::vector<glm::vec3> Vertices;
	std::vector<glm::vec3> TextureCoord;
	gl_kernel::createAQuad(Vertices, TextureCoord);

	m_pGLShader->bind();
	m_pGLShader->uploadAttrib("Pos", Vertices, 3);
	m_pGLShader->uploadAttrib("TextureCoord", TextureCoord, 3);

	m_pPreComputeBRDFShader->bind();
	m_pPreComputeBRDFShader->uploadAttrib("Pos", Vertices, 3);
	m_pPreComputeBRDFShader->uploadAttrib("TextureCoord", TextureCoord, 3);

	gl_kernel::STexture BRDFTexture;
	BRDFTexture.m_TextureType = gl_kernel::STexture::ETextureType::TEXTURE_2D;
	BRDFTexture.m_Width = BRDFTexture.m_Height = 512;
	BRDFTexture.m_InternelFormat = GL_RG16F;
	BRDFTexture.m_ExternalFormat = GL_RG;
	BRDFTexture.m_DataType = GL_FLOAT;
	BRDFTexture.m_IsUseMipMap = GL_FALSE;
	m_pTexture = std::make_shared<CGLTexture>(BRDFTexture);

	m_pFrameBuffer = std::make_shared<CGLFrameBuffer>();
	m_pFrameBuffer->init({ &(*m_pTexture) });

	m_pFrameBuffer->bind();
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glViewport(0, 0, m_pTexture->getTextureWidth(), m_pTexture->getTextureWidth());
	m_pPreComputeBRDFShader->bind();
	m_pPreComputeBRDFShader->drawArray(GL_TRIANGLES, 0, 6);

	m_pFrameBuffer->release();

	glViewport(0, 0, m_WindowWidth, m_WindowHeight);

	m_pGLShader->bind();
	m_pTexture->bind(m_pTexture->getTextureID());
	m_pGLShader->setIntUniform("u_Texture", m_pTexture->getTextureID());
}

CViewer::~CViewer()
{
}

//***********************************************************************************************
//Function:
void CViewer::drawContentsV()
{
	double CurrentTime = glfwGetTime();
	m_DeltaTime = CurrentTime - m_LastGLFWTime;
	m_LastGLFWTime = CurrentTime;
	__processInput();

	glm::mat4 Projection = m_pCamera->computeProjectionMatrix(static_cast<float>(m_WindowWidth) / static_cast<float>(m_WindowHeight));
	glm::mat4 View = m_pCamera->getViewMatrix();
	glm::mat4 Model = glm::mat4(1.0f);
	m_pGLShader->bind();
	m_pGLShader->setMat4Uniform("projection", &Projection[0][0]);
	m_pGLShader->setMat4Uniform("view", &View[0][0]);
	m_pGLShader->setMat4Uniform("model", &Model[0][0]);
	m_pGLShader->drawArray(GL_TRIANGLES, 0, 6);
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