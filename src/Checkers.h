#pragma once

#include <functional>

#include "Piece.h"
#include "VectorMath.hpp"

class GameWindow;

typedef PieceType Player;

class Checkers
{
	struct Selection
	{
		Piece* piece;
		glm::ivec2 coords;

		static const Selection NONE;

		bool operator==(const Selection& other) const
		{
			return piece == other.piece && coords == other.coords;
		}

		bool operator !=(const Selection& other) const
		{
			return !(*this == other);
		}
	};

	enum GameState
	{
		PlayerMoving,
		ChangingPlayer,
		Ended
	};

public:
	Piece*** board;

	GameState state = GameState::PlayerMoving;
	Player currentPlayer = Player::Light;
	bool capturedThisTurn = false;
	glm::ivec2 capturingPieceCoords;

	Selection selected = Selection::NONE;
	Selection held = Selection::NONE;

public:
	Checkers(GameWindow* window, uint8_t boardSize);
	virtual ~Checkers();

public:
	void reset() const;

	void update(const float& deltaTime);

	void render(const Renderer& renderer) const;

	void forEachPiece(const std::function<void(Piece*)>& func) const;

protected:
	glm::vec3 positionAt(int x, int z) const
	{
		return {
			static_cast<float>(x) - m_boardSize / 2.0f + 0.5f,
			0.0f,
			static_cast<float>(z) - m_boardSize / 2.0f + 0.5f
		};
	}

	static bool isDarkTile(int x, int z)
	{
		return z % 2 == 0 && x % 2 != 0
			|| z % 2 != 0 && x % 2 == 0;
	}

	Selection getSelection() const;

	std::vector<glm::ivec2> getMovablePieces(Player player) const
	{
		if (capturedThisTurn)
		{
			return {capturingPieceCoords};
		}
		const bool playerHasCaptures = hasCaptures(player);
		std::vector<glm::ivec2> movables;
		for (int z = 0; z < m_boardSize; z++)
		{
			for (int x = 0; x < m_boardSize; x++)
			{
				if (board[z][x] != nullptr
					&& board[z][x]->getType() == player
					&& !board[z][x]->getViableMoves().empty()
					&& (!playerHasCaptures
						|| (playerHasCaptures
							&& board[z][x]->getCapturesCount() > 0)))
				{
					movables.push_back({z, x});
				}
			}
		}
		return movables;
	}

	bool hasCaptures(Player player) const
	{
		for (int z = 0; z < m_boardSize; z++)
		{
			for (int x = 0; x < m_boardSize; x++)
			{
				if (board[z][x] != nullptr
					&& board[z][x]->getType() == player
					&& board[z][x]->getCapturesCount() > 0)
				{
					return true;
				}
			}
		}
		return false;
	}

private:
	GameWindow* m_window = nullptr;
	uint8_t m_boardSize = 10;
};
