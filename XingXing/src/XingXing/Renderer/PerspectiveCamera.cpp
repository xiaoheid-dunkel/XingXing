#include "hzpch.h"
#include "PerspectiveCamera.h"

#include <glm/gtc/matrix_transform.hpp>

namespace Hazel {

	PerspectiveCamera::PerspectiveCamera(float fov, float aspectRatio, float nearClip, float farClip)
	{
		SetProjection(fov, aspectRatio, nearClip, farClip);
	}

	void PerspectiveCamera::SetProjection(float fov, float aspectRatio, float nearClip, float farClip)
	{
		m_Projection = glm::perspective(glm::radians(fov), aspectRatio, nearClip, farClip);
		m_ViewProjectionMatrix = m_Projection * m_ViewMatrix;
	}

	void PerspectiveCamera::RecalculateViewMatrix()
	{
		// Calculate view matrix from position and rotation
		glm::mat4 transform = glm::translate(glm::mat4(1.0f), m_Position);
		transform = glm::rotate(transform, m_Rotation.y, glm::vec3(0, 1, 0)); // Yaw
		transform = glm::rotate(transform, m_Rotation.x, glm::vec3(1, 0, 0)); // Pitch
		transform = glm::rotate(transform, m_Rotation.z, glm::vec3(0, 0, 1)); // Roll

		m_ViewMatrix = glm::inverse(transform);
		m_ViewProjectionMatrix = m_Projection * m_ViewMatrix;
	}

}
