#include "Player.h"

#include <imgui/imgui.h>

#include "Level.h"
#include "Bullet.h"
#include "ShooterGame.h"

Player::Player()
	: m_ShootRate(0.25f)
	, m_MaxHealth(100.0f)
	, m_CurrentHealth(m_MaxHealth)
{
	SetSpeed(5.0f);
	SetSphereCollisionData(0.75f);
}

void Player::Init()
{
	Super::Init();

	SetName("PlayerShip");

	m_Level = GetLevel();

	m_ShipTexture = ZeoEngine::Texture2D::Create("assets/textures/Ship.png");
	SetTranslucent(m_ShipTexture->HasAlpha());

	m_BulletPool = ZeoEngine::CreateScope<BulletPool>(m_Level);
}

void Player::OnUpdate(ZeoEngine::DeltaTime dt)
{
	Super::OnUpdate(dt);

	// Movement control
	if (m_Level)
	{
		if (ZeoEngine::Input::IsKeyPressed(ZE_KEY_W))
		{
			SetPosition({ GetPosition().x, GetPosition().y + GetSpeed() * dt, GetPosition().z });
			if (GetPosition().y > m_Level->GetLevelBounds().top - 0.5f)
			{
				SetPosition({ GetPosition().x, m_Level->GetLevelBounds().top - 0.5f, GetPosition().z });
			}
		}
		if (ZeoEngine::Input::IsKeyPressed(ZE_KEY_A))
		{
			SetPosition({ GetPosition().x - GetSpeed() * dt, GetPosition().y, GetPosition().z });
			if (GetPosition().x < m_Level->GetLevelBounds().left + 0.5f)
			{
				SetPosition({ m_Level->GetLevelBounds().left + 0.5f, GetPosition().y, GetPosition().z });
			}
		}
		if (ZeoEngine::Input::IsKeyPressed(ZE_KEY_S))
		{
			SetPosition({ GetPosition().x, GetPosition().y - GetSpeed() * dt, GetPosition().z });
			if (GetPosition().y < m_Level->GetLevelBounds().bottom + 0.5f)
			{
				SetPosition({ GetPosition().x, m_Level->GetLevelBounds().bottom + 0.5f, GetPosition().z });
			}
		}
		if (ZeoEngine::Input::IsKeyPressed(ZE_KEY_D))
		{
			SetPosition({ GetPosition().x + GetSpeed() * dt, GetPosition().y, GetPosition().z });
			if (GetPosition().x > m_Level->GetLevelBounds().right - 0.5f)
			{
				SetPosition({ m_Level->GetLevelBounds().right - 0.5f, GetPosition().y, GetPosition().z });
			}
		}
	}

	// Shoot bullets
	{
		if (ZeoEngine::Input::IsKeyPressed(ZE_KEY_SPACE))
		{
			if (m_bCanShoot)
			{
				m_bCanShoot = false;
				GetTimerManager()->SetTimer(m_ShootRate, [&]() {
					m_bCanShoot = true;
				});

				SpawnBullet();
			}
		}
	}
}

void Player::SpawnBullet()
{
	// "Spawn" a bullet from pool
	Bullet* bullet = m_BulletPool->GetNextPooledObject();
	if (bullet)
	{
		bullet->SetPosition(GetPosition() + glm::vec3({ 0.0f, 0.5f, 0.0f }));
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

	ImGui::Begin("Player Stats");

	ImGui::Text("Health: %.f / %.f", m_CurrentHealth, m_MaxHealth);

	ImGui::End();
}

void Player::TakeDamage(GameObject* source, float damage)
{
	Super::TakeDamage(source, damage);

	m_CurrentHealth -= damage;
	if (m_CurrentHealth <= 0.0f)
	{
		Destroy();
	}
}
