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
	});

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

	if (m_window->isMouseButtonPressed(SDL_BUTTON_LEFT)
		&& held != Selection::NONE
		&& selected != Selection::NONE
		&& (board[selected.coords.x][selected.coords.y] == nullptr
			|| board[selected.coords.x][selected.coords.y] == held.piece)
		&& isDarkTile(selected.coords.y, selected.coords.x))
	{
		held.piece->setHeld(false);
		held.piece->setDesiredPosition(
			positionAt(selected.coords.y, selected.coords.x)
		);

		board[held.coords.x][held.coords.y] = nullptr;
		board[selected.coords.x][selected.coords.y] = held.piece;
		held = Selection::NONE;
	}
	else if (m_window->isMouseButtonPressed(SDL_BUTTON_LEFT)
		&& selected != Selection::NONE
		&& held == Selection::NONE
		&& (board[selected.coords.x][selected.coords.y] != nullptr
			&& board[selected.coords.x][selected.coords.y]->isMovable()))
	{
		held = selected;
		held.piece->setHeld(true);
	}
	if (held != Selection::NONE)
	{
		held.piece->setDesiredPosition(intersectUp);
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
