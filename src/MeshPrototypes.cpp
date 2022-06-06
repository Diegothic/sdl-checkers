#include "MeshPrototypes.h"

MeshPrototypes::MeshPrototypes()
{
	m_generator.setColor(tableTileColorOdd);
	m_pTileOdd = m_generator.makeQuad(1.0f, 1.0f);

	m_generator.setColor(tableTileColorEven);
	m_pTileEven = m_generator.makeQuad(1.0f, 1.0f);

	m_generator.setColor(pawnColorLightNeutral);
	m_pPawnLightNeutral = m_generator.makeCylinder(0.35f, 0.2f);
	m_pPawnLightMovable = m_pPawnLightNeutral;
	for (auto& tri : m_pPawnLightMovable)
	{
		tri.v1.color += pawnColorOffsetLightMovable;
		tri.v2.color += pawnColorOffsetLightMovable;
		tri.v3.color += pawnColorOffsetLightMovable;
	}
	m_pPawnLightSelected = m_pPawnLightNeutral;
	for (auto& tri : m_pPawnLightSelected)
	{
		tri.v1.color += pawnColorOffsetLightSelected;
		tri.v2.color += pawnColorOffsetLightSelected;
		tri.v3.color += pawnColorOffsetLightSelected;
	}
	m_pPawnLightCapture = m_pPawnLightNeutral;
	for (auto& tri : m_pPawnLightCapture)
	{
		tri.v1.color += pawnColorOffsetLightCapture;
		tri.v2.color += pawnColorOffsetLightCapture;
		tri.v3.color += pawnColorOffsetLightCapture;
	}


	m_generator.setColor(pawnColorDarkNeutral);
	m_pPawnDarkNeutral = m_generator.makeCylinder(0.35f, 0.2f);
	m_pPawnDarkMovable = m_pPawnDarkNeutral;
	for (auto& tri : m_pPawnDarkMovable)
	{
		tri.v1.color += pawnColorOffsetDarkMovable;
		tri.v2.color += pawnColorOffsetDarkMovable;
		tri.v3.color += pawnColorOffsetDarkMovable;
	}
	m_pPawnDarkSelected = m_pPawnDarkNeutral;
	for (auto& tri : m_pPawnDarkSelected)
	{
		tri.v1.color += pawnColorOffsetDarkSelected;
		tri.v2.color += pawnColorOffsetDarkSelected;
		tri.v3.color += pawnColorOffsetDarkSelected;
	}
	m_pPawnDarkCapture = m_pPawnDarkNeutral;
	for (auto& tri : m_pPawnDarkCapture)
	{
		tri.v1.color += pawnColorOffsetDarkCapture;
		tri.v2.color += pawnColorOffsetDarkCapture;
		tri.v3.color += pawnColorOffsetDarkCapture;
	}
}
