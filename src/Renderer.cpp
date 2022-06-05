#include "Renderer.h"

#include <sdl/SDL_opengl.h>
#include <glm/gtc/type_ptr.hpp>

Renderer::Renderer(const Window* const window)
{
	m_glContext = window->createContext();
	glEnable(GL_DEPTH_TEST);
}

Renderer::~Renderer()
{
	SDL_GL_DeleteContext(m_glContext);
}

void Renderer::beginFrame(const Camera* const camera) const
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glLoadMatrixf(glm::value_ptr(camera->getProj()));

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glLoadMatrixf(glm::value_ptr(camera->getModelView()));
}

void Renderer::drawTriangles(
	const std::vector<Triangle>& triangles,
	const Transform& transform
) const
{
	for (const auto& triangle : triangles)
	{
		drawTriangle(triangle, transform);
	}
}

void Renderer::drawTriangle(
	const Triangle& triangle,
	const Transform& transform
) const
{
	glBegin(GL_TRIANGLES);

	drawVertex(triangle.v1, transform);
	drawVertex(triangle.v2, transform);
	drawVertex(triangle.v3, transform);

	glEnd();
}

void Renderer::drawVertex(
	const Vertex& vertex,
	const Transform& transform
) const
{
	glm::mat4 model = glm::mat4(1.0f);
	model = glm::scale(model, transform.scale);
	model = glm::rotate(
		model,
		glm::radians(transform.rotation.x),
		{1.0f, 0.0f, 0.0f}
	);
	model = glm::rotate(
		model,
		glm::radians(transform.rotation.y),
		{0.0f, 1.0f, 0.0f}
	);
	model = glm::rotate(
		model,
		glm::radians(transform.rotation.z),
		{0.0f, 0.0f, 1.0f}
	);
	model = glm::translate(model, transform.position);

	glm::vec4 position = {
		vertex.position.x,
		vertex.position.y,
		vertex.position.z,
		1.0f
	};
	glm::vec4 normal = {
		vertex.normal.x,
		vertex.normal.y,
		vertex.normal.z,
		1.0f
	};
	position = model * position;
	normal = model * normal;

	const glm::vec3 normalized = glm::normalize(glm::vec3(normal.x, normal.y, normal.z));
	const float dot = glm::dot(m_lightSource.direction, normalized);
	float intensity = m_lightSource.intensity * 0.5f * (1.0f - dot);
	intensity = glm::clamp(intensity, 0.0f, 1.0f);
	const glm::vec3 color = vertex.color * m_ambientLight + vertex.color * intensity;

	glColor3f(color.r, color.g, color.b);
	glVertex3f(position.x, position.y, position.z);
}

void Renderer::setClearColor(const glm::vec3 clearColor)
{
	m_clearColor = clearColor;
	glClearColor(
		m_clearColor.r,
		m_clearColor.g,
		m_clearColor.b,
		1.0f
	);
}

void Renderer::setLightSource(const LightSource& lightSource)
{
	m_lightSource = lightSource;
	m_lightSource.intensity = glm::clamp(m_lightSource.intensity, 0.0f, 1.0f);
}
