#include <string>
#include <iostream>

#include <sdl/SDL.h>
#include <sdl/SDL_opengl.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Camera.h"

constexpr float CAMERA_ROT_SPEED = 300.0f;
constexpr glm::vec3 LIGHT_VECTOR = glm::vec3(0.3f, -1.0f, 0.45f);
constexpr glm::vec2 TABLE_SIZE = glm::vec2(8, 8);

constexpr glm::vec3 PAWN_COLOR_WHITE = glm::vec3(0.45f);
constexpr glm::vec3 PAWN_COLOR_BLACK = glm::vec3(0.2f);
constexpr glm::vec3 TABLE_COLOR_ODD = glm::vec3(0.25f, 0.22f, 0.15f);
constexpr glm::vec3 TABLE_COLOR_EVEN = glm::vec3(0.14f, 0.09f, 0.06f);

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

void drawPawn(glm::vec3 color, glm::vec3 center)
{
	glPushMatrix();
	glTranslatef(center.x, center.y, center.z);

	glm::vec3 lightVec = glm::normalize(LIGHT_VECTOR);
	glm::vec3 normal;
	float inten;

	float r = 0.35f;
	float h = 0.2f;
	double step = glm::pi<double>() / 10.0;

	for (double i = 0.0; i < glm::pi<double>() * 2.0; i += step)
	{
		float x = sin(i);
		float y = cos(i);

		//TOP
		normal = glm::normalize(glm::vec3(0.0f, 1.0f, 0.0f));
		inten = 1.0f - glm::dot(lightVec, normal);
		glColor3f(color.x * inten, color.y * inten, color.z * inten);

		glBegin(GL_TRIANGLES);

		glVertex3f(0.0f, h, 0.0f);
		glVertex3f(sin(i) * r, h, cos(i) * r);
		glVertex3f(sin(i + step) * r, h, cos(i + step) * r);

		glEnd();

		//SIDE
		normal = glm::normalize(glm::vec3(sin(i), 0.0f, cos(i)));
		inten = 1.0f - glm::dot(lightVec, normal);
		glColor3f(color.x * inten, color.y * inten, color.z * inten);

		glBegin(GL_QUADS);

		glVertex3f(sin(i) * r, h, cos(i) * r);
		glVertex3f(sin(i) * r, 0.0f, cos(i) * r);
		glVertex3f(sin(i + step) * r, 0.0f, cos(i + step) * r);
		glVertex3f(sin(i + step) * r, h, cos(i + step) * r);

		glEnd();

		//BOTTOM
		normal = glm::normalize(glm::vec3(0.0f, -1.0f, 0.0f));
		inten = 1.0f - glm::dot(lightVec, normal);
		glColor3f(color.x * inten, color.y * inten, color.z * inten);

		glBegin(GL_TRIANGLES);

		glVertex3f(0.0f, 0.0f, 0.0f);
		glVertex3f(sin(i) * r, 0.0f, cos(i) * r);
		glVertex3f(sin(i + step) * r, 0.0f, cos(i + step) * r);

		glEnd();
	}
	glPopMatrix();
}

void drawTable()
{
	glPushMatrix();

	glm::vec3 lightVec = glm::normalize(LIGHT_VECTOR);
	glm::vec3 normal = glm::normalize(glm::vec3(0.0f, 1.0f, 0.0f));
	float inten = 1.0f - glm::dot(lightVec, normal);
	glm::vec3 colorOdd = glm::vec3(TABLE_COLOR_ODD * inten);
	glm::vec3 colorEven = glm::vec3(TABLE_COLOR_EVEN * inten);
	glm::vec3 color;

	for (int z = 0; z < TABLE_SIZE.y; z++)
	{
		for (int x = 0; x < TABLE_SIZE.x; x++)
		{
			glPushMatrix();

			glTranslatef(x - TABLE_SIZE.x / 2.0f + 0.5f, 0.0f, z - TABLE_SIZE.y / 2.0f + 0.5f);

			if (z % 2 == 0)
			{
				color = x % 2 == 0 ? colorOdd : colorEven;
			}
			else
			{
				color = x % 2 == 1 ? colorOdd : colorEven;
			}

			if (viableMoves[x][z] == 1)
			{
				color = glm::vec3(0.35f, 0.25f, 0.75f);
			}

			glColor3f(color.x * inten, color.y * inten, color.z * inten);

			glBegin(GL_QUADS);

			glVertex3f(-0.5f, 0.0f, -0.5f);
			glVertex3f(0.5f, 0.0f, -0.5f);
			glVertex3f(0.5f, 0.0f, 0.5f);
			glVertex3f(-0.5f, 0.0f, 0.5f);

			glEnd();

			glPopMatrix();
		}
	}

	glPopMatrix();
}

void drawLine(glm::vec3 start, glm::vec3 end, float width)
{
	glColor3f(1.0f, 1.0f, 1.0f);
	glLineWidth(width);

	glBegin(GL_LINES);

	glVertex3f(start.x, start.y, start.z);
	glVertex3f(end.x, end.y, end.z);

	glEnd();
}

void drawPawn(int x, int y)
{
	if (gameState[y][x] == 0)
	{
		return;
	}
	glm::vec3 color = PAWN_COLOR_WHITE;
	if (gameState[y][x] == 2)
	{
		color = PAWN_COLOR_BLACK;
	}
	if (selectedPawn != glm::vec2(x, y) && isSelected(x, y))
	{
		color += glm::vec3(0.2f);
	}
	if (possibleCaptures[x][y] == 1)
	{
		color += currentPlayer == 2
			         ? glm::vec3(0.1f, -0.3f, -0.3f)
			         : glm::vec3(0.3f, -0.05f, -0.05f);
	}
	if (movable[x][y] == 1)
	{
		color += currentPlayer == 1
			         ? glm::vec3(-0.08f, -0.08f, 0.08f)
			         : glm::vec3(0.0f, 0.0f, 0.08f);
	}
	float height = 0.0f;
	if (selectedPawn.x == x && selectedPawn.y == y)
	{
		color += currentPlayer == 1
			         ? glm::vec3(-0.1f, -0.1f, 0.1f)
			         : glm::vec3(0.0f, 0.0f, 0.1f);
		height = 0.25f;
	}
	glm::vec3 location = glm::vec3(
		1.0f * x - TABLE_SIZE.x / 2.0f + 0.5f,
		height,
		1.0f * y - TABLE_SIZE.y / 2.0f + 0.5f
	);

	drawPawn(color, location);
}

void drawPawns()
{
	for (int y = 0; y < TABLE_SIZE.y; y++)
	{
		for (int x = 0; x < TABLE_SIZE.x; x++)
		{
			drawPawn(x, y);
		}
	}
}

void updateMouseCoords(glm::uvec2 mousePos)
{
	mouseCoords.x = mousePos.x;
	mouseCoords.y = mousePos.y;
}

int main(int argc, char* argv[])
{
	SDL_Init(SDL_INIT_EVERYTHING);

	constexpr glm::uvec2 windowDimensions{1280u, 720u};
	const auto window = SDL_CreateWindow(
		"SDL-Checkers",
		SDL_WINDOWPOS_CENTERED,
		SDL_WINDOWPOS_CENTERED,
		windowDimensions.x,
		windowDimensions.y,
		SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL
	);

	const auto glContext = SDL_GL_CreateContext(window);
	SDL_GL_MakeCurrent(window, glContext);

	bool windowShouldClose = false;
	SDL_Event event;

	uint64_t frameTime = SDL_GetPerformanceCounter();
	uint64_t lastFrameTime;
	double deltaTime;

	Camera camera;
	camera.setViewportDimensions({windowDimensions.x, windowDimensions.y});
	camera.makePerspective(45.0f, 1.0f, 1000.0f);
	camera.makeLookAt({0.0f, 0.0f, 1.0f}, {0.0f, 1.0f, 0.0f});
	camera.setPosition({0.0f, 0.0f, 10.0f});
	camera.setRotation({-40.0f, 0.0f, 0.0f});

	setMovable();

	constexpr glm::vec3 bgColor(0.3f);
	glClearColor(bgColor.x, bgColor.y, bgColor.z, 1.0);
	glEnable(GL_DEPTH_TEST);

	while (!windowShouldClose)
	{
		lastFrameTime = frameTime;
		frameTime = SDL_GetPerformanceCounter();
		deltaTime = (frameTime - lastFrameTime) / (double)SDL_GetPerformanceFrequency();

		if (SDL_GetTicks() % 10 == 0)
		{
			SDL_SetWindowTitle(window, ("SDL-Checkers | FPS: " + std::to_string((int)round(1.0 / deltaTime))).c_str());
		}

		while (SDL_PollEvent(&event))
		{
			switch (event.type)
			{
			case SDL_QUIT:
				windowShouldClose = true;
				break;
			case SDL_MOUSEMOTION:
				updateMouseCoords({event.button.x, event.button.y});
				break;
			case SDL_MOUSEBUTTONDOWN:
				if (event.button.button == SDL_BUTTON_LEFT)
				{
					makeMove();
				}
				break;
			default:
				break;
			}
		}

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		glLoadMatrixf(glm::value_ptr(camera.getProj()));

		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
		glLoadMatrixf(glm::value_ptr(camera.getModelView()));

		calcSelectedField(camera);
		animateCamera(camera, deltaTime);

		drawTable();
		drawPawns();

		SDL_GL_SwapWindow(window);
	}

	SDL_GL_DeleteContext(glContext);
	SDL_DestroyWindow(window);
	SDL_Quit();

	return 0;
}
