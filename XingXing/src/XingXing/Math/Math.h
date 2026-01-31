#pragma once

#include <glm/glm.hpp>

namespace Hazel::Math {

	bool DecomposeTransform(const glm::mat4& transform, glm::vec3& translation, glm::vec3& rotation, glm::vec3& scale);

	// 3D Math utilities
	inline glm::mat4 CreatePerspectiveProjection(float fov, float aspect, float near, float far)
	{
		return glm::perspective(glm::radians(fov), aspect, near, far);
	}

	inline glm::mat4 CreateLookAt(const glm::vec3& eye, const glm::vec3& target, const glm::vec3& up)
	{
		return glm::lookAt(eye, target, up);
	}

}
