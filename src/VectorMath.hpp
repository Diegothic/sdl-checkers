#pragma once

#include <glm/glm.hpp>

class VectorMath
{
private:
	VectorMath() = default;

public:
	static glm::vec3 linePlaneIntersect(
		const glm::vec3& rayVec,
		const glm::vec3& rayPoint,
		const glm::vec3& planeNormal,
		const glm::vec3& planePoint
	)
	{
		const glm::vec3 diff = rayPoint - planePoint;
		const float prod1 = glm::dot(diff, planeNormal);
		const float prod2 = glm::dot(rayVec, planeNormal);
		const float prod3 = prod1 / prod2;
		return rayPoint - rayVec * prod3;
	}
};
