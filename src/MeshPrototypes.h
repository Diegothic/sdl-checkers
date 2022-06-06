#pragma once

#include <vector>

#include <glm/glm.hpp>

#include "MeshGenerator.h"

class MeshPrototypes
{
public:
	MeshPrototypes();

public:
	const std::vector<Triangle>& getTileOdd() const { return m_pTileOdd; }
	const std::vector<Triangle>& getTileEven() const { return m_pTileEven; }

	const std::vector<Triangle>& getPawnLightNeutral() const { return m_pPawnLightNeutral; }
	const std::vector<Triangle>& getPawnLightMovable() const { return m_pPawnLightMovable; }
	const std::vector<Triangle>& getPawnLightSelected() const { return m_pPawnLightSelected; }
	const std::vector<Triangle>& getPawnLightCapture() const { return m_pPawnLightCapture; }

	const std::vector<Triangle>& getPawnDarkNeutral() const { return m_pPawnDarkNeutral; }
	const std::vector<Triangle>& getPawnDarkMovable() const { return m_pPawnDarkMovable; }
	const std::vector<Triangle>& getPawnDarkSelected() const { return m_pPawnDarkSelected; }
	const std::vector<Triangle>& getPawnDarkCapture() const { return m_pPawnDarkCapture; }

protected:
	static constexpr glm::vec3 tableTileColorOdd = {0.85f, 0.75f, 0.5f};
	static constexpr glm::vec3 tableTileColorEven = {0.29f, 0.24f, 0.21f};

	static constexpr glm::vec3 pawnColorLightNeutral = {0.9f, 0.9f, 0.9f};
	static constexpr glm::vec3 pawnColorOffsetLightMovable = {-0.15f, -0.15f, 0.1f};
	static constexpr glm::vec3 pawnColorOffsetLightSelected = {0.1f, 0.1f, 0.1f};
	static constexpr glm::vec3 pawnColorOffsetLightCapture = {0.1f, -0.45f, -0.4f};

	static constexpr glm::vec3 pawnColorDarkNeutral = {0.2f, 0.2f, 0.2f};
	static constexpr glm::vec3 pawnColorOffsetDarkMovable = {-0.02f, -0.02f, 0.23f};
	static constexpr glm::vec3 pawnColorOffsetDarkSelected = {0.15f, 0.15f, 0.15f};
	static constexpr glm::vec3 pawnColorOffsetDarkCapture = {0.23f, -0.02f, -0.02f};


private:
	MeshGenerator m_generator;

	std::vector<Triangle> m_pTileOdd;
	std::vector<Triangle> m_pTileEven;

	std::vector<Triangle> m_pPawnLightNeutral;
	std::vector<Triangle> m_pPawnLightMovable;
	std::vector<Triangle> m_pPawnLightSelected;
	std::vector<Triangle> m_pPawnLightCapture;

	std::vector<Triangle> m_pPawnDarkNeutral;
	std::vector<Triangle> m_pPawnDarkMovable;
	std::vector<Triangle> m_pPawnDarkSelected;
	std::vector<Triangle> m_pPawnDarkCapture;
};
