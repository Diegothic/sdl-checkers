#pragma once

#include "Window.h"
#include "Renderer.h"
#include "Camera.h"

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

		m_renderer.setClearColor({0.2f, 0.2f, 0.2f});
		m_renderer.setLightSource(light);
		m_renderer.setAmbientLight({0.15f, 0.15f, 0.15f});
	}

	void update(const float& deltaTime) override
	{
		m_elapsed = m_elapsed + deltaTime;
		m_renderer.beginFrame(&m_camera);
		glm::vec3 color = {
			glm::sin((m_elapsed + 1.0f) * 0.5f),
			glm::cos((m_elapsed + 1.0f) * 0.5f),
			glm::sin((-m_elapsed + 1.0f) * 0.5f)
		};
		color = glm::vec3(1.0f) - color;
		color.x = glm::min(color.x, 0.8f);
		color.y = glm::min(color.y, 0.8f);
		color.z = glm::min(color.z, 0.8f);
		std::vector<Triangle> donut = makeDonut(
			color,
			1.0f + 1.0f * glm::sin(m_elapsed),
			0.5f + 0.5f * glm::cos(m_elapsed)
		);
		Transform donutTransform = {};
		donutTransform.rotation.z = m_elapsed * 45.0f;
		donutTransform.rotation.x = m_elapsed * 60.0f;
		donutTransform.rotation.y = m_elapsed * 90.0f;
		m_renderer.drawTriangles(donut, donutTransform);
	}

	void onResize(const glm::uvec2& windowDimensions) override
	{
		Window::onResize(windowDimensions);
		m_camera.setViewportDimensions({windowDimensions.x, windowDimensions.y});
		m_camera.makePerspective(45.0f, 1.0f, 1000.0f);
		m_camera.makeLookAt({0.0f, 0.0f, 1.0f}, {0.0f, 1.0f, 0.0f});
	}

	std::vector<Triangle> makePawn(glm::vec3 color) const
	{
		std::vector<Triangle> triangles;
		constexpr float step = glm::pi<float>() / 10.0f;

		for (float i = 0.0; i < glm::pi<float>() * 2.0f; i += step)
		{
			constexpr float r = 0.35f;
			constexpr float h = 0.1f;
			const float x = glm::sin(i);
			const float y = glm::cos(i);

			const float xN = glm::sin(i + step);
			const float yN = glm::cos(i + step);

			Vertex vertices[10];

			for (auto& vertex : vertices)
			{
				vertex.color = color;
			}

			// TOP
			vertices[0].position = {0.0f, h, 0.0f};
			vertices[1].position = {x * r, h, y * r};
			vertices[2].position = {xN * r, h, yN * r};

			vertices[0].normal = {0.0f, 1.0f, 0.0f};
			vertices[1].normal = {0.0f, 1.0f, 0.0f};
			vertices[2].normal = {0.0f, 1.0f, 0.0f};

			triangles.push_back({vertices[0], vertices[1], vertices[2]});

			// SIDE
			vertices[3].position = {x * r, h, y * r};
			vertices[4].position = {x * r, -h, y * r};
			vertices[5].position = {xN * r, -h, yN * r};
			vertices[6].position = {xN * r, h, yN * r};

			vertices[3].normal = {x * r, 0.0f, y * r};
			vertices[4].normal = {x * r, 0.0f, y * r};
			vertices[5].normal = {xN * r, 0.0f, yN * r};
			vertices[6].normal = {xN * r, 0.0f, yN * r};

			triangles.push_back({vertices[3], vertices[4], vertices[6]});
			triangles.push_back({vertices[4], vertices[5], vertices[6]});


			// BOTTOM
			vertices[7].position = {0.0f, -h, 0.0f};
			vertices[8].position = {x * r, -h, y * r};
			vertices[9].position = {xN * r, -h, yN * r};

			vertices[7].normal = {0.0f, -1.0f, 0.0f};
			vertices[8].normal = {0.0f, -1.0f, 0.0f};
			vertices[9].normal = {0.0f, -1.0f, 0.0f};

			triangles.push_back({vertices[7], vertices[8], vertices[9]});
		}
		return triangles;
	}

	std::vector<Triangle> makeDonut(glm::vec3 color, float R, float r) const
	{
		std::vector<Triangle> triangles;
		constexpr float step = glm::pi<float>() / 16.0f;

		for (float i = 0.0; i < glm::pi<float>() * 2.0f; i += step)
		{
			for (float j = 0.0; j < glm::pi<float>() * 2.0f; j += step)
			{
				const float Rx = R * glm::sin(i);
				const float Ry = R * glm::cos(i);

				const float x = Rx + r * glm::cos(j) * glm::sin(i);
				const float y = Ry + r * glm::cos(j) * glm::cos(i);
				const float z = r * glm::sin(j);

				const float xN = Rx + r * glm::cos(j + step) * glm::sin(i);
				const float yN = Ry + r * glm::cos(j + step) * glm::cos(i);
				const float zN = r * glm::sin(j + step);

				const float R_x = R * glm::sin(i + step);
				const float R_y = R * glm::cos(i + step);

				const float _x = R_x + r * glm::cos(j) * glm::sin(i + step);
				const float _y = R_y + r * glm::cos(j) * glm::cos(i + step);
				const float _z = r * glm::sin(j);

				const float _xN = R_x + r * glm::cos(j + step) * glm::sin(i + step);
				const float _yN = R_y + r * glm::cos(j + step) * glm::cos(i + step);
				const float _zN = r * glm::sin(j + step);

				Vertex vertices[4];
				for (auto& vertex : vertices)
				{
					vertex.color = color;
				}
				vertices[0].position = {x, y, z};
				vertices[1].position = {xN, yN, zN};
				vertices[2].position = {_x, _y, _z};
				vertices[3].position = {_xN, _yN, _zN};

				vertices[0].normal = {
					glm::cos(j) * glm::sin(i),
					glm::cos(j) * glm::cos(i),
					glm::sin(j)
				};
				vertices[1].normal = {
					glm::cos(j + step) * glm::sin(i),
					glm::cos(j + step) * glm::cos(i),
					glm::sin(j + step)
				};
				vertices[2].normal = {
					glm::cos(j) * glm::sin(i + step),
					glm::cos(j) * glm::cos(i + step),
					glm::sin(j)
				};
				vertices[3].normal = {
					glm::cos(j + step) * glm::sin(i + step),
					glm::cos(j + step) * glm::cos(i + step),
					glm::sin(j + step)
				};

				triangles.push_back({vertices[0], vertices[1], vertices[2]});
				triangles.push_back({vertices[2], vertices[1], vertices[3]});
			}
		}
		return triangles;
	}

private:
	Renderer m_renderer;
	Camera m_camera;

	float m_elapsed = 0.0;
};
