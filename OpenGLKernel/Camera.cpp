#include "Camera.h"
#include <iostream>

NAMESPACE_BEGIN(gl_kernel)

CCamera::CCamera(glm::vec3 vCameraPos, glm::vec3 vCameraFront, float vNear, float vFar, float vFov)
	: m_CameraPos(vCameraPos), m_CameraFront(vCameraFront), m_Near(vNear), m_Far(vFar), m_Fov(vFov)
{
	m_Pitch = asin(m_CameraFront.y);
	m_Yaw = asin(m_CameraFront.z / cos(m_Pitch));
	m_CameraRight = glm::normalize(glm::cross(m_CameraFront, m_WorldUp));
	m_CameraUp = glm::normalize(glm::cross(m_CameraRight, m_CameraFront));
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
		m_Yaw += glm::radians(vXOffset);
		m_Pitch += glm::radians(vYOffset);

		if (vConstrainPicth)
		{
			if (m_Pitch > 89.0f) m_Pitch = 89.0f;
			if (m_Pitch < -89.0f) m_Pitch = -89.0f;
		}

		glm::vec3 TempFront;
		TempFront.x = cos(m_Yaw) * cos(m_Pitch);
		TempFront.y = sin(m_Pitch);
		TempFront.z = sin(m_Yaw) * cos(m_Pitch);

		m_CameraFront = glm::normalize(TempFront);
		m_CameraRight = glm::normalize(glm::cross(m_CameraFront, m_WorldUp));
		m_CameraUp = glm::normalize(glm::cross(m_CameraRight, m_CameraFront));
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
void CCamera::printCurrentCameraPosition()
{
	std::cout << "[Current Camera Position]: " << m_CameraPos.x << "," << m_CameraPos.y << "," << m_CameraPos.z << std::endl;
}

//***********************************************************************************************
//Function:
void CCamera::printCurrentCameraFront()
{
	std::cout << "[Current Camera Front]: " << m_CameraFront.x << "," << m_CameraFront.y << "," << m_CameraFront.z << std::endl;
}

NAMESPACE_END(gl_kernel)