#pragma once

#include <vector>

#include <glm/glm.hpp>

#include "Window.h"
#include "Camera.h"

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

struct Transform
{
	glm::vec3 position = {0.0f, 0.0f, 0.0f};
	glm::vec3 rotation = {0.0f, 0.0f, 0.0f};
	glm::vec3 scale = {1.0f, 1.0f, 1.0f};
};

struct LightSource
{
	glm::vec3 direction;
	float intensity;
};

class Renderer
{
public:
	explicit Renderer(const Window* const window);
	~Renderer();

public:
	void beginFrame(const Camera* const camera) const;

	void drawTriangles(const std::vector<Triangle>& triangles, const Transform& transform = {}) const;
	void drawTriangle(const Triangle& triangle, const Transform& transform = {}) const;
	void drawVertex(const Vertex& vertex, const Transform& transform = {}) const;

public:
	void setClearColor(const glm::vec3 clearColor);

	const LightSource& getLightSource() const { return m_lightSource; }
	void setLightSource(const LightSource& lightSource);

	const glm::vec3& getAmbientLight() const { return m_ambientLight; }
	void setAmbientLight(const glm::vec3& ambientLight) { m_ambientLight = ambientLight; }

private:
	SDL_GLContext m_glContext;

	glm::vec3 m_clearColor = {0.0f, 0.0f, 0.0f};
	LightSource m_lightSource = {{0.0f, -1.0f, 0.0f}, 1.0f};
	glm::vec3 m_ambientLight = {0.0f, 0.0f, 0.0f};
};
