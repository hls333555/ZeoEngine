#include "ParticleSystem.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/compatibility.hpp>

#include "RandomEngine.h"
#include "GameObject.h"

ParticleSystem::ParticleSystem(const ParticleTemplate& particleTemplate, GameObject* attachToParent, bool bAutoDestroy)
	: m_ParticleTemplate(particleTemplate)
	, m_SpawnRate(1.0f / 30.0f)
	, m_Parent(attachToParent)
	, m_bAutoDestroy(bAutoDestroy)
{
	m_ParticlePool.resize(1000);

	m_LoopCount = particleTemplate.loopCount;
	m_bInfiniteLoop = m_LoopCount <= 0;

	// Spawn rate
	{
		float evaluated = 0.0f;
		switch (particleTemplate.spawnRate.variationType)
		{
		case ParticleVariationType::Constant:
			evaluated = particleTemplate.spawnRate.val1;
			break;
		case ParticleVariationType::RandomInRange:
			evaluated = RandomEngine::RandFloatInRange(particleTemplate.spawnRate.val1, particleTemplate.spawnRate.val2);
			break;
		default:
			break;
		}
		if (evaluated == 0.0f)
		{
			m_SpawnRate = 0.0f;
		}
		else if (evaluated > 0.0f)
		{
			m_SpawnRate = 1.0f / evaluated;
		}
	}

	// Burst list
	{
		for (const auto& burstData : particleTemplate.burstList)
		{
			switch (burstData.second.variationType)
			{
			case ParticleVariationType::Constant:
				m_BurstList.emplace(burstData.first, burstData.second.val1);
				break;
			case ParticleVariationType::RandomInRange:
				m_BurstList.emplace(burstData.first, (int32_t)RandomEngine::RandFloatInRange((float)burstData.second.val1, (float)burstData.second.val2));
				break;
			default:
				break;
			}
		}
	}

	// Texture
	{
		float x = particleTemplate.subImageSize.x == 0 ? 1.0f : 1.0f / particleTemplate.subImageSize.x;
		float y = particleTemplate.subImageSize.y == 0 ? 1.0f : 1.0f / particleTemplate.subImageSize.y;
		m_TilingFactor = { x, y };
	}

}

void ParticleSystem::Emit()
{
	Particle& particle = m_ParticlePool[m_PoolIndex];
	particle.bActive = true;

	// Position
	{
		switch (m_ParticleTemplate.initialPosition.variationType)
		{
		case ParticleVariationType::Constant:
			particle.position = m_ParticleTemplate.initialPosition.val1;
			break;
		case ParticleVariationType::RandomInRange:
			particle.position.x = RandomEngine::RandFloatInRange(m_ParticleTemplate.initialPosition.val1.x, m_ParticleTemplate.initialPosition.val2.x);
			particle.position.y = RandomEngine::RandFloatInRange(m_ParticleTemplate.initialPosition.val1.y, m_ParticleTemplate.initialPosition.val2.y);
			break;
		default:
			break;
		}
		if (m_Parent)
		{
			particle.position += m_Parent->GetPosition2D();
		}
	}

	// Rotation
	{
		switch (m_ParticleTemplate.initialRotation.variationType)
		{
		case ParticleVariationType::Constant:
			particle.rotation = m_ParticleTemplate.initialRotation.val1;
			break;
		case ParticleVariationType::RandomInRange:
			particle.rotation = RandomEngine::RandFloatInRange(m_ParticleTemplate.initialRotation.val1, m_ParticleTemplate.initialRotation.val2);
			break;
		default:
			break;
		}
		switch (m_ParticleTemplate.rotationRate.variationType)
		{
		case ParticleVariationType::Constant:
			particle.rotationRate = m_ParticleTemplate.rotationRate.val1;
			break;
		case ParticleVariationType::RandomInRange:
			particle.rotationRate = RandomEngine::RandFloatInRange(m_ParticleTemplate.rotationRate.val1, m_ParticleTemplate.rotationRate.val2);
			break;
		default:
			break;
		}
	}

	// Size
	{
		switch (m_ParticleTemplate.sizeBegin.variationType)
		{
		case ParticleVariationType::Constant:
			particle.sizeBegin = m_ParticleTemplate.sizeBegin.val1;
			break;
		case ParticleVariationType::RandomInRange:
			particle.sizeBegin.x = RandomEngine::RandFloatInRange(m_ParticleTemplate.sizeBegin.val1.x, m_ParticleTemplate.sizeBegin.val2.x);
			particle.sizeBegin.y = RandomEngine::RandFloatInRange(m_ParticleTemplate.sizeBegin.val1.y, m_ParticleTemplate.sizeBegin.val2.y);
			break;
		case ParticleVariationType::UniformInRange:
			particle.sizeBegin.x = RandomEngine::RandFloatInRange(m_ParticleTemplate.sizeBegin.val1.x, m_ParticleTemplate.sizeBegin.val2.x);
			particle.sizeBegin.y = particle.sizeBegin.x;
			break;
		default:
			break;
		}
		switch (m_ParticleTemplate.sizeEnd.variationType)
		{
		case ParticleVariationType::Constant:
			particle.sizeEnd = m_ParticleTemplate.sizeEnd.val1;
			break;
		case ParticleVariationType::RandomInRange:
			particle.sizeEnd.x = RandomEngine::RandFloatInRange(m_ParticleTemplate.sizeEnd.val1.x, m_ParticleTemplate.sizeEnd.val2.x);
			particle.sizeEnd.y = RandomEngine::RandFloatInRange(m_ParticleTemplate.sizeEnd.val1.y, m_ParticleTemplate.sizeEnd.val2.y);
			break;
		case ParticleVariationType::UniformInRange:
			particle.sizeEnd.x = RandomEngine::RandFloatInRange(m_ParticleTemplate.sizeEnd.val1.x, m_ParticleTemplate.sizeEnd.val2.x);
			particle.sizeEnd.y = particle.sizeEnd.x;
			break;
		default:
			break;
		}
	}

	// Velocity
	{
		switch (m_ParticleTemplate.initialVelocity.variationType)
		{
		case ParticleVariationType::Constant:
			particle.velocity = m_ParticleTemplate.initialVelocity.val1;
			break;
		case ParticleVariationType::RandomInRange:
			particle.velocity.x = RandomEngine::RandFloatInRange(m_ParticleTemplate.initialVelocity.val1.x, m_ParticleTemplate.initialVelocity.val2.x);
			particle.velocity.y = RandomEngine::RandFloatInRange(m_ParticleTemplate.initialVelocity.val1.y, m_ParticleTemplate.initialVelocity.val2.y);
			break;
		default:
			break;
		}
		m_ParticleTemplate.inheritVelocity = glm::clamp(m_ParticleTemplate.inheritVelocity, { 0.0f, 0.0f }, { 1.0f, 1.0f });
	}

	// Color
	{
		switch (m_ParticleTemplate.colorBegin.variationType)
		{
		case ParticleVariationType::Constant:
			particle.colorBegin = m_ParticleTemplate.colorBegin.val1;
			break;
		case ParticleVariationType::RandomInRange:
			particle.colorBegin.r = RandomEngine::RandFloatInRange(m_ParticleTemplate.colorBegin.val1.r, m_ParticleTemplate.colorBegin.val2.r);
			particle.colorBegin.g = RandomEngine::RandFloatInRange(m_ParticleTemplate.colorBegin.val1.g, m_ParticleTemplate.colorBegin.val2.g);
			particle.colorBegin.b = RandomEngine::RandFloatInRange(m_ParticleTemplate.colorBegin.val1.b, m_ParticleTemplate.colorBegin.val2.b);
			particle.colorBegin.a = RandomEngine::RandFloatInRange(m_ParticleTemplate.colorBegin.val1.a, m_ParticleTemplate.colorBegin.val2.a);
			break;
		default:
			break;
		}
		switch (m_ParticleTemplate.colorEnd.variationType)
		{
		case ParticleVariationType::Constant:
			particle.colorEnd = m_ParticleTemplate.colorEnd.val1;
			break;
		case ParticleVariationType::RandomInRange:
			particle.colorEnd.r = RandomEngine::RandFloatInRange(m_ParticleTemplate.colorEnd.val1.r, m_ParticleTemplate.colorEnd.val2.r);
			particle.colorEnd.g = RandomEngine::RandFloatInRange(m_ParticleTemplate.colorEnd.val1.g, m_ParticleTemplate.colorEnd.val2.g);
			particle.colorEnd.b = RandomEngine::RandFloatInRange(m_ParticleTemplate.colorEnd.val1.b, m_ParticleTemplate.colorEnd.val2.b);
			particle.colorEnd.a = RandomEngine::RandFloatInRange(m_ParticleTemplate.colorEnd.val1.a, m_ParticleTemplate.colorEnd.val2.a);
			break;
		default:
			break;
		}
	}

	// Life
	{
		switch (m_ParticleTemplate.lifeTime.variationType)
		{
		case ParticleVariationType::Constant:
			particle.lifeTime = m_ParticleTemplate.lifeTime.val1;
			break;
		case ParticleVariationType::RandomInRange:
			particle.lifeTime = RandomEngine::RandFloatInRange(m_ParticleTemplate.lifeTime.val1, m_ParticleTemplate.lifeTime.val2);
			break;
		default:
			break;
		}
		particle.lifeRemaining = particle.lifeTime;
	}

	// Texture
	{
		auto count = m_ParticleTemplate.subImageSize.x * m_ParticleTemplate.subImageSize.y;
		m_UVAnimationInterval = count == 0 ? 0.0f : particle.lifeTime / count;
		// Initialize uv animation start point to left-up corner
		particle.uvOffset.x = 0;
		particle.uvOffset.y = (float)m_ParticleTemplate.subImageSize.y - 1.0f;
	}

	m_PoolIndex = --m_PoolIndex % m_ParticlePool.size();
}

void ParticleSystem::OnUpdate(ZeoEngine::DeltaTime dt)
{
	if (m_bPendingDestroy)
		return;
	if (!m_bStartUpdate)
		return;

	m_Time += dt;
	if (m_bActive)
	{
		if (m_Time - m_LoopStartTime >= m_ParticleTemplate.loopDuration)
		{
			if (m_LoopCount != 0)
			{
				--m_LoopCount;
			}
			m_LoopStartTime = m_Time;
			m_BurstTime = m_Time;
			for (auto& burstData : m_BurstList)
			{
				const_cast<BurstTimeData&>(burstData.first).bProcessed = false;
			}
		}
		
		if ((m_bInfiniteLoop || m_LoopCount != 0))
		{
			// Process SpawnRate
			if (m_SpawnRate != 0.0f && (m_SpawnTime == 0.0f || m_Time - m_SpawnTime > m_SpawnRate))
			{
				Emit();
				m_SpawnTime = m_Time;
			}
			// Process BurstList
			for (auto& burstData : m_BurstList)
			{
				if (m_Time - m_BurstTime >= burstData.first.time * m_ParticleTemplate.loopDuration)
				{
					if (!burstData.first.bProcessed && burstData.second > 0)
					{
						const_cast<BurstTimeData&>(burstData.first).bProcessed = true;
						for (uint32_t i = 0; i < burstData.second; ++i)
						{
							Emit();
						}
					}
				}
			}
		}
	}

	bool bSystemComplete = true;
	for (auto& particle : m_ParticlePool)
	{
		if (!particle.bActive)
			continue;

		if (particle.lifeRemaining <= 0.0f)
		{
			particle.bActive = false;
			continue;
		}

		bSystemComplete = false;

		particle.lifeRemaining -= dt;
		glm::vec2 velocity{ 0.0f, 0.0f };
		if (m_Parent)
		{
			velocity = m_ParticleTemplate.bIsLocalSpace ?
				m_Parent->GetVelocity() + particle.velocity :
				m_Parent->GetVelocity() * m_ParticleTemplate.inheritVelocity + particle.velocity;
		}
		else
		{
			velocity = particle.velocity;
		}
		particle.position += velocity * (float)dt;
		particle.rotation += particle.rotationRate * dt;
		float lifeRatio = particle.lifeRemaining / particle.lifeTime;
		particle.size = glm::lerp(particle.sizeEnd, particle.sizeBegin, lifeRatio);
		particle.color = glm::lerp(particle.colorEnd, particle.colorBegin, lifeRatio);
		// Process UV animation
		if (m_UVAnimationInterval != 0.0f && m_Time - m_UVAnimationTime >= m_UVAnimationInterval)
		{
			// When animation reaches the last sub-image, deactivate this particle immediately
			if (particle.uvOffset.x == m_ParticleTemplate.subImageSize.x - 1.0f && particle.uvOffset.y == 0.0f)
			{
				particle.bActive = false;
				continue;
			}
			// Next column
			particle.uvOffset.x += 1.0f;
			if (particle.uvOffset.x == m_ParticleTemplate.subImageSize.x)
			{
				particle.uvOffset.x = 0.0f;
				// Next row
				particle.uvOffset.y -= 1.0f;
			}
			m_UVAnimationTime = m_Time;
		}
	}

	if (m_bAutoDestroy && bSystemComplete && ((!m_bInfiniteLoop && m_LoopCount == 0) || !m_bActive))
	{
		m_bPendingDestroy = true;
	}
}

void ParticleSystem::OnRender()
{
	if (m_bPendingDestroy)
		return;

	m_bStartUpdate = true;

	for (auto& particle : m_ParticlePool)
	{
		if (!particle.bActive)
			continue;

		if (m_ParticleTemplate.texture)
		{
			ZeoEngine::Renderer2D::DrawRotatedQuad(particle.position, particle.size, glm::radians(particle.rotation), m_ParticleTemplate.texture, m_TilingFactor, particle.uvOffset, particle.color);
		}
		else
		{
			ZeoEngine::Renderer2D::DrawRotatedQuad(particle.position, particle.size, glm::radians(particle.rotation), particle.color);
		}
	}
}

void ParticleSystem::Activate()
{
	if (m_bPendingDestroy || m_bActive)
		return;

	m_bActive = true;
	m_LoopCount = m_ParticleTemplate.loopCount;
	m_Time = m_LoopStartTime = m_SpawnTime = 0.0f;
}

void ParticleSystem::Deactivate()
{
	m_bActive = false;
}

ParticleManager::~ParticleManager()
{
	for (auto* ps : m_ParticleSystems)
	{
		delete ps;
	}
}

void ParticleManager::OnUpdate(ZeoEngine::DeltaTime dt)
{
	for (uint32_t i = 0; i < m_ParticleSystems.size(); ++i)
	{
		if (m_ParticleSystems[i]->m_bPendingDestroy)
		{
			delete m_ParticleSystems[i];
			m_ParticleSystems.erase(m_ParticleSystems.begin() + i);
		}
		else
		{
			// Update particle system
			m_ParticleSystems[i]->OnUpdate(dt);
		}
	}
}

void ParticleManager::OnRender()
{
	for (uint32_t i = 0; i < m_ParticleSystems.size(); ++i)
	{
		// Render particle system
		m_ParticleSystems[i]->OnRender();
	}
}

void ParticleManager::AddParticleSystem(ParticleSystem* particleSystem)
{
	m_ParticleSystems.push_back(particleSystem);
}
