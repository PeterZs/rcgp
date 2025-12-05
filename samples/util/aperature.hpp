#pragma once

#include <glm/glm.hpp>
#include <glm/ext/matrix_clip_space.hpp>

struct Aperature {
	float aspect = 1.0f;
	float fovy = 45.0f;
	float near = 0.1f;
	float far = 10000.0f;

	glm::mat4 projection_matrix() const {
		// Vulkan's viewport has +Y pointing down; flip the sign to keep CCW winding.
		auto proj = glm::perspectiveRH_ZO(glm::radians(fovy), aspect, near, far);
		proj[1][1] *= -1.0f;
		return proj;
	}
};
