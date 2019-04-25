#include "Camera.h"

NAMESPACE_BEGIN(gl_kernel)

CCamera::CCamera(glm::vec3 vCameraPos, glm::vec3 vCameraFront, glm::vec3 vCameraUp)
	: m_CameraPos(vCameraPos), m_CameraFront(vCameraFront), m_CameraUp(vCameraUp)
{
}

CCamera::~CCamera()
{
}

//***********************************************************************************************
//Function:
glm::mat4 CCamera::getViewMatrix() const
{
	return glm::lookAt(m_CameraPos, m_CameraPos + m_CameraFront, m_CameraUp);
}

//***********************************************************************************************
//Function:
glm::mat4 CCamera::computeProjectionMatrix(float vAspect) const
{
	return glm::perspective(glm::radians(m_Fov), vAspect, m_Near, m_Far);
}

//***********************************************************************************************
//Function:
void CCamera::processKeyEvent(ECameraMovement vMovementDirection, float vDeltaTime)
{
	if (0.0f == vDeltaTime) vDeltaTime = 0.05f;
	float Velocity = m_MoveSpeed * vDeltaTime;

	switch (vMovementDirection)
	{
	case ECameraMovement::CAMERA_MOVE_FORWARD:	m_CameraPos += m_CameraFront * Velocity; break;
	case ECameraMovement::CAMERA_MOVE_BACKWARD:	m_CameraPos -= m_CameraFront * Velocity; break;
	case ECameraMovement::CAMERA_MOVE_LEFT:		m_CameraPos -= glm::cross(m_CameraFront, m_CameraUp) * Velocity; break;
	case ECameraMovement::CAMERA_MOVE_RIGHT:	m_CameraPos += glm::cross(m_CameraFront, m_CameraUp) * Velocity; break;
	case ECameraMovement::CAMERA_MOVE_UP:		m_CameraPos += m_CameraUp * Velocity; break;
	case ECameraMovement::CAMERA_MOVE_DOWN:		m_CameraPos -= m_CameraUp * Velocity; break;
	default: break;
	}
}

//***********************************************************************************************
//Function:
void CCamera::processCursorMovementEvent(float vXOffset, float vYOffset, bool vConstrainPicth)
{
	if (m_IsCameraMoved)
	{
		vXOffset *= m_MouseSensitivity;
		vYOffset *= m_MouseSensitivity;
		m_Yaw += vXOffset;
		m_Pitch += vYOffset;

		if (vConstrainPicth)
		{
			if (m_Pitch > 89.0f) m_Pitch = 89.0f;
			if (m_Pitch < -89.0f) m_Pitch = -89.0f;
		}

		__updateCamera();
	}
}

//***********************************************************************************************
//Function:
void CCamera::processMouseScrollEvent(float vYOffset)
{
	if (m_Fov >= 1.0f && m_Fov <= 45.0f)
		m_Fov -= vYOffset;

	if (m_Fov <= 1.0f) m_Fov = 1.0f;
	if (m_Fov >= 45.0f) m_Fov = 45.0f;
}

//***********************************************************************************************
//Function:
void CCamera::__updateCamera()
{
	glm::vec3 TempFront;
	TempFront.x = cos(glm::radians(m_Yaw)) * cos(glm::radians(m_Pitch));
	TempFront.y = sin(glm::radians(m_Pitch));
	TempFront.z = sin(glm::radians(m_Yaw)) * cos(glm::radians(m_Pitch));

	m_CameraFront = glm::normalize(TempFront);
	m_CameraRight = glm::normalize(glm::cross(m_CameraFront, m_WorldUp));
	m_CameraUp = glm::normalize(glm::cross(m_CameraRight, m_CameraFront));
}

NAMESPACE_END(gl_kernel)