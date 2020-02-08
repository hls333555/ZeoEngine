#include "Enemy.h"

#include "GameLevel.h"
#include "Player.h"

RTTR_REGISTRATION
{
	using namespace rttr;
	using namespace ZeoEngine;
	registration::class_<Enemy>("Enemy")
		.constructor(&Enemy::SpawnGameObject, policy::ctor::as_raw_ptr);
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
	m_SpriteTexture = ZeoEngine::Texture2D::Create("assets/textures/Ship2.png");
}

void Enemy::Init()
{
	Super::Init();

	FillSphereCollisionData(GetScale().x / 2.0f * 0.75f);

	m_ExplosionTexture = ZeoEngine::GetTexture2DLibrary()->Get("assets/textures/Explosion_2x4.png");

	m_BulletPool = ZeoEngine::CreateScope<BulletPool>(this);

}

void Enemy::BeginPlay()
{
	Super::BeginPlay();

	// Move to a random position of upper half screen nearby every 2 seconds
	ZeoEngine::GetTimerManager()->SetTimer(2.0f, [&]() {
		glm::vec2 targetPosition = GetRandomPositionInRange2D(GetPosition2D(), { 2.0f, 2.0f });
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
		ZeoEngine::GetTimerManager()->SetTimer(m_ShootRate, [&]() {
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

void Enemy::OnDestroyed()
{
	Super::OnDestroyed();

	// Spawn next enemy
	//m_Level->DelaySpawnEnemy(2.0f);
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
