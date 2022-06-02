#pragma once

#include <vector>

#include <sdl/SDL_opengl.h>
#include <glm/glm.hpp>

struct Vertex
{
	glm::vec3 position;
	glm::vec3 normal;
	glm::vec3 color;
};

struct Triangle
{
	Vertex v1, v2, v3;
};

struct LightSource
{
	glm::vec3 direction;
	float intensity;
};

class Renderer
{
public:
	Renderer() = default;

public:
	void drawTriangles(const std::vector<Triangle>& triangles) const
	{
		for(const auto& triangle : triangles)
		{
			drawTriangle(triangle);
		}
	}

	void drawTriangle(const Triangle& triangle) const
	{
		glBegin(GL_TRIANGLES);

		drawVertex(triangle.v1);
		drawVertex(triangle.v2);
		drawVertex(triangle.v3);

		glEnd();
	}

	void drawVertex(const Vertex& vertex) const
	{
		const float dot = glm::dot(m_lightSource.direction, glm::normalize(vertex.normal));
		float intensity = m_lightSource.intensity * 0.5f * (1.0f - dot);
		intensity = glm::clamp(intensity, 0.0f, 1.0f);
		const glm::vec3 color = vertex.color * m_ambientLight + vertex.color * intensity;

		glColor3f(color.r, color.g, color.b);
		glVertex3f(vertex.position.x, vertex.position.y, vertex.position.z);
	}

public:
	const LightSource& getLightSource() const
	{
		return m_lightSource;
	}

	void setLightSource(const LightSource& lightSource)
	{
		m_lightSource = lightSource;
		m_lightSource.intensity = glm::clamp(m_lightSource.intensity, 0.0f, 1.0f);
	}

	const glm::vec3& getAmbientLight() const
	{
		return m_ambientLight;
	}

	void setAmbientLight(const glm::vec3& ambientLight)
	{
		m_ambientLight = ambientLight;
	}

private:
	LightSource m_lightSource = {{0.0f, -1.0f, 0.0f}, 1.0f};
	glm::vec3 m_ambientLight = {0.0f, 0.0f, 0.0f};
};
