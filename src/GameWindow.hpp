#pragma once

#include "Window.h"
#include "Renderer.h"
#include "Camera.h"
#include "MeshPrototypes.h"
#include "ManPiece.h"

#define SELECTED_NONE glm::ivec2(-1.0f)

class GameWindow : public Window
{
public:
	GameWindow(const char* title, const glm::uvec2& dimensions)
		: Window(title, dimensions),
		  m_renderer(this)
	{
	}

	virtual ~GameWindow() override
	{
		for (int i = 0; i < 10; i++)
		{
			for (int j = 0; j < 10; j++)
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

protected:
	void init() override
	{
		const glm::uvec2& windowDimensions = getDimensions();
		m_camera.setViewportDimensions({windowDimensions.x, windowDimensions.y});
		m_camera.makePerspective(45.0f, 1.0f, 1000.0f);
		m_camera.makeLookAt({0.0f, 0.0f, 1.0f}, {0.0f, 1.0f, 0.0f});
		m_camera.setPosition({0.0f, -1.0f, 11.0f});
		m_camera.setRotation({-45.0f, 0.0f, 0.0f});

		const LightSource light = {{0.3f, -1.0f, 0.45f}, 1.0f};

		m_renderer.setClearColor({0.15f, 0.15f, 0.15f});
		m_renderer.setLightSource(light);
		m_renderer.setAmbientLight({0.4f, 0.4f, 0.4f});

		board = new Piece**[10];
		for (int i = 0; i < 10; i++)
		{
			board[i] = new Piece*[10];
			for (int j = 0; j < 10; j++)
			{
				board[i][j] = nullptr;
			}
		}

		for (int z = 0; z < 10; z++)
		{
			for (int x = 0; x < 10; x++)
			{
				const glm::vec3 pos = {x - 10 / 2.0f + 0.5f, 0.0f, z - 10 / 2.0f + 0.5f};
				if (z % 2 == 0 && x % 2 != 0 || z % 2 != 0 && x % 2 == 0)
				{
					if (z < 4)
					{
						board[z][x] = new ManPiece(PieceType::Dark);
						board[z][x]->setDesiredPosition(pos);
						board[z][x]->setInterpolationSpeed(5.0f);
					}
					else if (z > 5)
					{
						board[z][x] = new ManPiece(PieceType::Light);
						board[z][x]->setDesiredPosition(pos);
						board[z][x]->setInterpolationSpeed(5.0f);
					}
				}
			}
		}
	}

	Piece*** board;
	glm::ivec2 selected = SELECTED_NONE;
	Piece* selectedPiece;
	glm::ivec2 held = SELECTED_NONE;
	Piece* heldPiece;

	glm::vec2 mousePosLastFrame = getMousePosition();

	void update(const float& deltaTime) override
	{
		m_elapsed = m_elapsed + deltaTime;

		const LightSource light = {
			{glm::cos(m_elapsed) * 0.3f, -1.0f, glm::sin(m_elapsed) * 0.45f},
			1.0f
		};
		m_renderer.setLightSource(light);

		glm::vec2 mouseDelta = getMousePosition() - mousePosLastFrame;
		mousePosLastFrame = getMousePosition();

		glm::vec3 camRot = m_camera.getRotation();
		if(isMouseButtonDown(SDL_BUTTON_MIDDLE))
		{
			camRot.y -= mouseDelta.x * 0.3f;
		}
		m_camera.setRotation(camRot);

		for (int z = 0; z < 10; z++)
		{
			for (int x = 0; x < 10; x++)
			{
				if (board[z][x] != nullptr)
					board[z][x]->setState(PieceState::Neutral);
			}
		}

		updateSelected();
		if (heldPiece != nullptr)
		{
			heldPiece->setState(PieceState::Selected);
		}
		else if (selected != SELECTED_NONE
			&& board[selected.x][selected.y] != nullptr)
		{
			board[selected.x][selected.y]->setState(PieceState::Selected);
		}

		const glm::vec2 mousePos = getMousePosition();
		const glm::vec3 mousePosWorld = m_camera.unProject(mousePos);
		const glm::vec3 mouseForward = m_camera.forwardVec(mousePos);
		const glm::vec3 intersectUp = linePlaneIntersect(
			mouseForward,
			mousePosWorld,
			{0.0f, 1.0f, 0.0f},
			{0.0f, 1.0f, 0.0f}
		);
		const glm::vec3 intersect = linePlaneIntersect(
			mouseForward,
			mousePosWorld,
			{0.0f, 1.0f, 0.0f},
			{0.0f, 0.0f, 0.0f}
		);

		if (isMouseButtonPressed(SDL_BUTTON_RIGHT)
			&& heldPiece != nullptr)
		{
			heldPiece->setHeld(false);
			heldPiece->setDesiredPosition(positionAt(held.y, held.x));
			held = SELECTED_NONE;
			heldPiece = nullptr;
		}
		if (isMouseButtonPressed(SDL_BUTTON_LEFT)
			&& heldPiece != nullptr
			&& selected != SELECTED_NONE
			&& (board[selected.x][selected.y] == nullptr
				|| board[selected.x][selected.y] == heldPiece))
		{
			heldPiece->setHeld(false);
			heldPiece->setDesiredPosition(positionAt(selected.y, selected.x));

			board[held.x][held.y] = nullptr;
			board[selected.x][selected.y] = heldPiece;
			held = SELECTED_NONE;
			heldPiece = nullptr;
		}
		else if (isMouseButtonPressed(SDL_BUTTON_LEFT)
			&& selectedPiece != nullptr
			&& heldPiece == nullptr)
		{
			held = selected;
			heldPiece = selectedPiece;
			heldPiece->setHeld(true);
		}
		if (heldPiece != nullptr)
		{
			heldPiece->setDesiredPosition(intersectUp);
		}

		for (int z = 0; z < 10; z++)
		{
			for (int x = 0; x < 10; x++)
			{
				if (board[z][x] != nullptr)
					board[z][x]->update(deltaTime);
			}
		}

		m_renderer.beginFrame(&m_camera);

		for (int z = 0; z < 10; z++)
		{
			for (int x = 0; x < 10; x++)
			{
				if (board[z][x] != nullptr)
					board[z][x]->render(m_renderer);
			}
		}

		Transform boardTransform = {};
		for (int z = 0; z < 10; z++)
		{
			for (int x = 0; x < 10; x++)
			{
				const glm::vec3 pos = positionAt(x, z);
				boardTransform.position = pos;
				boardTransform.position.y = -0.1f;
				if (z % 2 == 0 && x % 2 == 0 || z % 2 != 0 && x % 2 != 0)
				{
					m_renderer.drawTriangles(m_prototypes.getTileOdd(), boardTransform);
				}
				else
				{
					m_renderer.drawTriangles(m_prototypes.getTileEven(), boardTransform);
				}
			}
		}
	}

	void onResize(const glm::uvec2& windowDimensions) override
	{
		Window::onResize(windowDimensions);
		m_camera.setViewportDimensions({windowDimensions.x, windowDimensions.y});
		m_camera.makePerspective(45.0f, 1.0f, 1000.0f);
		m_camera.makeLookAt({0.0f, 0.0f, 1.0f}, {0.0f, 1.0f, 0.0f});
	}

protected:
	glm::vec3 positionAt(int x, int z) const
	{
		return {
			static_cast<float>(x) - 10.0f / 2.0f + 0.5f,
			0.0f,
			static_cast<float>(z) - 10.0f / 2.0f + 0.5f
		};
	}

	void updateSelected()
	{
		const glm::vec2 mousePos = getMousePosition();
		glm::vec3 intersect = linePlaneIntersect(
			m_camera.forwardVec(mousePos),
			m_camera.unProject(mousePos),
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
			selected = {
				static_cast<int>(intersect.z),
				static_cast<int>(intersect.x)
			};
			selectedPiece = board[selected.x][selected.y];
		}
		else
		{
			selected = {-1, -1};
			selectedPiece = nullptr;
		}
	}

	glm::vec3 linePlaneIntersect(
		const glm::vec3& rayVec,
		const glm::vec3& rayPoint,
		const glm::vec3& planeNormal,
		const glm::vec3& planePoint
	) const
	{
		const glm::vec3 diff = rayPoint - planePoint;
		const float prod1 = glm::dot(diff, planeNormal);
		const float prod2 = glm::dot(rayVec, planeNormal);
		const float prod3 = prod1 / prod2;
		return rayPoint - rayVec * prod3;
	}

private:
	Renderer m_renderer;
	Camera m_camera;

	float m_elapsed = 0.0;

	MeshPrototypes m_prototypes;
};
