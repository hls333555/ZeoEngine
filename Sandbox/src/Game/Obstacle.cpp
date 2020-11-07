#include "Obstacle.h"

#include "Player.h"

RTTR_REGISTRATION
{
	using namespace rttr;
	using namespace ZeoEngine;
	registration::class_<Obstacle>("Obstacle")
		.constructor(&Obstacle::SpawnGameObject, policy::ctor::as_raw_ptr)
		.property("ExplosionParticle", &Obstacle::m_ExplosionParticle)
		(
			metadata(PropertyMeta::Category, "FX")
		);
}

Obstacle::Obstacle()
	: m_ScoreAmount(1)
{
	SetCollisionType(ZeoEngine::ObjectCollisionType::Sphere);
	SetGenerateOverlapEvents(true);
	m_SpriteTexture = ZeoEngine::Texture2DLibrary::Get().GetOrLoad("../Sandbox/assets/textures/Obstacle.png");
	//m_ExplosionParticle = ZeoEngine::GetParticleLibrary()->GetOrLoad("../Sandbox/assets/particles/Explosion.zparticle");
}

void Obstacle::Init()
{
	Super::Init();

	FillSphereCollisionData(GetScale().x / 2.0f * 0.75f);

}

void Obstacle::OnUpdate(ZeoEngine::DeltaTime dt)
{
	Super::OnUpdate(dt);

	SetRotation(GetRotation(false) + m_RotationSpeed * dt);

	SetPosition2D(GetPosition2D() - GetWorldUpVector2D() * GetSpeed() * static_cast<float>(dt));
	if (GetPosition().y < ZeoEngine::GetActiveGameCamera()->GetCameraBounds().Bottom - 0.5f)
	{
		// Lifetime is determined by ObjectPooler
		SetActive(false);
	}
}

void Obstacle::Reset()
{
	Super::Reset();

	m_MaxHealth = ZeoEngine::RandomEngine::RandFloatInRange(10.0f, 50.0f);
	m_CurrentHealth = m_MaxHealth;
	m_ExplosionDamage = ZeoEngine::RandomEngine::RandFloatInRange(5.0f, 20.0f);
	m_RotationSpeed = ZeoEngine::RandomEngine::RandFloatInRange(-10.0f, 10.0f);
	const auto& cameraBounds = ZeoEngine::GetActiveGameCamera()->GetCameraBounds();
	SetPosition2D({ ZeoEngine::RandomEngine::RandFloatInRange(cameraBounds.Right - 0.5f, cameraBounds.Left + 0.5f), cameraBounds.Top });
	SetRotation(ZeoEngine::RandomEngine::RandFloatInRange(0.0f, 180.0f));
	SetScale(ZeoEngine::RandomEngine::RandFloatInRange(0.75f, 1.8f));
	FillSphereCollisionData(GetScale().x / 2.0f * 0.75f);
	SetSpeed(ZeoEngine::RandomEngine::RandFloatInRange(0.5f, 1.5f));
}

void Obstacle::TakeDamage(float damage, GameObject* causer, GameObject* instigator)
{
	Super::TakeDamage(damage, causer, instigator);

	m_CurrentHealth -= damage;
	if (m_CurrentHealth <= 0.0f)
	{
		Player* player = dynamic_cast<Player*>(instigator);
		if (player)
		{
			player->AddScore(m_ScoreAmount);
		}
		// Lifetime is determined by ObjectPooler
		SetActive(false);
		Explode();
	}
}

void Obstacle::OnOverlap(GameObject* other)
{
	Super::OnOverlap(other);

	if (Player* player = dynamic_cast<Player*>(other))
	{
		ApplyDamage(m_ExplosionDamage, other, this, this);
		if (player)
		{
			player->AddScore(-m_ScoreAmount);
		}
		// Lifetime is determined by ObjectPooler
		SetActive(false);
		Explode();
	}
}

void Obstacle::Explode()
{
	ZeoEngine::Level::Get().SpawnParticleSystemAtPosition(m_ExplosionParticle, GetPosition2D());
}
