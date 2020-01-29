#include "Player.h"

#include <imgui/imgui.h>

#include "GameLevel.h"

Player::Player()
	: m_ShootRate(0.25f)
	, m_MaxHealth(100.0f)
	, m_CurrentHealth(m_MaxHealth)
	, m_Score(0)
{
	SetSpeed(5.0f);
	SetSphereCollisionData(0.75f);

}

void Player::Init()
{
	Super::Init();

	SetName("PlayerShip");

	m_Level = ZeoEngine::GetLevel<GameLevel>();
	m_Font = ZeoEngine::GetFont();

	m_PlayerTexture = ZeoEngine::Texture2D::Create("assets/textures/Ship.png");
	SetTranslucent(m_PlayerTexture->HasAlpha());

	m_ExplosionTexture = ZeoEngine::GetTexture2DLibrary()->Get("assets/textures/Explosion_2x4.png");

	m_BulletPool = ZeoEngine::CreateScope<BulletPool>(m_Level, this);

	ZeoEngine::ParticleTemplate m_FlameEmitter;
	m_FlameEmitter.lifeTime.SetRandom(0.75f, 1.5f);
	m_FlameEmitter.spawnRate.SetConstant(30.0f);
	m_FlameEmitter.initialPosition.SetConstant(glm::vec2(0.0f, -0.45f));
	m_FlameEmitter.initialRotation.SetRandom(0.0f, 360.0f);
	m_FlameEmitter.rotationRate.SetRandom(10.0f, 50.0f);
	m_FlameEmitter.initialVelocity.SetRandom({ -0.5f, -0.5f }, { 0.5f, -2.0f });
	m_FlameEmitter.inheritVelocity = { 0.1f, 0.1f };
	m_FlameEmitter.sizeBegin.SetRandom(0.075f, 0.2f);
	m_FlameEmitter.sizeEnd.SetConstant({ 0.0f, 0.0f });
	m_FlameEmitter.colorBegin.SetRandom({ 0.9f, 0.4f, 0.2f, 1.0f }, { 0.9f, 0.8f, 0.2f, 1.0f });
	m_FlameEmitter.colorEnd.SetConstant({ 0.9f, 0.8f, 0.5f, 0.0f });
	m_FlameParticle = m_Level->SpawnParticleSystem(m_FlameEmitter, this);

}

void Player::OnUpdate(ZeoEngine::DeltaTime dt)
{
	Super::OnUpdate(dt);

	// Movement control
	if (m_Level)
	{
		if (ZeoEngine::Input::IsKeyPressed(ZE_KEY_W))
		{
			SetPosition2D(GetPosition2D() + GetForwardVector() * GetSpeed() * (float)dt);
			if (GetPosition().y > m_Level->GetLevelBounds().top - 0.5f)
			{
				SetPosition2D({ GetPosition().x, m_Level->GetLevelBounds().top - 0.5f });
			}
		}
		if (ZeoEngine::Input::IsKeyPressed(ZE_KEY_A))
		{
			SetPosition2D(GetPosition2D() - GetRightVector() * GetSpeed() * (float)dt);
			if (GetPosition().x < m_Level->GetLevelBounds().left + 0.5f)
			{
				SetPosition2D({ m_Level->GetLevelBounds().left + 0.5f, GetPosition().y });
			}
		}
		if (ZeoEngine::Input::IsKeyPressed(ZE_KEY_S))
		{
			SetPosition2D(GetPosition2D() - GetForwardVector() * GetSpeed() * (float)dt);
			if (GetPosition().y < m_Level->GetLevelBounds().bottom + 0.5f)
			{
				SetPosition2D({ GetPosition().x, m_Level->GetLevelBounds().bottom + 0.5f });
			}
		}
		if (ZeoEngine::Input::IsKeyPressed(ZE_KEY_D))
		{
			SetPosition2D(GetPosition2D() + GetRightVector() * GetSpeed() * (float)dt);
			if (GetPosition().x > m_Level->GetLevelBounds().right - 0.5f)
			{
				SetPosition2D({ m_Level->GetLevelBounds().right - 0.5f, GetPosition().y });
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
				ZeoEngine::GetTimerManager()->SetTimer(m_ShootRate, [&]() {
					m_bCanShoot = true;
				});

				SpawnBullet();
			}
		}
	}
}

void Player::OnRender()
{
	Super::OnRender();

	ZeoEngine::Renderer2D::DrawQuad(GetPosition(), GetScale(), m_PlayerTexture);
}

void Player::OnImGuiRender()
{
	Super::OnImGuiRender();

	// TODO: Should be draw in GameView window
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
	ZeoEngine::ParticleTemplate m_ExplosionEmitter;
	m_ExplosionEmitter.loopCount = 1;
	m_ExplosionEmitter.lifeTime.SetConstant(0.4f);
	m_ExplosionEmitter.AddBurstData(0.0f, 1);
	m_ExplosionEmitter.initialPosition.SetConstant(GetPosition2D());
	m_ExplosionEmitter.sizeBegin.SetConstant(GetScale());
	m_ExplosionEmitter.sizeEnd.SetConstant(GetScale());
	m_ExplosionEmitter.texture = m_ExplosionTexture;
	m_ExplosionEmitter.subImageSize = { 4, 2 };
	m_ExplosionParticle = m_Level->SpawnParticleSystem(m_ExplosionEmitter);

	m_FlameParticle->Deactivate();
	m_FlameParticle->OnSystemFinished = std::bind([&]() {
		Destroy();
	});
}
