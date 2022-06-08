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

	Piece(const Piece& other)
	{
		m_viableMoves = other.m_viableMoves;
		m_capturesCount = other.m_capturesCount;
		m_transform = other.m_transform;
		m_desiredPosition = other.m_desiredPosition;
		m_interSpeed = other.m_interSpeed;
		m_type = other.m_type;
		m_state = other.m_state;
		m_isHeld = other.m_isHeld;
		m_isMovable = other.m_isMovable;
		m_isCaptured = other.m_isCaptured;
	}

	virtual ~Piece() = default;

public:
	virtual void update(float deltaTime)
	{
		interpolatePosition(m_interSpeed * deltaTime);
	}

	virtual void render(const Renderer& renderer) const = 0;

	virtual void recalculateMoves(
		const glm::ivec2& coords,
		Piece*** board,
		int boardSize
	)
	{
		m_viableMoves.clear();
		m_capturesCount = 0;
	}

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

	const bool& isCaptured() const { return m_isCaptured; }
	void setCaptured(const bool& captured) { m_isCaptured = captured; }

	const std::vector<Move>& getViableMoves() const { return m_viableMoves; }
	const int& getCapturesCount() const { return m_capturesCount; }

protected:
	std::vector<Move> m_viableMoves;
	int m_capturesCount = 0;

private:
	Transform m_transform = {};
	glm::vec3 m_desiredPosition = {};

	float m_interSpeed = 1.0f;

	PieceType m_type;
	PieceState m_state = PieceState::Neutral;

	bool m_isHeld = false;
	bool m_isMovable = false;
	bool m_isCaptured = false;
};
