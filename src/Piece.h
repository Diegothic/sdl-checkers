#pragma once

#include <vector>

#include <glm/glm.hpp>

#include "PieceTypes.h"
#include "Renderer.h"

class Piece
{
public:
	Piece(PieceType type) : m_type(type)
	{
	}

	virtual ~Piece() = default;

public:
	virtual void update(float deltaTime)
	{
		interpolatePosition(m_interSpeed * deltaTime);
	}

	virtual void render(const Renderer& renderer) const = 0;

	virtual std::vector<glm::ivec2> getViableMoves() const = 0;

private:
	void interpolatePosition(float speed)
	{
		const glm::vec3 movementVector = m_desiredPosition - m_transform.position;
		const float movementVectorLength = glm::length(movementVector);
		glm::vec3 desiredMovementVector = glm::normalize(movementVector) * speed;
		desiredMovementVector += desiredMovementVector * movementVectorLength;
		const float desiredMovementVectorLength = glm::length(desiredMovementVector);
		if (desiredMovementVectorLength < movementVectorLength)
		{
			m_transform.position += desiredMovementVector;
		}
		else
		{
			m_transform.position = m_desiredPosition;
		}

		if (m_isHeld)
		{
			m_transform.rotation = {movementVector.z * 100.0f, 0.0f, -movementVector.x * 100.0f};
			m_transform.rotation.x = glm::clamp(m_transform.rotation.x, -60.0f, 60.0f);
			m_transform.rotation.z = glm::clamp(m_transform.rotation.z, -60.0f, 60.0f);
		}
		else
		{
			m_transform.rotation = {0.0f, 0.0f, 0.0f};
		}
	}

public:
	const Transform& getTransform() const { return m_transform; }
	void setDesiredPosition(const glm::vec3& position) { m_desiredPosition = position; }

	void setInterpolationSpeed(const float& interSpeed) { m_interSpeed = interSpeed; }

	const PieceType& getType() const { return m_type; }

	const PieceState& getState() const { return m_state; }
	void setState(PieceState state) { m_state = state; }

	void setHeld(const bool& held) { m_isHeld = held; }

	const bool& isMovable() const { return m_isMovable; }
	void setMovable(const bool& movable) { m_isMovable = movable; }

private:
	Transform m_transform = {};
	glm::vec3 m_desiredPosition = {};

	float m_interSpeed = 1.0f;

	PieceType m_type;
	PieceState m_state = PieceState::Neutral;

	bool m_isHeld = false;
	bool m_isMovable = false;
};
