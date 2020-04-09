#pragma once
#include <GLM/glm.hpp>
#include <GLM/gtc/matrix_transform.hpp>
#include "Common.h"

NAMESPACE_BEGIN(gl_kernel)

enum class OPENGL_KERNEL_EXPORT ECameraMovement
{
	CAMERA_MOVE_FORWARD = 0,
	CAMERA_MOVE_BACKWARD,
	CAMERA_MOVE_LEFT,
	CAMERA_MOVE_RIGHT,
	CAMERA_MOVE_UP,
	CAMERA_MOVE_DOWN
};

class OPENGL_KERNEL_EXPORT CCamera
{
public:
	CCamera(glm::vec3 vCameraPos = glm::vec3(0.0f, 0.0f, 3.0f), glm::vec3 vCameraFront = glm::vec3(0.0f, 0.0f, -1.0f), float vNear = 0.1f, float vFar = 1000.0f, float vFov = 60.0f);
	~CCamera();

	const glm::vec3& getCameraPos() const { return m_CameraPos; }
	float getCameraFov() const { return m_Fov; }
	glm::mat4 getViewMatrix() const;
	
	glm::mat4 computeProjectionMatrix(float vAspect) const;

	void setMoveState(bool vIsMoved) { m_IsCameraMoved = vIsMoved; }

	void setCameraPos(glm::vec3 vCameraPos) { m_CameraPos = vCameraPos; }
	void setCameraFov(float vFov) { m_Fov = vFov; }
	void setCameraFarPlane(float vFar) { m_Far = vFar; }
	void setMoveSpeed(float vMoveSpeed) { m_MoveSpeed = vMoveSpeed; }

	void processKeyEvent(ECameraMovement vMovementDirection, float vDeltaTime);
	void processCursorMovementEvent(float vXOffset, float vYOffset, bool vConstrainPicth = true);
	void processMouseScrollEvent(float vYOffset);

	void printCurrentCameraPosition();
	void printCurrentCameraFront();

private:

	glm::vec3 m_CameraPos{};
	glm::vec3 m_CameraRight{};
	glm::vec3 m_CameraFront{};
	glm::vec3 m_CameraUp{};
	glm::vec3 m_WorldUp = glm::vec3(0.0f, 1.0f, 0.0f);

	float m_Pitch = 0.0f;
	float m_Yaw = 0.0f;
	float m_Fov = 45.0f;
	float m_MoveSpeed = 2.5f;
	float m_MouseSensitivity = 0.03f;
	float m_Near = 0.1f;
	float m_Far = 100.0f;

	bool m_IsCameraMoved = false;
};

NAMESPACE_END(gl_kernel)