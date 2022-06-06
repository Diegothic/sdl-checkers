#pragma once

#include "Window.h"
#include "Renderer.h"
#include "Camera.h"
#include "MeshPrototypes.h"
#include "ManPiece.h"

class GameWindow : public Window
{
public:
	GameWindow(const char* title, const glm::uvec2& dimensions)
		: Window(title, dimensions),
		  m_renderer(this)
	{
	}

protected:
	void init() override
	{
		const glm::uvec2& windowDimensions = getDimensions();
		m_camera.setViewportDimensions({windowDimensions.x, windowDimensions.y});
		m_camera.makePerspective(45.0f, 1.0f, 1000.0f);
		m_camera.makeLookAt({0.0f, 0.0f, 1.0f}, {0.0f, 1.0f, 0.0f});
		m_camera.setPosition({0.0f, 0.0f, 10.0f});
		m_camera.setRotation({-40.0f, 0.0f, 0.0f});

		const LightSource light = {{0.3f, -1.0f, 0.45f}, 1.0f};

		m_renderer.setClearColor({0.15f, 0.15f, 0.15f});
		m_renderer.setLightSource(light);
		m_renderer.setAmbientLight({0.4f, 0.4f, 0.4f});
	}

	ManPiece piece = ManPiece(PieceType::Light);

	void update(const float& deltaTime) override
	{
		m_elapsed = m_elapsed + deltaTime;
		glm::vec3 camRot = m_camera.getRotation();
		camRot.y = 180.0f;
		m_camera.setRotation(camRot);

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

		piece.setInterpolationSpeed(15.0f);

		if (isMouseButtonDown(SDL_BUTTON_LEFT))
		{
			piece.setDesiredPosition(intersectUp);
		}
		if (isMouseButtonReleased(SDL_BUTTON_LEFT))
		{
			piece.setDesiredPosition(intersect);
		}
		piece.update(deltaTime);

		m_renderer.beginFrame(&m_camera);
		piece.render(m_renderer);

		Transform t1{};
		t1.position = glm::vec3(-2.0f, 0.1f, 0.0f);
		m_renderer.drawTriangles(m_prototypes.getPieceDarkNeutral(), t1);
		t1.position = glm::vec3(-1.0f, 0.1f, 0.0f);
		m_renderer.drawTriangles(m_prototypes.getPieceDarkMovable(), t1);
		t1.position = glm::vec3(0.0f, 0.1f, 0.0f);
		m_renderer.drawTriangles(m_prototypes.getPieceDarkSelected(), t1);
		t1.position = glm::vec3(1.0f, 0.1f, 0.0f);
		m_renderer.drawTriangles(m_prototypes.getPieceDarkCapture(), t1);

		t1.position = glm::vec3(-2.0f, 0.1f, 1.0f);
		m_renderer.drawTriangles(m_prototypes.getPieceLightNeutral(), t1);
		t1.position = glm::vec3(-1.0f, 0.1f, 1.0f);
		m_renderer.drawTriangles(m_prototypes.getPieceLightMovable(), t1);
		t1.position = glm::vec3(0.0f, 0.1f, 1.0f);
		m_renderer.drawTriangles(m_prototypes.getPieceLightSelected(), t1);
		t1.position = glm::vec3(1.0f, 0.1f, 1.0f);
		m_renderer.drawTriangles(m_prototypes.getPieceLightCapture(), t1);

		Transform t2{};
		for (int z = 0; z < 8; z++)
		{
			for (int x = 0; x < 8; x++)
			{
				const glm::vec3 pos = {x - 8 / 2.0f + 0.5f, -0.1f, z - 8 / 2.0f + 0.5f};
				t2.position = pos;
				if (z % 2 == 0 && x % 2 == 0 || z % 2 != 0 && x % 2 != 0)
				{
					m_renderer.drawTriangles(m_prototypes.getTileOdd(), t2);
				}
				else
				{
					m_renderer.drawTriangles(m_prototypes.getTileEven(), t2);
				}
			}
		}

		Transform t3{};
		t3.position = glm::vec3(0.0f, 2.0f, 0.0f);
		t3.rotation.x = glm::sin(m_elapsed * 8.0f) * 10.0f;
		t3.rotation.z = glm::cos(m_elapsed * 8.0f) * 10.0f;
		m_renderer.drawTriangles(m_prototypes.getPieceLightNeutral(), t3);
	}

	void onResize(const glm::uvec2& windowDimensions) override
	{
		Window::onResize(windowDimensions);
		m_camera.setViewportDimensions({windowDimensions.x, windowDimensions.y});
		m_camera.makePerspective(45.0f, 1.0f, 1000.0f);
		m_camera.makeLookAt({0.0f, 0.0f, 1.0f}, {0.0f, 1.0f, 0.0f});
	}

protected:
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
