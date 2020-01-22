#include "Player.h"

#include "Level.h"

Player::Player()
	: m_Position({0.0f, 0.0f})
	, m_Velocity(5.f)
{

}

void Player::Init(Level* level)
{
	m_Level = level;
	m_Position = { 0.0f, level->GetLevelBounds().z + 1.0f };
	m_ShipTexture = ZeoEngine::Texture2D::Create("assets/textures/Ship.png");
}

void Player::OnUpdate(ZeoEngine::DeltaTime dt)
{
	if (ZeoEngine::Input::IsKeyPressed(ZE_KEY_W))
	{
		m_Position.y += m_Velocity * dt;
		if (m_Position.y >= m_Level->GetLevelBounds().w - 0.5f)
		{
			m_Position.y = m_Level->GetLevelBounds().w - 0.5f;
		}
	}
	if (ZeoEngine::Input::IsKeyPressed(ZE_KEY_A))
	{
		m_Position.x -= m_Velocity * dt;
		if (m_Position.x <= m_Level->GetLevelBounds().x + 0.5f)
		{
			m_Position.x = m_Level->GetLevelBounds().x + 0.5f;
		}
	}
	if (ZeoEngine::Input::IsKeyPressed(ZE_KEY_S))
	{
		m_Position.y -= m_Velocity * dt;
		if (m_Position.y <= m_Level->GetLevelBounds().z + 0.5f)
		{
			m_Position.y = m_Level->GetLevelBounds().z + 0.5f;
		}
	}
	if (ZeoEngine::Input::IsKeyPressed(ZE_KEY_D))
	{
		m_Position.x += m_Velocity * dt;
		if (m_Position.x >= m_Level->GetLevelBounds().y - 0.5f)
		{
			m_Position.x = m_Level->GetLevelBounds().y - 0.5f;
		}
	}
}

void Player::OnRender()
{
	ZeoEngine::Renderer2D::DrawQuad(m_Position, { 1.0f, 1.0f }, m_ShipTexture);
}

void Player::OnImGuiRender()
{

}
