#include "Obstacle.h"

#include "Player.h"

RTTR_REGISTRATION
{
	using namespace rttr;
	using namespace ZeoEngine;
	registration::class_<Obstacle>("Obstacle")
		.constructor(&Obstacle::SpawnGameObject, policy::ctor::as_raw_ptr);
}

Obstacle::Obstacle()
	: m_ScoreAmount(1)
{
	SetCollisionType(ZeoEngine::ObjectCollisionType::Sphere);
	SetGenerateOverlapEvents(true);
	m_SpriteTexture = ZeoEngine::Texture2D::Create("assets/textures/Obstacle.png");
}

void Obstacle::Init()
{
	Super::Init();

	FillSphereCollisionData(GetScale().x / 2.0f * 0.75f);

	m_ExplosionTexture = ZeoEngine::GetTexture2DLibrary()->Get("assets/textures/Explosion_2x4.png");

}

void Obstacle::OnUpdate(ZeoEngine::DeltaTime dt)
{
	Super::OnUpdate(dt);

	SetRotation(GetRotation(false) + m_RotationSpeed * dt);

	SetPosition2D(GetPosition2D() - WORLD_UP_VECTOR * GetSpeed() * (float)dt);
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
	ZeoEngine::ParticleTemplate m_ExplosionEmitter;
	m_ExplosionEmitter.loopCount = 1;
	m_ExplosionEmitter.lifeTime.SetConstant(0.4f);
	m_ExplosionEmitter.AddBurstData(0.0f, 1);
	m_ExplosionEmitter.initialPosition.SetConstant(GetPosition2D());
	m_ExplosionEmitter.sizeBegin.SetConstant(GetScale());
	m_ExplosionEmitter.sizeEnd.SetConstant(GetScale());
	m_ExplosionEmitter.texture = m_ExplosionTexture;
	m_ExplosionEmitter.subImageSize = { 4, 2 };
	m_ExplosionParticle = ZeoEngine::Level::Get().SpawnParticleSystem(m_ExplosionEmitter);
}
