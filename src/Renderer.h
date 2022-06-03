#pragma once

#include <vector>

#include <sdl/SDL_opengl.h>
#include <glm/glm.hpp>

#include "Window.h"

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
	explicit Renderer(const Window* const window)
	{
		m_glContext = window->createContext();
		glEnable(GL_DEPTH_TEST);
	}

	~Renderer()
	{
		SDL_GL_DeleteContext(m_glContext);
	}

public:
	void beginFrame(const Camera* const camera) const
	{
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		glLoadMatrixf(glm::value_ptr(camera->getProj()));

		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
		glLoadMatrixf(glm::value_ptr(camera->getModelView()));
	}

	void drawTriangles(const std::vector<Triangle>& triangles) const
	{
		for (const auto& triangle : triangles)
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
	void setClearColor(const glm::vec3 clearColor)
	{
		m_clearColor = clearColor;
		glClearColor(m_clearColor.r, m_clearColor.g, m_clearColor.b, 1.0);
	}

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
	SDL_GLContext m_glContext;

	glm::vec3 m_clearColor = {0.0f, 0.0f, 0.0f};
	LightSource m_lightSource = {{0.0f, -1.0f, 0.0f}, 1.0f};
	glm::vec3 m_ambientLight = {0.0f, 0.0f, 0.0f};
};
