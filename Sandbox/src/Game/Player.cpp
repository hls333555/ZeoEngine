#include "Player.h"

#include "Engine/ImGui/MyImGui.h"

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
	m_SpriteTexture = ZeoEngine::Texture2DLibrary::Get().GetOrLoad("../Sandbox/assets/textures/Ship.png");
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
	if (ZeoEngine::Input::IsKeyPressed(ZeoEngine::Key::W))
	{
		SetPosition2D(GetPosition2D() + GetForwardVector2D() * GetSpeed() * static_cast<float>(dt));
		if (GetPosition().y > cameraBounds.Top - 0.5f)
		{
			SetPosition2D({ GetPosition().x, cameraBounds.Top - 0.5f });
		}
	}
	if (ZeoEngine::Input::IsKeyPressed(ZeoEngine::Key::A))
	{
		SetPosition2D(GetPosition2D() - GetRightVector2D() * GetSpeed() * static_cast<float>(dt));
		if (GetPosition().x < cameraBounds.Left + 0.5f)
		{
			SetPosition2D({ cameraBounds.Left + 0.5f, GetPosition().y });
		}
	}
	if (ZeoEngine::Input::IsKeyPressed(ZeoEngine::Key::S))
	{
		SetPosition2D(GetPosition2D() - GetForwardVector2D() * GetSpeed() * static_cast<float>(dt));
		if (GetPosition().y < cameraBounds.Bottom + 0.5f)
		{
			SetPosition2D({ GetPosition().x, cameraBounds.Bottom + 0.5f });
		}
	}
	if (ZeoEngine::Input::IsKeyPressed(ZeoEngine::Key::D))
	{
		SetPosition2D(GetPosition2D() + GetRightVector2D() * GetSpeed() * static_cast<float>(dt));
		if (GetPosition().x > cameraBounds.Right - 0.5f)
		{
			SetPosition2D({ cameraBounds.Right - 0.5f, GetPosition().y });
		}
	}

	// Shoot bullets
	{
		if (ZeoEngine::Input::IsKeyPressed(ZeoEngine::Key::Space))
		{
			if (m_bCanShoot)
			{
				m_bCanShoot = false;
				ZeoEngine::GetTimerManager()->SetTimer(m_ShootRate, this, [&]() {
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
	
	// TODO: HealthBar may disappear on window resize
	if (HasBegunPlay())
	{
		const float healthBarWidth = 200.0f;
#if WITH_EDITOR
		if (ImGui::Begin("Game View"))
		{
			ImVec2 pos = ImGui::GetWindowPos();
			ImGui::AddProgressBar(ImGui::GetWindowDrawList(), m_CurrentHealth / m_MaxHealth,
				{ pos.x + 50.0f, pos.y + 50.0f },
				{ pos.x + 50.0f + healthBarWidth, pos.y + 70.0f },
				ImGui::GetColorU32(ImGuiCol_PlotHistogram), ImGui::GetColorU32(ImGuiCol_FrameBg));
			ImGui::End();
		}
#else
		ImGui::AddProgressBar(ImGui::GetForegroundDrawList(), m_CurrentHealth / m_MaxHealth,
			{ 50.0f, 50.0f },
			{ 50.0f + healthBarWidth, 70.0f },
			ImGui::GetColorU32(ImGuiCol_PlotHistogram), ImGui::GetColorU32(ImGuiCol_FrameBg));
#endif
	}

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
	m_SpawnedShipFlameParticle->m_OnSystemFinished += GET_MEMBER_FUNC(static_cast<GameObject*>(this), &Player::Destroy);
}
