#pragma once

#include <algorithm>

#include "Piece.h"

class ManPiece : public Piece
{
public:
	ManPiece(PieceType type) : Piece(type)
	{
	}

	ManPiece(const Piece& other) : Piece(other)
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
		Piece::recalculateMoves(coords, board, boardSize);
		const int forward = getType() == PieceType::Light ? -1 : 1;

		calculateMovesDiagonal(coords, board, boardSize, forward, 1, 1);
		calculateMovesDiagonal(coords, board, boardSize, forward, -1, 1);

		calculateCapturesDiagonal(coords, board, boardSize, forward, 1, 1);
		calculateCapturesDiagonal(coords, board, boardSize, forward, -1, 1);
		calculateCapturesDiagonal(coords, board, boardSize, -forward, 1, 1);
		calculateCapturesDiagonal(coords, board, boardSize, -forward, -1, 1);

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
	void calculateMovesDiagonal(
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
			else
			{
				break;
			}
			check++;
		}
	}

	void calculateCapturesDiagonal(
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
			if (board[checkCoords.x][checkCoords.y] != nullptr)
			{
				if (board[checkCoords.x][checkCoords.y]->getType() != getType()
					&& !board[checkCoords.x][checkCoords.y]->isCaptured()
					&& checkCoords.x + yStep >= 0 && checkCoords.x + yStep < boardSize
					&& checkCoords.y + xStep >= 0 && checkCoords.y + xStep < boardSize
					&& board[checkCoords.x + yStep][checkCoords.y + xStep] == nullptr)
				{
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
				}
				break;
			}
			check++;
		}
	}
};
