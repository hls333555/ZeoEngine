#include "Player.h"

#include "Level.h"
#include "Bullet.h"
#include "ShooterGame.h"

Player::Player()
	: m_ShootInterval(0.25f)
	, m_MaxHealth(100.0f)
	, m_CurrentHealth(0.0f)
{
	SetSpeed(5.0f);
}

void Player::Init()
{
	Super::Init();

	SetName("PlayerShip");

	m_Level = GetLevel();

	m_ShipTexture = ZeoEngine::Texture2D::Create("assets/textures/Ship.png");
	m_BulletPool = ZeoEngine::CreateScope<ObjectPooler<Bullet, 10>>();
}

void Player::OnUpdate(ZeoEngine::DeltaTime dt)
{
	Super::OnUpdate(dt);

	m_Time += dt;

	// Movement control
	if (m_Level)
	{
		if (ZeoEngine::Input::IsKeyPressed(ZE_KEY_W))
		{
			GetPosition().y += GetSpeed() * dt;
			if (GetPosition().y > m_Level->GetLevelBounds().top - 0.5f)
			{
				GetPosition().y = m_Level->GetLevelBounds().top - 0.5f;
			}
		}
		if (ZeoEngine::Input::IsKeyPressed(ZE_KEY_A))
		{
			GetPosition().x -= GetSpeed() * dt;
			if (GetPosition().x < m_Level->GetLevelBounds().left + 0.5f)
			{
				GetPosition().x = m_Level->GetLevelBounds().left + 0.5f;
			}
		}
		if (ZeoEngine::Input::IsKeyPressed(ZE_KEY_S))
		{
			GetPosition().y -= GetSpeed() * dt;
			if (GetPosition().y < m_Level->GetLevelBounds().bottom + 0.5f)
			{
				GetPosition().y = m_Level->GetLevelBounds().bottom + 0.5f;
			}
		}
		if (ZeoEngine::Input::IsKeyPressed(ZE_KEY_D))
		{
			GetPosition().x += GetSpeed() * dt;
			if (GetPosition().x > m_Level->GetLevelBounds().right - 0.5f)
			{
				GetPosition().x = m_Level->GetLevelBounds().right - 0.5f;
			}
		}
	}

	// Shoot bullets
	{
		if (!m_bCanShoot && m_Time - m_ShootTime > m_ShootInterval)
		{
			m_bCanShoot = true;
		}
		if (ZeoEngine::Input::IsKeyPressed(ZE_KEY_SPACE))
		{
			if (m_bCanShoot)
			{
				m_bCanShoot = false;
				m_ShootTime = m_Time;

				// "Spawn" a bullet from pool
				Bullet* bullet = m_BulletPool->GetNextPooledObject();
				if (bullet)
				{
					bullet->SetActive(true);
					bullet->SetPosition(GetPosition() + glm::vec2({ 0.0f, 0.5f }));
				}
			}
		}
	}
}

void Player::OnRender()
{
	Super::OnRender();

	ZeoEngine::Renderer2D::DrawQuad(GetPosition(), GetScale(), m_ShipTexture);
}

void Player::OnImGuiRender()
{
	Super::OnImGuiRender();

}
