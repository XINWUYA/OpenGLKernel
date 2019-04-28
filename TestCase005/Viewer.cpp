#include "Viewer.h"
#include <iostream>
#include <functional>
#include <unordered_map>
#include <fstream>

CViewer::CViewer(const std::string& WindowTitle, int vWindowWidth, int vWindowHeight) : CGLScreen(WindowTitle, vWindowWidth, vWindowHeight), m_WindowWidth(vWindowWidth), m_WindowHeight(vWindowHeight)
{
	m_LastMouseX = static_cast<float>(vWindowWidth) / 2.0;
	m_LastMouseY = static_cast<float>(vWindowHeight) / 2.0;

	m_pCamera = std::make_shared<CCamera>();
	m_pTexture = std::make_shared<CGLTexture>("textures/container2.png");
	m_pGLShader = std::make_shared<CGLShader>();
	m_pGLShader->initFromFiles("triangle", "shaders/triangle.vert", "shaders/triangle.frag");
	
	std::vector<glm::vec3> Vertices = {
		glm::vec3(-0.5f, -0.5f, 0.0f),
		glm::vec3( 0.5f, -0.5f, 0.0f),
		glm::vec3(-0.5f,  0.5f, 0.0f),
		glm::vec3( 0.5f,  0.5f, 0.0f)
	};

	std::vector<glm::ivec3> Indices =
	{
		glm::ivec3(0, 1, 2),
		glm::ivec3(1, 2, 3)
	};

	std::vector<glm::vec2> TextureCoord = {
		glm::vec2(0.0f, 0.0f),
		glm::vec2(1.0f, 0.0f),
		glm::vec2(0.0f, 1.0f),
		glm::vec2(1.0f, 1.0f)
	};

	m_pGLShader->bind();
	m_pGLShader->uploadAttrib("Pos", Vertices, 3);
	m_pGLShader->uploadAttrib("Indices", Indices, 3);
	m_pGLShader->uploadAttrib("TextureCoord", TextureCoord, 2);

	m_pTexture->bind(m_pTexture->getTextureID());
	m_pGLShader->setIntUniform("u_Texture", m_pTexture->getTextureID());
}

CViewer::~CViewer()
{
}

//***********************************************************************************************
//Function:
void CViewer::drawContents()
{
	double CurrentTime = glfwGetTime();
	m_DeltaTime = CurrentTime - m_LastGLFWTime;
	m_LastGLFWTime = CurrentTime;
	__processInput();

	glm::mat4 Projection = m_pCamera->computeProjectionMatrix(static_cast<float>(m_WindowWidth) / static_cast<float>(m_WindowHeight));
	glm::mat4 View = m_pCamera->getViewMatrix();
	glm::mat4 Model;
	m_pGLShader->bind();
	m_pGLShader->setMat4Uniform("projection", &Projection[0][0]);
	m_pGLShader->setMat4Uniform("view", &View[0][0]);
	m_pGLShader->setMat4Uniform("model", &Model[0][0]);
	
	//m_pGLShader->drawArray(GL_TRIANGLES, 0, 6);
	m_pGLShader->drawIndexed(GL_TRIANGLES, 0, 2);
}

//***********************************************************************************************
//Function:
void CViewer::loadModel(const std::string& vFileName)
{
	std::string FileExt;
	if (vFileName.size() > 4)
		FileExt = gl_kernel::convertStr2Lower(vFileName.substr(vFileName.size() - 4));
	if (FileExt == ".obj")
		__loadObj(vFileName);
	else if (FileExt == ".ply")
		__loadPly(vFileName);
	else
		std::cerr << "Error: File with extension" << FileExt << " is not supported. " << std::endl;
}

//***********************************************************************************************
//Function:
void CViewer::processCursorPosCallbackEvent(double vX, double vY)
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
void CViewer::processMouseButtonCallbackEvent(int vButton, int vAction, int vModifiers)
{
	if (vAction == GLFW_PRESS)
	{
		if (vButton == GLFW_MOUSE_BUTTON_LEFT)
			m_pCamera->setMoveState(true);
	}
	else
		m_pCamera->setMoveState(false);
}

//***********************************************************************************************
//Function:
void CViewer::processKeyCallbackEvent(int vKey, int vScancode, int vAction, int vMods)
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
void CViewer::processScrollCallbackEvent(double vX, double vY)
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
void CViewer::__loadObj(const std::string& vFileName)
{
	struct SOBJVertex
	{
		uint32_t p = (uint32_t)-1;
		uint32_t n = (uint32_t)-1;
		uint32_t uv = (uint32_t)-1;

		inline SOBJVertex() {}
		inline SOBJVertex(const std::string& vStr)
		{
			std::vector<std::string> Tokens = gl_kernel::tokenizeStr(vStr, '/', true);
			if (Tokens.size() < 1 || Tokens.size() > 3)
				throw std::runtime_error("Invalid vertex data: \"" + vStr + "\"");
			p = gl_kernel::convertStr2uint32_t(Tokens[0]);

#if 1
			if (Tokens.size() >= 2 && !Tokens[1].empty())
				uv = gl_kernel::convertStr2uint32_t(Tokens[1]);

			if (Tokens.size() >= 3 && !Tokens[2].empty())
				n = gl_kernel::convertStr2uint32_t(Tokens[2]);
#endif
		}
		inline bool operator==(const SOBJVertex& vOBJVertex) const
		{
			return vOBJVertex.p == p && vOBJVertex.n == n && vOBJVertex.uv == uv;
		}
	};

	struct SOBJVertexHash : std::unary_function<SOBJVertex, size_t>
	{
		size_t operator()(const SOBJVertex& vOBJVertex) const
		{
			size_t HashValue = std::hash<uint32_t>()(vOBJVertex.p);
			HashValue = HashValue * 37 + std::hash<uint32_t>()(vOBJVertex.uv);
			HashValue = HashValue * 37 + std::hash<uint32_t>()(vOBJVertex.n);
			return HashValue;
		}
	};

	typedef std::unordered_map<SOBJVertex, uint32_t, SOBJVertexHash> VertexMap;

	std::ifstream FileStream(vFileName);
	if (FileStream.fail())
		throw std::runtime_error("Unable to open OBJ file \"" + vFileName + "\"!");
	std::cout << "Loading \"" << vFileName << "\" .. ";
	std::cout.flush();

	std::vector<glm::vec3>   PositionSet;
	std::vector<glm::vec2>   TexcoordSet;
	std::vector<glm::vec3>   NormalSet;
	std::vector<uint32_t>    IndicesSet;
	std::vector<SOBJVertex>  VerticesSet;
	VertexMap TempVertexMap;

	std::string LineStr;
	while (std::getline(FileStream, LineStr)) {
		std::istringstream LineStream(LineStr);

		std::string prefix;
		LineStream >> prefix;

		if (prefix == "v") {
			glm::vec3 p;
			LineStream >> p.x >> p.y >> p.z;
			PositionSet.push_back(p);
		}
		else if (prefix == "vt") {
			
			glm::vec2 tc;
			LineStream >> tc.x >> tc.y;
			TexcoordSet.push_back(tc);
			
		}
		else if (prefix == "vn") {
			
			glm::vec3 n;
			LineStream >> n.x >> n.y >> n.z;
			NormalSet.push_back(n);
			
		}
		else if (prefix == "f") {
			std::string v1, v2, v3, v4;
			LineStream >> v1 >> v2 >> v3 >> v4;
			SOBJVertex tri[6];
			int nVertices = 3;

			tri[0] = SOBJVertex(v1);
			tri[1] = SOBJVertex(v2);
			tri[2] = SOBJVertex(v3);

			if (!v4.empty()) {
				/* This is a quad, split into two triangles */
				tri[3] = SOBJVertex(v4);
				tri[4] = tri[0];
				tri[5] = tri[2];
				nVertices = 6;
			}
			/* Convert to an indexed vertex list */
			for (int i = 0; i < nVertices; ++i) {
				const SOBJVertex& v = tri[i];
				VertexMap::const_iterator it = TempVertexMap.find(v);
				if (it == TempVertexMap.end()) {
					TempVertexMap[v] = (uint32_t)VerticesSet.size();
					IndicesSet.push_back((uint32_t)VerticesSet.size());
					VerticesSet.push_back(v);
				}
				else
				{
					IndicesSet.push_back(it->second);
				}
			}
		}
	}
}

//***********************************************************************************************
//Function:
void CViewer::__loadPly(const std::string& vFileName)
{
}
