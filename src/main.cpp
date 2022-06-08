#include <string>
#include <iostream>

#include <sdl/SDL.h>
#include <sdl/SDL_opengl.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Window.h"
#include "GameWindow.h"

constexpr float CAMERA_ROT_SPEED = 300.0f;
constexpr glm::vec2 TABLE_SIZE = glm::vec2(8, 8);

int gameState[8][8] = {
	{0, 2, 0, 2, 0, 2, 0, 2},
	{2, 0, 2, 0, 2, 0, 2, 0},
	{0, 2, 0, 2, 0, 2, 0, 2},
	{0, 0, 0, 0, 0, 0, 0, 0},
	{0, 0, 0, 0, 0, 0, 0, 0},
	{1, 0, 1, 0, 1, 0, 1, 0},
	{0, 1, 0, 1, 0, 1, 0, 1},
	{1, 0, 1, 0, 1, 0, 1, 0},
};

glm::uvec2 mouseCoords = glm::uvec2(0, 0);
glm::vec2 selectedField = glm::vec2(-1, -1);

int currentPlayer = 1;
glm::vec2 selectedPawn = glm::vec2(-1, -1);
int viableMoves[8][8] = {
	{0, 0, 0, 0, 0, 0, 0, 0},
	{0, 0, 0, 0, 0, 0, 0, 0},
	{0, 0, 0, 0, 0, 0, 0, 0},
	{0, 0, 0, 0, 0, 0, 0, 0},
	{0, 0, 0, 0, 0, 0, 0, 0},
	{0, 0, 0, 0, 0, 0, 0, 0},
	{0, 0, 0, 0, 0, 0, 0, 0},
	{0, 0, 0, 0, 0, 0, 0, 0},
};

int possibleCaptures[8][8] = {
	{0, 0, 0, 0, 0, 0, 0, 0},
	{0, 0, 0, 0, 0, 0, 0, 0},
	{0, 0, 0, 0, 0, 0, 0, 0},
	{0, 0, 0, 0, 0, 0, 0, 0},
	{0, 0, 0, 0, 0, 0, 0, 0},
	{0, 0, 0, 0, 0, 0, 0, 0},
	{0, 0, 0, 0, 0, 0, 0, 0},
	{0, 0, 0, 0, 0, 0, 0, 0},
};

int movable[8][8] = {
	{0, 0, 0, 0, 0, 0, 0, 0},
	{0, 0, 0, 0, 0, 0, 0, 0},
	{0, 0, 0, 0, 0, 0, 0, 0},
	{0, 0, 0, 0, 0, 0, 0, 0},
	{0, 0, 0, 0, 0, 0, 0, 0},
	{0, 0, 0, 0, 0, 0, 0, 0},
	{0, 0, 0, 0, 0, 0, 0, 0},
	{0, 0, 0, 0, 0, 0, 0, 0},
};

bool hasSecondCapture = false;
bool canMove = true;
bool doCameraAnimation = false;
float desiredCamRot = 0.0f;

glm::vec3 linePlaneIntersect(glm::vec3 rayVec, glm::vec3 rayPoint, glm::vec3 planeNormal, glm::vec3 planePoint)
{
	glm::vec3 diff = rayPoint - planePoint;
	float prod1 = glm::dot(diff, planeNormal);
	float prod2 = glm::dot(rayVec, planeNormal);
	float prod3 = prod1 / prod2;
	return rayPoint - rayVec * prod3;
}

void calcSelectedField(const Camera& camera)
{
	glm::vec3 intersect = linePlaneIntersect(
		camera.forwardVec(mouseCoords),
		camera.unProject(mouseCoords),
		glm::vec3(0.0f, 1.0f, 0.0f),
		glm::vec3(0.0f)
	);

	if (intersect.x > -TABLE_SIZE.x / 2.0f
		&& intersect.x < TABLE_SIZE.x / 2.0f
		&& intersect.z > -TABLE_SIZE.y / 2.0f
		&& intersect.z < TABLE_SIZE.y / 2.0f)
	{
		int x = (int)(intersect.x + TABLE_SIZE.x / 2.0f);
		int y = (int)(intersect.z + TABLE_SIZE.y / 2.0f);
		selectedField = glm::vec2(x, y);
	}
	else
	{
		selectedField = glm::vec2(-1, -1);
	}
}

bool isSelected(int x, int y)
{
	if (gameState[y][x] != currentPlayer)
	{
		return false;
	}
	return x == selectedField.x && y == selectedField.y;
}

void resetMoves()
{
	for (int y = 0; y < 8; y++)
	{
		for (int x = 0; x < 8; x++)
		{
			viableMoves[x][y] = 0;
			possibleCaptures[x][y] = 0;
		}
	}
}

void resetSelected()
{
	selectedPawn = glm::vec2(-1, -1);
	resetMoves();
}

bool isInBounds(int x, int y)
{
	return x >= 0 && x < TABLE_SIZE.x && y >= 0 && y < TABLE_SIZE.y;
}

void calcMove(int x, int y, int dX, int dY)
{
	if (isInBounds(x + dX, y + dY)
		&& gameState[y + dY][x + dX] == 0)
	{
		viableMoves[x + dX][y + dY] = 1;
	}
}

void calcCapture(int x, int y, int dX, int dY)
{
	if (isInBounds(x + dX, y + dY)
		&& gameState[y + dY][x + dX] != 0
		&& gameState[y + dY][x + dX] != currentPlayer
		&& isInBounds(x + dX * 2, y + dY * 2)
		&& gameState[y + dY * 2][x + dX * 2] == 0)
	{
		viableMoves[x + dX * 2][y + dY * 2] = 1;
		possibleCaptures[x + dX][y + dY] = 1;
	}
}

void calcViableCaptures(int x, int y)
{
	calcCapture(x, y, 1, 1);
	calcCapture(x, y, -1, 1);
	calcCapture(x, y, -1, -1);
	calcCapture(x, y, 1, -1);
}

void calcViableMoves(int x, int y)
{
	if (currentPlayer == 1)
	{
		calcMove(x, y, 1, -1);
		calcMove(x, y, -1, -1);
	}
	else
	{
		calcMove(x, y, 1, 1);
		calcMove(x, y, -1, 1);
	}
	calcViableCaptures(x, y);
}

bool hasViableMoves(int x, int y)
{
	bool hasViableMoves = false;
	calcViableMoves(x, y);
	for (int y = 0; y < TABLE_SIZE.y; y++)
	{
		for (int x = 0; x < TABLE_SIZE.x; x++)
		{
			if (viableMoves[x][y] == 1)
			{
				hasViableMoves = true;
				resetSelected();
				return true;
			}
		}
	}
	resetSelected();
	return false;
}

void setMovable()
{
	for (int y = 0; y < TABLE_SIZE.y; y++)
	{
		for (int x = 0; x < TABLE_SIZE.x; x++)
		{
			movable[x][y] = 0;
			if (gameState[y][x] == currentPlayer
				&& hasViableMoves(x, y))
			{
				movable[x][y] = 1;
			}
		}
	}
}

void selectPawn()
{
	if (selectedField.x < 0
		&& selectedField.x >= TABLE_SIZE.x
		&& selectedField.y < 0
		&& selectedField.y >= TABLE_SIZE.y)
	{
		return;
	}
	for (int y = 0; y < TABLE_SIZE.y; y++)
	{
		for (int x = 0; x < TABLE_SIZE.x; x++)
		{
			if (isSelected(x, y))
			{
				resetSelected();
				selectedPawn = glm::vec2(x, y);
				int x = selectedPawn.x;
				int y = selectedPawn.y;
				calcViableMoves(x, y);
			}
		}
	}
}

void endGame(int winner)
{
	std::string winnerS = winner == 1 ? "White" : "Black";
	std::cout << winnerS << " player wins!" << std::endl;
}

void checkGameState()
{
	int whiteCheckers = 0;
	int blackCheckers = 0;
	int moves = 0;

	for (int y = 0; y < TABLE_SIZE.y; y++)
	{
		for (int x = 0; x < TABLE_SIZE.x; x++)
		{
			if (gameState[y][x] == 1)
				whiteCheckers++;
			if (gameState[y][x] == 2)
				blackCheckers++;
			if (movable[x][y] == 1)
				moves++;
		}
	}
	if (whiteCheckers == 0)
		endGame(2);
	else if (blackCheckers == 0)
		endGame(1);
	else if (moves == 0)
		endGame(currentPlayer == 1 ? 2 : 1);
}

void changePlayer()
{
	canMove = false;
	currentPlayer = currentPlayer == 1 ? 2 : 1;

	doCameraAnimation = true;
	desiredCamRot = currentPlayer == 1 ? 0.0f : 180.0f;

	setMovable();
	checkGameState();
}

void makeMove()
{
	if (!canMove)
	{
		return;
	}
	int x = selectedField.x;
	int y = selectedField.y;
	if (!hasSecondCapture && gameState[y][x] == currentPlayer && movable[x][y] == 1)
	{
		selectPawn();
		return;
	}
	if (viableMoves[x][y] == 1)
	{
		int pX = selectedPawn.x;
		int pY = selectedPawn.y;
		gameState[pY][pX] = 0;
		gameState[y][x] = currentPlayer;
		int cX = (x + pX) / 2;
		int cY = (y + pY) / 2;
		bool hasViableCapture = false;
		if (possibleCaptures[cX][cY] == 1)
		{
			hasSecondCapture = false;
			gameState[cY][cX] = 0;
			resetSelected();
			selectedPawn = glm::vec2(x, y);
			calcViableCaptures(x, y);
			for (int y = 0; y < TABLE_SIZE.y; y++)
			{
				for (int x = 0; x < TABLE_SIZE.x; x++)
				{
					if (possibleCaptures[x][y] == 1)
					{
						hasViableCapture = true;
						hasSecondCapture = true;
						break;
					}
				}
				if (hasViableCapture)
					break;
			}
		}
		if (!hasViableCapture)
		{
			resetSelected();
			changePlayer();
		}
	}
}

void animateCamera(Camera& camera, float dT)
{
	if (!doCameraAnimation)
		return;
	glm::vec3 cameraRotation = camera.getRotation();
	if (currentPlayer == 1 && cameraRotation.y > desiredCamRot)
	{
		cameraRotation.y -= dT * CAMERA_ROT_SPEED;
	}
	else if (currentPlayer == 2 && cameraRotation.y < desiredCamRot)
	{
		cameraRotation.y += dT * CAMERA_ROT_SPEED;
	}
	else
	{
		cameraRotation.y = desiredCamRot;
		doCameraAnimation = false;
		canMove = true;
	}
	camera.setRotation(cameraRotation);
}

void updateMouseCoords(glm::uvec2 mousePos)
{
	mouseCoords.x = mousePos.x;
	mouseCoords.y = mousePos.y;
}

int main(int argc, char* argv[])
{
	constexpr glm::uvec2 windowDimensions{1280u, 720u};
	Window* const window = new GameWindow("SDL-Checkers", windowDimensions);

	window->run();

	delete window;
	return 0;
}
