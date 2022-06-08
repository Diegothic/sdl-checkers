#pragma once

#include <algorithm>

#include "Piece.h"

class ManPiece : public Piece
{
public:
	ManPiece(PieceType type) : Piece(type)
	{
	}

	virtual ~ManPiece() override = default;

public:
	virtual void update(float deltaTime) override
	{
		Piece::update(deltaTime);
	}

	virtual void render(const Renderer& renderer) const override
	{
		const std::vector<Triangle> mesh = renderer.getMeshPrototypes()
		                                           .getPiece(getType(), getState());

		renderer.drawTriangles(mesh, getTransform());
	}

	virtual void recalculateMoves(
		const glm::ivec2& coords,
		Piece*** board,
		int boardSize
	) override
	{
		m_viableMoves.clear();
		m_capturesCount = 0;
		const int forward = getType() == PieceType::Light ? -1 : 1;

		calculateDiagonal(coords, board, boardSize, forward, 1, 100);
		calculateDiagonal(coords, board, boardSize, forward, -1, 100);
		calculateDiagonal(coords, board, boardSize, -forward, 1, 100);
		calculateDiagonal(coords, board, boardSize, -forward, -1, 100);

		if (getCapturesCount() > 0)
		{
			m_viableMoves.erase(
				std::remove_if(
					m_viableMoves.begin(),
					m_viableMoves.end(),
					[](const Move& move) -> bool
					{
						return !move.captures;
					}),
				m_viableMoves.end()
			);
		}
	}

protected:
	void calculateDiagonal(
		const glm::ivec2& coords,
		Piece*** board,
		int boardSize,
		int yStep,
		int xStep,
		int limit
	)
	{
		int check = 0;
		glm::ivec2 checkCoords = coords;
		while (check < limit
			&& checkCoords.x + yStep >= 0
			&& checkCoords.x + yStep < boardSize
			&& checkCoords.y + xStep >= 0
			&& checkCoords.y + xStep < boardSize)
		{
			checkCoords.x += yStep;
			checkCoords.y += xStep;
			if (board[checkCoords.x][checkCoords.y] == nullptr)
			{
				m_viableMoves.push_back({
					checkCoords,
					{},
					false
				});
			}
			else if (board[checkCoords.x][checkCoords.y]->getType() == getType())
			{
				break;
			}
			else if (checkCoords.x + yStep >= 0 && checkCoords.x + yStep < boardSize
				&& checkCoords.y + xStep >= 0 && checkCoords.y + xStep < boardSize
				&& board[checkCoords.x + yStep][checkCoords.y + xStep] == nullptr)
			{
				if (board[checkCoords.x][checkCoords.y]->isCaptured())
				{
					break;
				}
				++m_capturesCount;
				const glm::ivec2 moveCoords = {
					checkCoords.x + yStep,
					checkCoords.y + xStep
				};
				m_viableMoves.push_back({
					moveCoords,
					checkCoords,
					true
				});
				break;
			}
			else
			{
				break;
			}
			check++;
		}
	}
};
