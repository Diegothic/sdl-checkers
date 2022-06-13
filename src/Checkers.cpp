#include "Checkers.h"

#include "GameWindow.h"
#include "KingPiece.h"
#include "ManPiece.h"

Checkers::Checkers(GameWindow* window, uint8_t boardSize)
	: m_window(window),
	  m_boardSize(boardSize)
{
	m_board = new Piece* *[m_boardSize];
	for (int i = 0; i < m_boardSize; i++)
	{
		m_board[i] = new Piece*[m_boardSize];
		for (int j = 0; j < m_boardSize; j++)
		{
			m_board[i][j] = nullptr;
		}
	}
}

Checkers::~Checkers()
{
	for (int i = 0; i < m_boardSize; i++)
	{
		for (int j = 0; j < m_boardSize; j++)
		{
			if (m_board[i][j] != nullptr)
			{
				delete m_board[i][j];
				m_board[i][j] = nullptr;
			}
		}
		delete[] m_board[i];
	}
	delete[] m_board;
}

void Checkers::reset() const
{
	for (int z = 0; z < m_boardSize; z++)
	{
		for (int x = 0; x < m_boardSize; x++)
		{
			if (m_board[z][x] != nullptr)
			{
				delete m_board[z][x];
				m_board[z][x] = nullptr;
			}

			const glm::vec3 pos = positionFromBoardCoords(x, z);
			if (isDarkTile(x, z))
			{
				if (z < m_boardSize / 2 - 1)
				{
					m_board[z][x] = new ManPiece(PieceType::Dark);
					m_board[z][x]->setDesiredPosition(pos);
					m_board[z][x]->setInterpolationSpeed(5.0f);
				}
				else if (z > m_boardSize / 2)
				{
					m_board[z][x] = new ManPiece(PieceType::Light);
					m_board[z][x]->setDesiredPosition(pos);
					m_board[z][x]->setInterpolationSpeed(5.0f);
				}
			}
		}
	}
	updateBoardState();
}

void Checkers::update(const float& deltaTime)
{
	switch (m_state)
	{
	case GameState::PlayerMoving:
		updateStatePlayerMoving(deltaTime);
		break;
	case GameState::ChangingPlayer:
		updateStateChangingPlayer(deltaTime);
		break;
	case GameState::Ended:
		updateStateEnded(deltaTime);
		break;
	}

	forEachPiece([&deltaTime](Piece* const piece, int z, int x)
	{
		piece->update(deltaTime);
	});
}


void Checkers::updateStatePlayerMoving(const float& deltaTime)
{
	updateHeldPosition();
	updateSelection();
	updatePieceStates();

	if (m_window->isMouseButtonPressed(SDL_BUTTON_RIGHT))
	{
		putBackPiece();
	}

	if (m_window->isMouseButtonPressed(SDL_BUTTON_LEFT))
	{
		if (m_held == Selection::NONE)
		{
			pickUpPiece();
		}
		else
		{
			const auto possibleMove = std::find(
				m_held.piece->getViableMoves().begin(),
				m_held.piece->getViableMoves().end(),
				glm::ivec2(m_selected.coords.x, m_selected.coords.y)
			);
			if (possibleMove != m_held.piece->getViableMoves().end())
			{
				makeMove(*possibleMove);
			}
		}
	}
}

void Checkers::updateStateChangingPlayer(const float& deltaTime)
{
	const bool finishedAnimation = animateCamera(deltaTime);
	forEachPiece([&](Piece* const piece, int z, int x)
	{
		piece->setState(PieceState::Neutral);
		if (piece->isCaptured())
		{
			piece->setState(PieceState::Capture);
			Transform pieceTransform = piece->getTransform();
			pieceTransform.position.y += 0.05f;
			piece->setDesiredPosition(pieceTransform.position);
		}
	});
	if (finishedAnimation)
	{
		forEachPiece([&](const Piece* const piece, int z, int x)
		{
			if (piece->isCaptured())
			{
				delete m_board[z][x];
				m_board[z][x] = nullptr;
			}
		});
		m_state = GameState::PlayerMoving;
		updateBoardState();
	}
}

void Checkers::updateStateEnded(const float& deltaTime)
{
}

void Checkers::updateHeldPosition() const
{
	if (m_held == Selection::NONE)
	{
		return;
	}
	const glm::vec2 mousePos = m_window->getMousePosition();
	const glm::vec3 intersectUp = VectorMath::linePlaneIntersect(
		m_window->getCamera().forwardVec(mousePos),
		m_window->getCamera().unProject(mousePos),
		{0.0f, 1.0f, 0.0f},
		{0.0f, 1.5f, 0.0f}
	);
	m_held.piece->setDesiredPosition(intersectUp);
}

void Checkers::updateSelection()
{
	const glm::vec2 mousePos = m_window->getMousePosition();
	glm::vec3 intersection = VectorMath::linePlaneIntersect(
		m_window->getCamera().forwardVec(mousePos),
		m_window->getCamera().unProject(mousePos),
		{0.0f, 1.0f, 0.0f},
		{0.0f, 0.0f, 0.0f}
	);
	intersection.x += 5.0f;
	intersection.z += 5.0f;
	const glm::ivec2 selectedCoords = {
		static_cast<int>(intersection.z),
		static_cast<int>(intersection.x)
	};
	m_selected = isInBoardBounds(selectedCoords)
		             ? Selection{
			             m_board[selectedCoords.x][selectedCoords.y],
			             selectedCoords
		             }
		             : Selection::NONE;
}

void Checkers::updateBoardState() const
{
	forEachPiece([&](Piece* const piece, int z, int x)
	{
		piece->setMovable(false);
		piece->recalculateMoves({z, x}, m_board, m_boardSize);
	});

	if (m_held == Selection::NONE)
	{
		const auto movables = getMovablePieces();
		for (const auto& movable : movables)
		{
			m_board[movable.x][movable.y]->setMovable(true);
		}
	}
}

void Checkers::updatePieceStates() const
{
	forEachPiece([&](Piece* const piece, int z, int x)
	{
		piece->setState(PieceState::Neutral);
		if (piece->isCaptured())
			piece->setState(PieceState::Capture);
		if (piece->isMovable())
			piece->setState(PieceState::Movable);
	});
	if (m_held != Selection::NONE)
	{
		m_held.piece->setState(PieceState::Selected);
		for (const auto& move : m_held.piece->getViableMoves())
		{
			if (move.captures)
			{
				m_board[move.capture.x][move.capture.y]->setState(PieceState::Capture);
			}
		}
	}
	else if (m_selected != Selection::NONE && m_selected.hasPiece())
	{
		m_selected.piece->setState(PieceState::Selected);
	}
}

void Checkers::pickUpPiece()
{
	if (m_selected == Selection::NONE)
	{
		return;
	}
	if (m_selected.hasPiece() && m_selected.piece->isMovable())
	{
		m_held = m_selected;
		m_held.piece->setHeld(true);
	}
	updateBoardState();
}

void Checkers::putBackPiece()
{
	if (m_held == Selection::NONE)
	{
		return;
	}
	m_held.piece->setHeld(false);
	m_held.piece->setDesiredPosition(positionFromBoardCoords(m_held.coords.y, m_held.coords.x));
	m_held = Selection::NONE;
	updateBoardState();
}

void Checkers::makeMove(const Move& move)
{
	const glm::ivec2 newPieceCoords = {m_selected.coords.x, m_selected.coords.y};
	m_held.piece->setHeld(false);
	m_held.piece->setDesiredPosition(
		positionFromBoardCoords(newPieceCoords.y, newPieceCoords.x)
	);

	m_board[m_held.coords.x][m_held.coords.y] = nullptr;
	m_board[newPieceCoords.x][newPieceCoords.y] = m_held.piece;
	m_held = Selection::NONE;

	if (move.captures)
	{
		m_board[move.capture.x][move.capture.y]->setCaptured(true);
		m_currentPlayer.capturedThisTurn = true;
		m_currentPlayer.capturingPieceCoords = m_selected.coords;
	}

	checkForPieceUpgrade(newPieceCoords.x, newPieceCoords.y);
	Piece* newPiece = m_board[newPieceCoords.x][newPieceCoords.y];
	newPiece->recalculateMoves(newPieceCoords, m_board, m_boardSize);

	if (!(m_currentPlayer.capturedThisTurn && newPiece->getCapturesCount() > 0))
	{
		finishMove();
	}

	updateBoardState();
}

void Checkers::checkForPieceUpgrade(int z, int x) const
{
	const Piece* checked = m_board[z][x];
	if (checked == nullptr)
	{
		return;
	}
	if ((m_currentPlayer.pieceType == PieceType::Light && z == 0)
		|| (m_currentPlayer.pieceType == PieceType::Dark && z == 9))
	{
		Piece* kingPiece = new KingPiece(*checked);
		delete m_board[z][x];
		m_board[z][x] = kingPiece;
	}
}

void Checkers::finishMove()
{
	m_currentPlayer = m_currentPlayer.pieceType == PieceType::Light
		                  ? Player{PieceType::Dark, false}
		                  : Player{PieceType::Light, false};
	m_state = GameState::ChangingPlayer;
}

void Checkers::render(const Renderer& renderer) const
{
	forEachPiece([&renderer](const Piece* const piece, int z, int x)
	{
		piece->render(renderer);
	});

	if (m_held != Selection::NONE)
	{
		drawMoves(renderer);
	}

	drawTable(renderer);
}

void Checkers::drawMoves(const Renderer& renderer) const
{
	Transform transform = {};
	transform.scale = glm::vec3(0.5f);
	for (const auto& move : m_held.piece->getViableMoves())
	{
		transform.position = positionFromBoardCoords(move.destination.y, move.destination.x);
		renderer.drawTriangles(
			m_currentPlayer.pieceType == PieceType::Light
				? m_window->getPrototypes().getPieceLightMovable()
				: m_window->getPrototypes().getPieceDarkMovable(),
			transform
		);
	}
}

void Checkers::drawTable(const Renderer& renderer) const
{
	Transform tileTransform = {};
	for (int z = 0; z < m_boardSize; z++)
	{
		for (int x = 0; x < m_boardSize; x++)
		{
			const glm::vec3 pos = positionFromBoardCoords(x, z);
			tileTransform.position = pos;
			tileTransform.position.y = -0.1f;
			renderer.drawTriangles(
				isDarkTile(x, z)
					? m_window->getPrototypes().getTileEven()
					: m_window->getPrototypes().getTileOdd(),
				tileTransform
			);
		}
	}
}

void Checkers::forEachPiece(const std::function<void(Piece*, int, int)>& func) const
{
	for (int z = 0; z < m_boardSize; z++)
	{
		for (int x = 0; x < m_boardSize; x++)
		{
			if (m_board[z][x] != nullptr)
				func(m_board[z][x], z, x);
		}
	}
}

bool Checkers::isDarkTile(int x, int z)
{
	return z % 2 == 0 && x % 2 != 0
		|| z % 2 != 0 && x % 2 == 0;
}

glm::vec3 Checkers::positionFromBoardCoords(int x, int z) const
{
	return {
		static_cast<float>(x) - static_cast<float>(m_boardSize) / 2.0f + 0.5f,
		0.0f,
		static_cast<float>(z) - static_cast<float>(m_boardSize) / 2.0f + 0.5f
	};
}

bool Checkers::isInBoardBounds(const glm::ivec2& position) const
{
	return position.y >= 0
		&& position.y < m_boardSize
		&& position.x >= 0
		&& position.x < m_boardSize;
}

std::vector<glm::ivec2> Checkers::getMovablePieces() const
{
	if (m_currentPlayer.capturedThisTurn)
	{
		return {m_currentPlayer.capturingPieceCoords};
	}
	const bool hasAnyCapture = hasCaptures();
	std::vector<glm::ivec2> movables;
	forEachPiece([&](const Piece* const piece, int z, int x)
	{
		if (piece->getType() == m_currentPlayer.pieceType
			&& !piece->getViableMoves().empty()
			&& (!hasAnyCapture
				|| (hasAnyCapture
					&& piece->getCapturesCount() > 0)))
		{
			movables.emplace_back(glm::ivec2{z, x});
		}
	});
	return movables;
}

bool Checkers::hasCaptures() const
{
	for (int z = 0; z < m_boardSize; z++)
	{
		for (int x = 0; x < m_boardSize; x++)
		{
			if (m_board[z][x] != nullptr
				&& m_board[z][x]->getType() == m_currentPlayer.pieceType
				&& m_board[z][x]->getCapturesCount() > 0)
			{
				return true;
			}
		}
	}
	return false;
}
