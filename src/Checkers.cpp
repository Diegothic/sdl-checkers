#include "Checkers.h"

#include "GameWindow.h"
#include "ManPiece.h"

const Checkers::Selection Checkers::Selection::NONE = {nullptr, {-1.0f, -1.0f}};

Checkers::Checkers(GameWindow* window, uint8_t boardSize)
	: m_window(window),
	  m_boardSize(boardSize)
{
	board = new Piece* *[m_boardSize];
	for (int i = 0; i < m_boardSize; i++)
	{
		board[i] = new Piece*[m_boardSize];
		for (int j = 0; j < m_boardSize; j++)
		{
			board[i][j] = nullptr;
		}
	}
}

Checkers::~Checkers()
{
	for (int i = 0; i < m_boardSize; i++)
	{
		for (int j = 0; j < m_boardSize; j++)
		{
			if (board[i][j] != nullptr)
			{
				delete board[i][j];
				board[i][j] = nullptr;
			}
		}
		delete[] board[i];
	}
	delete[] board;
}

void Checkers::reset() const
{
	for (int z = 0; z < m_boardSize; z++)
	{
		for (int x = 0; x < m_boardSize; x++)
		{
			if (board[z][x] != nullptr)
			{
				delete board[z][x];
				board[z][x] = nullptr;
			}

			const glm::vec3 pos = positionAt(x, z);
			if (isDarkTile(x, z))
			{
				if (z < m_boardSize / 2 - 1)
				{
					board[z][x] = new ManPiece(PieceType::Dark);
					board[z][x]->setDesiredPosition(pos);
					board[z][x]->setInterpolationSpeed(5.0f);
				}
				else if (z > m_boardSize / 2)
				{
					board[z][x] = new ManPiece(PieceType::Light);
					board[z][x]->setDesiredPosition(pos);
					board[z][x]->setInterpolationSpeed(5.0f);
				}
			}
		}
	}
}

void Checkers::update(const float& deltaTime)
{
	const glm::vec2 mousePos = m_window->getMousePosition();
	const glm::vec3 intersectUp = VectorMath::linePlaneIntersect(
		m_window->getCamera().forwardVec(mousePos),
		m_window->getCamera().unProject(mousePos),
		{0.0f, 1.0f, 0.0f},
		{0.0f, 1.5f, 0.0f}
	);

	selected = getSelection();

	forEachPiece([](Piece* const piece)
	{
		piece->setState(PieceState::Neutral);
		piece->setMovable(false);
		if (piece->isCaptured())
			piece->setState(PieceState::Capture);
	});

	for (int z = 0; z < m_boardSize; z++)
	{
		for (int x = 0; x < m_boardSize; x++)
		{
			if (board[z][x] != nullptr)
				board[z][x]->recalculateMoves(
					{z, x},
					board,
					m_boardSize
				);
		}
	}

	if (held == Selection::NONE)
	{
		const auto movables = getMovablePieces(currentPlayer);
		for (const auto& movable : movables)
		{
			board[movable.x][movable.y]->setState(PieceState::Movable);
			board[movable.x][movable.y]->setMovable(true);
		}
	}

	if (held != Selection::NONE)
	{
		held.piece->setState(PieceState::Selected);
		for (const auto& move : held.piece->getViableMoves())
		{
			if (move.captures)
			{
				board[move.capture.x][move.capture.y]->setState(PieceState::Capture);
			}
		}
	}
	else if (selected != Selection::NONE
		&& board[selected.coords.x][selected.coords.y] != nullptr)
	{
		board[selected.coords.x][selected.coords.y]->setState(PieceState::Selected);
	}

	if (m_window->isMouseButtonPressed(SDL_BUTTON_RIGHT)
		&& held != Selection::NONE)
	{
		held.piece->setHeld(false);
		held.piece->setDesiredPosition(positionAt(held.coords.y, held.coords.x));
		held = Selection::NONE;
	}

	if (held != Selection::NONE)
	{
		held.piece->setDesiredPosition(intersectUp);

		const auto possibleMove = std::find(
			held.piece->getViableMoves().begin(),
			held.piece->getViableMoves().end(),
			glm::ivec2(selected.coords.x, selected.coords.y)
		);

		if (m_window->isMouseButtonPressed(SDL_BUTTON_LEFT)
			&& selected != Selection::NONE
			&& possibleMove != held.piece->getViableMoves().end())
		{
			// Player made move
			held.piece->setHeld(false);
			held.piece->setDesiredPosition(
				positionAt(selected.coords.y, selected.coords.x)
			);

			board[held.coords.x][held.coords.y] = nullptr;
			board[selected.coords.x][selected.coords.y] = held.piece;

			bool captured = false;
			//End move or continue if captures available
			if (possibleMove->captures)
			{
				board[possibleMove->capture.x][possibleMove->capture.y]->setCaptured(true);
				captured = true;
				held.piece->recalculateMoves(selected.coords, board, m_boardSize);
				capturedThisTurn = true;
				capturingPieceCoords = selected.coords;
			}

			if (!(captured && held.piece->getCapturesCount() > 0))
			{
				//End move, remove captured pieces and change player
				currentPlayer = currentPlayer == Player::Light
					                ? Player::Dark
					                : Player::Light;
				capturedThisTurn = false;

				for (int z = 0; z < m_boardSize; z++)
				{
					for (int x = 0; x < m_boardSize; x++)
					{
						if (board[z][x] != nullptr
							&& board[z][x]->isCaptured())
						{
							delete board[z][x];
							board[z][x] = nullptr;
						}
					}
				}
			}

			held = Selection::NONE;
		}
	}
	else if (m_window->isMouseButtonPressed(SDL_BUTTON_LEFT)
		&& selected != Selection::NONE
		&& (board[selected.coords.x][selected.coords.y] != nullptr
			&& board[selected.coords.x][selected.coords.y]->isMovable()))
	{
		held = selected;
		held.piece->setHeld(true);
	}

	forEachPiece([&deltaTime](Piece* const piece)
	{
		piece->update(deltaTime);
	});
}

void Checkers::render(const Renderer& renderer) const
{
	forEachPiece([&renderer](const Piece* const piece)
	{
		piece->render(renderer);
	});

	if (held != Selection::NONE)
	{
		for (const auto& move : held.piece->getViableMoves())
		{
			Transform transform = {};
			transform.position = positionAt(move.destination.y, move.destination.x);
			transform.scale = glm::vec3(0.5f);
			renderer.drawTriangles(
				currentPlayer == Player::Light
					? m_window->getPrototypes().getPieceLightMovable()
					: m_window->getPrototypes().getPieceDarkMovable(),
				transform
			);
		}
	}

	Transform tileTransform = {};
	for (int z = 0; z < m_boardSize; z++)
	{
		for (int x = 0; x < m_boardSize; x++)
		{
			const glm::vec3 pos = positionAt(x, z);
			tileTransform.position = pos;
			tileTransform.position.y = -0.1f;
			if (isDarkTile(x, z))
			{
				renderer.drawTriangles(
					m_window->getPrototypes().getTileEven(),
					tileTransform
				);
			}
			else
			{
				renderer.drawTriangles(
					m_window->getPrototypes().getTileOdd(),
					tileTransform
				);
			}
		}
	}
}

void Checkers::forEachPiece(const std::function<void(Piece*)>& func) const
{
	for (int z = 0; z < m_boardSize; z++)
	{
		for (int x = 0; x < m_boardSize; x++)
		{
			if (board[z][x] != nullptr)
				func(board[z][x]);
		}
	}
}

Checkers::Selection Checkers::getSelection() const
{
	const glm::vec2 mousePos = m_window->getMousePosition();
	glm::vec3 intersect = VectorMath::linePlaneIntersect(
		m_window->getCamera().forwardVec(mousePos),
		m_window->getCamera().unProject(mousePos),
		{0.0f, 1.0f, 0.0f},
		{0.0f, 0.0f, 0.0f}
	);
	intersect.x += 5.0f;
	intersect.z += 5.0f;
	if (intersect.x >= 0.0f
		&& intersect.x < 10.0f
		&& intersect.z >= 0.0f
		&& intersect.z <= 10.0f)
	{
		const glm::ivec2 selectedCoords = {
			static_cast<int>(intersect.z),
			static_cast<int>(intersect.x)
		};
		return {
			board[selectedCoords.x][selectedCoords.y],
			selectedCoords
		};
	}
	else
	{
		return Selection::NONE;
	}
}
