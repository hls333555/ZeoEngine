#include "Player.h"

#include <imgui.h>

RTTR_REGISTRATION
{
	using namespace rttr;
	using namespace ZeoEngine;
	registration::class_<Player>("Player")
		.constructor(&Player::SpawnGameObject, policy::ctor::as_raw_ptr)
		.property("ShipFlameParticle", &Player::m_ShipFlameParticle)
		(
			metadata(PropertyMeta::Category, "FX")
		)
		.property("ExplosionParticle", &Player::m_ExplosionParticle)
		(
			metadata(PropertyMeta::Category, "FX")
		);
}

Player::Player()
	: m_ShootRate(0.25f)
	, m_MaxHealth(100.0f)
	, m_CurrentHealth(m_MaxHealth)
	, m_Score(0)
{
	SetSpeed(5.0f);
	SetCollisionType(ZeoEngine::ObjectCollisionType::Sphere);
	m_SpriteTexture = ZeoEngine::GetTexture2DLibrary()->GetOrLoad("assets/textures/Ship.png");
}

void Player::Init()
{
	Super::Init();

	FillSphereCollisionData(GetScale().x / 2.0f * 0.75f);

}

void Player::BeginPlay()
{
	Super::BeginPlay();

	m_BulletPool = ZeoEngine::CreateScope<BulletPool>(this);
	m_SpawnedShipFlameParticle = ZeoEngine::Level::Get().SpawnParticleSystemAttached(m_ShipFlameParticle, this);
}

void Player::OnUpdate(ZeoEngine::DeltaTime dt)
{
	Super::OnUpdate(dt);

	const auto& cameraBounds = ZeoEngine::GetActiveGameCamera()->GetCameraBounds();
	// Movement control
	if (ZeoEngine::Input::IsKeyPressed(ZE_KEY_W))
	{
		SetPosition2D(GetPosition2D() + GetForwardVector2D() * GetSpeed() * (float)dt);
		if (GetPosition().y > cameraBounds.Top - 0.5f)
		{
			SetPosition2D({ GetPosition().x, cameraBounds.Top - 0.5f });
		}
	}
	if (ZeoEngine::Input::IsKeyPressed(ZE_KEY_A))
	{
		SetPosition2D(GetPosition2D() - GetRightVector2D() * GetSpeed() * (float)dt);
		if (GetPosition().x < cameraBounds.Left + 0.5f)
		{
			SetPosition2D({ cameraBounds.Left + 0.5f, GetPosition().y });
		}
	}
	if (ZeoEngine::Input::IsKeyPressed(ZE_KEY_S))
	{
		SetPosition2D(GetPosition2D() - GetForwardVector2D() * GetSpeed() * (float)dt);
		if (GetPosition().y < cameraBounds.Bottom + 0.5f)
		{
			SetPosition2D({ GetPosition().x, cameraBounds.Bottom + 0.5f });
		}
	}
	if (ZeoEngine::Input::IsKeyPressed(ZE_KEY_D))
	{
		SetPosition2D(GetPosition2D() + GetRightVector2D() * GetSpeed() * (float)dt);
		if (GetPosition().x > cameraBounds.Right - 0.5f)
		{
			SetPosition2D({ cameraBounds.Right - 0.5f, GetPosition().y });
		}
	}

	// Shoot bullets
	{
		if (ZeoEngine::Input::IsKeyPressed(ZE_KEY_SPACE))
		{
			if (m_bCanShoot)
			{
				m_bCanShoot = false;
				ZeoEngine::GetTimerManager()->SetTimer(m_ShootRate, [&]() {
					m_bCanShoot = true;
				});

				SpawnBullet();
			}
		}
	}
}

void Player::OnImGuiRender()
{
	Super::OnImGuiRender();

	// TODO: Should be drawn in GameView window
	//auto pos = ImGui::GetWindowPos();
	//auto width = ZeoEngine::Application::Get().GetWindow().GetWidth();
	//auto height = ZeoEngine::Application::Get().GetWindow().GetHeight();
	//// Render health bar
	//{
	//	const float healthBarWidth = 200.0f;
	//	ImGui::GetWindowDrawList()->AddRectFilled(
	//		{ pos.x, pos.y - 15.0f },
	//		{ pos.x + m_CurrentHealth / m_MaxHealth * healthBarWidth, pos.y + 5.0f },
	//		IM_COL32(255, 0, 0, 255));
	//	ImGui::GetWindowDrawList()->AddRect(
	//		{ pos.x, pos.y - 15.0f },
	//		{ pos.x + healthBarWidth, pos.y + 5.0f },
	//		IM_COL32_BLACK);
	//}
	//// Render score text
	//{
	//	std::string scoreStr = std::string(u8"µÃ·Ö£º") + std::to_string(m_Score);
	//	ImGui::GetWindowDrawList()->AddText(m_Font, 30.0f,
	//		{ pos.x + width * 0.5f - 100.0f, pos.y - 20.0f },
	//		IM_COL32(255, 64, 0, 255), scoreStr.c_str());
	//}

}

void Player::TakeDamage(float damage, GameObject* causer, GameObject* instigator)
{
	Super::TakeDamage(damage, causer, instigator);

	m_CurrentHealth -= damage;
	if (m_CurrentHealth <= 0.0f)
	{
		Explode();
		SetActive(false);
	}
}

void Player::SpawnBullet()
{
	// "Spawn" a bullet from pool
	PlayerBullet* bullet = m_BulletPool->GetNextPooledObject();
	if (bullet)
	{
		bullet->SetPosition2D(GetPosition2D() + glm::vec2({ 0.0f, 0.5f }));
	}
}

void Player::Explode()
{
	ZeoEngine::Level::Get().SpawnParticleSystemAtPosition(m_ExplosionParticle, GetPosition2D());
	m_SpawnedShipFlameParticle->Deactivate();
	m_SpawnedShipFlameParticle->m_OnSystemFinished = std::bind([&]() {
		Destroy();
	});
}
