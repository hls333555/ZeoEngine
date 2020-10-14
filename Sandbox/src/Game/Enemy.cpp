#include "Enemy.h"

#include "Player.h"

RTTR_REGISTRATION
{
	using namespace rttr;
	using namespace ZeoEngine;
	registration::class_<Enemy>("Enemy")
		.constructor(&Enemy::SpawnGameObject, policy::ctor::as_raw_ptr)
		.property("ExplosionParticle", &Enemy::m_ExplosionParticle)
		(
			metadata(PropertyMeta::Category, "FX")
		);
}

Enemy::Enemy()
	: m_MaxHealth(ZeoEngine::RandomEngine::RandFloatInRange(30.0f, 80.0f))
	, m_CurrentHealth(m_MaxHealth)
	, m_ExplosionDamage(ZeoEngine::RandomEngine::RandFloatInRange(5.0f, 20.0f))
	, m_ScoreAmount(3)
	, m_ShootRate(0.75f)
{
	SetSpeed(2.0f);
	SetCollisionType(ZeoEngine::ObjectCollisionType::Sphere);
	SetGenerateOverlapEvents(true);
	m_SpriteTexture = ZeoEngine::Texture2DLibrary::Get().GetOrLoad("../Sandbox/assets/textures/Ship2.png");
	m_ExplosionParticle = ZeoEngine::GetParticleLibrary()->GetOrLoad("../Sandbox/assets/particles/Explosion.zparticle");
}

void Enemy::Init()
{
	Super::Init();

	FillSphereCollisionData(GetScale().x / 2.0f * 0.75f);

}

void Enemy::BeginPlay()
{
	Super::BeginPlay();

	m_BulletPool = ZeoEngine::CreateScope<BulletPool>(this);

	// Move to a random position of upper half screen nearby every 2 seconds
	ZeoEngine::GetTimerManager()->SetTimer(2.0f, this, [&]() {
		glm::vec2 targetPosition = GetRandomPositionInRange(GetPosition2D(), { 2.0f, 2.0f });
		if (targetPosition.y > 0 && glm::length(targetPosition - GetPosition2D()) > 1.0f)
		{
			TranslateTo2D(targetPosition);
		}
	}, 0, 0.0f);

}

void Enemy::OnUpdate(ZeoEngine::DeltaTime dt)
{
	Super::OnUpdate(dt);

	if (m_bCanShoot)
	{
		m_bCanShoot = false;
		ZeoEngine::GetTimerManager()->SetTimer(m_ShootRate, this, [&]() {
			m_bCanShoot = true;
		});

		SpawnBullet();
	}
}

void Enemy::TakeDamage(float damage, GameObject* causer, GameObject* instigator)
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
		Explode();
		Destroy();
	}
}

void Enemy::OnOverlap(GameObject* other)
{
	Super::OnOverlap(other);

	if (Player* player = dynamic_cast<Player*>(other))
	{
		ApplyDamage(m_ExplosionDamage, other, this, this);
		if (player)
		{
			player->AddScore(-m_ScoreAmount);
		}
		Explode();
		Destroy();
	}
}

glm::vec2 Enemy::GetRandomPositionInRange(const glm::vec2& center, const glm::vec2& extents)
{
	float lowerX = std::max(center.x - extents.x, ZeoEngine::GetActiveGameCamera()->GetCameraBounds().Left + 0.5f);
	float upperX = std::min(center.x + extents.x, ZeoEngine::GetActiveGameCamera()->GetCameraBounds().Right - 0.5f);
	float lowerY = std::max(center.y - extents.y, ZeoEngine::GetActiveGameCamera()->GetCameraBounds().Bottom + 0.5f);
	float upperY = std::min(center.y + extents.y, ZeoEngine::GetActiveGameCamera()->GetCameraBounds().Top - 0.5f);
	float randomX = ZeoEngine::RandomEngine::RandFloatInRange(lowerX, upperX);
	float randomY = ZeoEngine::RandomEngine::RandFloatInRange(lowerY, upperY);
	return { randomX, randomY };
}

void Enemy::SpawnBullet()
{
	// "Spawn" a bullet from pool
	EnemyBullet* bullet = m_BulletPool->GetNextPooledObject();
	if (bullet)
	{
		bullet->SetPosition2D(GetPosition2D() - glm::vec2({ 0.0f, 0.5f }));
	}
}

void Enemy::Explode()
{
	ZeoEngine::Level::Get().SpawnParticleSystemAtPosition(m_ExplosionParticle, GetPosition2D());
}
