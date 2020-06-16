#include "ZEpch.h"
#include "Engine/GameFramework/ParticleSystem.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/compatibility.hpp>
#include <imgui.h>

#include "Engine/Renderer/Renderer2D.h"
#include "Engine/Core/RandomEngine.h"
#include "Engine/GameFramework/GameObject.h"
#include "Engine/Core/Serializer.h"

RTTR_REGISTRATION
{
	using namespace rttr;
	using namespace ZeoEngine;

	registration::class_<ParticleVariation>("ParticleVariation")
		.enumeration<ParticleVariationType>("ParticleVariationType")
		(
			value("Constant", ParticleVariationType::Constant),
			value("Random In Range", ParticleVariationType::RandomInRange),
			value("Uniform In Range", ParticleVariationType::UniformInRange)
		)
		.property("VariationType", &ParticleVariation::VariationType);

	registration::class_<ParticleInt>("ParticleInt")
		.property("Value", &ParticleInt::Val1)
		.property("ValueHigh", &ParticleInt::Val2)
		(
			metadata(PropertyMeta::HideCondition, "VariationType==Constant")
		);

	registration::class_<ParticleFloat>("ParticleFloat")
		.property("Value", &ParticleFloat::Val1)
		.property("ValueHigh", &ParticleFloat::Val2)
		(
			metadata(PropertyMeta::HideCondition, "VariationType==Constant")
		);

	registration::class_<ParticleVec2>("ParticleVec2")
		.property("Value", &ParticleVec2::Val1)
		(
			policy::prop::bind_as_ptr
		)
		.property("ValueHigh", &ParticleVec2::Val2)
		(
			policy::prop::bind_as_ptr,
			metadata(PropertyMeta::HideCondition, "VariationType==Constant")
		);

	registration::class_<ParticleVec3>("ParticleVec3")
		.property("Value", &ParticleVec3::Val1)
		(
			policy::prop::bind_as_ptr
		)
		.property("ValueHigh", &ParticleVec3::Val2)
		(
			policy::prop::bind_as_ptr,
			metadata(PropertyMeta::HideCondition, "VariationType==Constant")
		);

	registration::class_<ParticleColor>("ParticleColor")
		.property("Value", &ParticleColor::Val1)
		(
			policy::prop::bind_as_ptr
		)
		.property("ValueHigh", &ParticleColor::Val2)
		(
			policy::prop::bind_as_ptr,
			metadata(PropertyMeta::HideCondition, "VariationType==Constant")
		);

	registration::class_<BurstData>("BurstData")
		.property("Time", &BurstData::Time)
		(
			metadata(PropertyMeta::Tooltip, u8"标准化的时间点：[0,1]"),
			metadata(PropertyMeta::Min, 0.0f),
			metadata(PropertyMeta::Max, 1.0f),
			metadata(PropertyMeta::DragSensitivity, 0.01f)
		)
		.property("Amount", &BurstData::Amount)
		(
			policy::prop::bind_as_ptr,
			metadata(PropertyMeta::Tooltip, u8"在该时间点一次性生成的粒子数")
		);

	registration::class_<ParticleTemplate>("ParticleTemplate")
		.property("IsLocalSpace", &ParticleTemplate::bIsLocalSpace)
		(
			metadata(PropertyMeta::Category, "Emitter"),
			metadata(PropertyMeta::Tooltip, u8"是否在局部空间模拟粒子")
		)
		.property("LoopCount", &ParticleTemplate::LoopCount)
		(
			metadata(PropertyMeta::Category, "Emitter"),
			metadata(PropertyMeta::Tooltip, u8"循环次数。若小于等于0, 则为无限循环")
		)
		.property("LoopDuration", &ParticleTemplate::LoopDuration)
		(
			metadata(PropertyMeta::Category, "Emitter"),
			metadata(PropertyMeta::Tooltip, u8"每次循环的时长")
		)
		.property("SpawnRate", &ParticleTemplate::SpawnRate)
		(
			policy::prop::bind_as_ptr,
			metadata(PropertyMeta::Category, "Emitter"),
			metadata(PropertyMeta::Tooltip, u8"每秒总共生成的粒子数，该变量决定粒子的生成速度。若小于等于0，则不生成")
		)
		.property("BurstList", &ParticleTemplate::BurstList)
		(
			policy::prop::bind_as_ptr,
			metadata(PropertyMeta::Category, "Emitter"),
			metadata(PropertyMeta::Tooltip, u8"每个时间点一次性生成的粒子数")
		)
		.property("InitialPosition", &ParticleTemplate::InitialPosition)
		(
			policy::prop::bind_as_ptr,
			metadata(PropertyMeta::Category, "Particle: Location"),
			metadata(PropertyMeta::Tooltip, u8"粒子的初始位置")
		)
		.property("InitialRotation", &ParticleTemplate::InitialRotation)
		(
			policy::prop::bind_as_ptr,
			metadata(PropertyMeta::Category, "Particle: Rotation"),
			metadata(PropertyMeta::Tooltip, u8"粒子的初始旋转")
		)
		.property("RotationRate", &ParticleTemplate::RotationRate)
		(
			policy::prop::bind_as_ptr,
			metadata(PropertyMeta::Category, "Particle: Rotation"),
			metadata(PropertyMeta::Tooltip, u8"决定粒子的旋转速度")
		)
		.property("SizeBegin", &ParticleTemplate::SizeBegin)
		(
			policy::prop::bind_as_ptr,
			metadata(PropertyMeta::Category, "Particle: Size"),
			metadata(PropertyMeta::Tooltip, u8"决定粒子的初始大小")
		)
		.property("SizeEnd", &ParticleTemplate::SizeEnd)
		(
			policy::prop::bind_as_ptr,
			metadata(PropertyMeta::Category, "Particle: Size"),
			metadata(PropertyMeta::Tooltip, u8"决定粒子的最终大小")
		)
		.property("InitialVelocity", &ParticleTemplate::InitialVelocity)
		(
			policy::prop::bind_as_ptr,
			metadata(PropertyMeta::Category, "Particle: Velocity"),
			metadata(PropertyMeta::Tooltip, u8"决定粒子的初始速度")
		)
		.property("InheritVelocity", &ParticleTemplate::InheritVelocity)
		(
			policy::prop::bind_as_ptr,
			metadata(PropertyMeta::Category, "Particle: Velocity"),
			metadata(PropertyMeta::Tooltip, u8"决定粒子所能继承的速度。该变量只有在非局部空间有效")
		)
		.property("ColorBegin", &ParticleTemplate::ColorBegin)
		(
			policy::prop::bind_as_ptr,
			metadata(PropertyMeta::Category, "Particle: Color"),
			metadata(PropertyMeta::Tooltip, u8"决定粒子的初始颜色")
		)
		.property("ColorEnd", &ParticleTemplate::ColorEnd)
		(
			policy::prop::bind_as_ptr,
			metadata(PropertyMeta::Category, "Particle: Color"),
			metadata(PropertyMeta::Tooltip, u8"决定粒子的最终颜色")
		)
		.property("Lifetime", &ParticleTemplate::Lifetime)
		(
			policy::prop::bind_as_ptr,
			metadata(PropertyMeta::Category, "Particle: Base"),
			metadata(PropertyMeta::Tooltip, u8"决定粒子的生命周期")
		)
		.property("Texture", &ParticleTemplate::Texture)
		(
			metadata(PropertyMeta::Category, "Renderer: Texture"),
			metadata(PropertyMeta::Tooltip, u8"粒子的材质贴图")
		)
		.property("SubImageSize", &ParticleTemplate::SubImageSize)
		(
			policy::prop::bind_as_ptr,
			metadata(PropertyMeta::Category, "Renderer: Texture"),
			metadata(PropertyMeta::Tooltip, u8"决定如何分割贴图来用于UV动画。x为列数，y为行数"),
			metadata(PropertyMeta::Min, 0)
		)
		.property("MaxDrawParticles", &ParticleTemplate::MaxDrawParticles)
		(
			metadata(PropertyMeta::Category, "Emitter"),
			metadata(PropertyMeta::Tooltip, u8"最多生成的粒子数")
		);

	registration::class_<ParticleSystem>("ParticleSystem")
#if WITH_EDITOR
		.method("PostPropertyValueEditChange", &ParticleSystem::PostPropertyValueEditChange)
#endif
		.property("ParticleEmitter", &ParticleSystem::m_ParticleTemplate)
		(
			policy::prop::bind_as_ptr
		);
}

namespace ZeoEngine {

	ParticleSystem::ParticleSystem(const std::string& filePath, const std::string& processedSrc)
		: m_PoolIndex(0)
		, m_Path(filePath)
		, m_SpawnRate(30.0f)
	{
		// Extract name from file path
		// "assets/particles/Particle.zparticle" -> "Particle.zparticle"
		auto lastSlash = m_Path.find_last_of("/\\"); // find_last_of() will find ANY of the provided characters
		lastSlash = lastSlash == std::string::npos ? 0 : lastSlash + 1;
		auto count = m_Path.size() - lastSlash;
		m_FileName = m_Path.substr(lastSlash, count);

		Serializer::Get().Deserialize<ParticleSystem*>(processedSrc, [&]() {
			return rttr::variant(this);
		});
		m_PoolIndex = m_ParticleTemplate.MaxDrawParticles - 1;
		ResizeParticlePool();
		EvaluateEmitterProperties();
	}

	ParticleSystem::ParticleSystem(const ParticleTemplate& particleTemplate, const glm::vec2& position, bool bAutoDestroy)
		: m_PoolIndex(particleTemplate.MaxDrawParticles - 1)
		, m_ParticleTemplate(particleTemplate)
		, m_SpawnPosition(position)
		, m_bAutoDestroy(bAutoDestroy)
		, m_SpawnRate(30.0f)
	{
		ResizeParticlePool();
		EvaluateEmitterProperties();
	}

	ParticleSystem::ParticleSystem(const ParticleTemplate& particleTemplate, GameObject* attachToParent, bool bAutoDestroy, bool bIsInParticleEditor)
		: m_PoolIndex(particleTemplate.MaxDrawParticles - 1)
		, m_ParticleTemplate(particleTemplate)
		, m_Parent(attachToParent)
		, m_bAutoDestroy(bAutoDestroy)
		, m_SpawnRate(30.0f)
#if WITH_EDITOR
		, m_bIsInParticleEditor(bIsInParticleEditor)
		, m_FiniteLoopRestartInterval(1.0f)
#endif
	{
		ResizeParticlePool();
		EvaluateEmitterProperties();
	}

#if WITH_EDITOR
	void ParticleSystem::PostPropertyValueEditChange(const rttr::property* prop, const rttr::property* outerProp)
	{
		ZE_CORE_ASSERT(prop);

		if (prop->get_name() == "LoopCount" ||
			outerProp->get_name() == "SpawnRate" ||
			outerProp->get_name() == "BurstList" /* e.g. BurstData.Time */ || outerProp->get_declaring_type().get_name() == "BurstData" /* e.g. BurstData.Amount.Value */ ||
			prop->get_name() == "SubImageSize")
		{
			EvaluateEmitterProperties();
			Resimulate();
		}
		else if (prop->get_name() == "MaxDrawParticles")
		{
			ResizeParticlePool();
		}
	}
#endif

	ParticleSystem* ParticleSystem::CreateDefaultParticleSystem()
	{
		ParticleTemplate m_DefaultEmitter;
		m_DefaultEmitter.Lifetime.SetRandom(0.75f, 1.5f);
		m_DefaultEmitter.SpawnRate.SetConstant(30.0f);
		m_DefaultEmitter.InitialRotation.SetRandom(0.0f, 360.0f);
		m_DefaultEmitter.RotationRate.SetRandom(10.0f, 50.0f);
		m_DefaultEmitter.InitialVelocity.SetRandom({ -0.5f, 0.5f }, { 0.5f, 2.0f });
		m_DefaultEmitter.SizeBegin.SetRandom(0.1f, 0.2f);
		m_DefaultEmitter.SizeEnd.SetConstant({ 0.0f, 0.0f });
		m_DefaultEmitter.ColorBegin.SetConstant({ 1.0f, 1.0f, 1.0f, 1.0f });
		m_DefaultEmitter.ColorEnd.SetConstant({ 0.0f, 0.0f, 0.0f, 0.0f });
		return new ParticleSystem(m_DefaultEmitter, nullptr, false, true);
	}

	void ParticleSystem::ResizeParticlePool()
	{
		m_ParticlePool.resize(m_ParticleTemplate.MaxDrawParticles);
		m_PoolIndex = m_ParticleTemplate.MaxDrawParticles - 1;
	}

	void ParticleSystem::EvaluateEmitterProperties()
	{
		m_LoopCount = m_ParticleTemplate.LoopCount;
		m_bInfiniteLoop = m_LoopCount <= 0;

		// Spawn rate
		{
			switch (m_ParticleTemplate.SpawnRate.VariationType)
			{
			case ParticleVariationType::Constant:
				m_SpawnRate = m_ParticleTemplate.SpawnRate.Val1;
				break;
			case ParticleVariationType::RandomInRange:
				m_SpawnRate = RandomEngine::RandFloatInRange(m_ParticleTemplate.SpawnRate.Val1, m_ParticleTemplate.SpawnRate.Val2);
				break;
			default:
				break;
			}
			m_SpawnRate = m_SpawnRate < 0.0f ? 0.0f : m_SpawnRate;
		}

		// Burst list
		{
			m_BurstList.clear();
			for (const auto& burstData : m_ParticleTemplate.BurstList)
			{
				uint32_t value = 0;
				switch (burstData.Amount.VariationType)
				{
				case ParticleVariationType::Constant:
				{
					value = burstData.Amount.Val1 > 0 ? burstData.Amount.Val1 : 0;
					m_BurstList.emplace(burstData.Time, value);
					break;
				}
				case ParticleVariationType::RandomInRange:
				{
					float tempValue = RandomEngine::RandFloatInRange(static_cast<float>(burstData.Amount.Val1), static_cast<float>(burstData.Amount.Val2));
					value = tempValue > 0.0f ? static_cast<uint32_t>(tempValue) : 0;
					m_BurstList.emplace(burstData.Time, value);
					break;
				}
				default:
					break;
				}
			}
		}

		// Texture
		{
			float x = m_ParticleTemplate.SubImageSize.x == 0 ? 1.0f : 1.0f / m_ParticleTemplate.SubImageSize.x;
			float y = m_ParticleTemplate.SubImageSize.y == 0 ? 1.0f : 1.0f / m_ParticleTemplate.SubImageSize.y;
			m_TilingFactor = { x, y };
		}
	}

	void ParticleSystem::EvaluateParticleProperties(Particle& particle)
	{
		// Position
		{
			switch (m_ParticleTemplate.InitialPosition.VariationType)
			{
			case ParticleVariationType::Constant:
				particle.Position = m_ParticleTemplate.InitialPosition.Val1;
				break;
			case ParticleVariationType::RandomInRange:
				particle.Position.x = RandomEngine::RandFloatInRange(m_ParticleTemplate.InitialPosition.Val1.x, m_ParticleTemplate.InitialPosition.Val2.x);
				particle.Position.y = RandomEngine::RandFloatInRange(m_ParticleTemplate.InitialPosition.Val1.y, m_ParticleTemplate.InitialPosition.Val2.y);
				break;
			default:
				break;
			}
			if (m_Parent)
			{
				particle.Position += m_Parent->GetPosition2D();
			}
			if (m_SpawnPosition != glm::vec2(0.0f))
			{
				particle.Position += m_SpawnPosition;
			}
		}

		// Rotation
		{
			switch (m_ParticleTemplate.InitialRotation.VariationType)
			{
			case ParticleVariationType::Constant:
				particle.Rotation = m_ParticleTemplate.InitialRotation.Val1;
				break;
			case ParticleVariationType::RandomInRange:
				particle.Rotation = RandomEngine::RandFloatInRange(m_ParticleTemplate.InitialRotation.Val1, m_ParticleTemplate.InitialRotation.Val2);
				break;
			default:
				break;
			}
			switch (m_ParticleTemplate.RotationRate.VariationType)
			{
			case ParticleVariationType::Constant:
				particle.RotationRate = m_ParticleTemplate.RotationRate.Val1;
				break;
			case ParticleVariationType::RandomInRange:
				particle.RotationRate = RandomEngine::RandFloatInRange(m_ParticleTemplate.RotationRate.Val1, m_ParticleTemplate.RotationRate.Val2);
				break;
			default:
				break;
			}
		}

		// Size
		{
			switch (m_ParticleTemplate.SizeBegin.VariationType)
			{
			case ParticleVariationType::Constant:
				particle.SizeBegin = m_ParticleTemplate.SizeBegin.Val1;
				break;
			case ParticleVariationType::RandomInRange:
				particle.SizeBegin.x = RandomEngine::RandFloatInRange(m_ParticleTemplate.SizeBegin.Val1.x, m_ParticleTemplate.SizeBegin.Val2.x);
				particle.SizeBegin.y = RandomEngine::RandFloatInRange(m_ParticleTemplate.SizeBegin.Val1.y, m_ParticleTemplate.SizeBegin.Val2.y);
				break;
			case ParticleVariationType::UniformInRange:
				particle.SizeBegin.x = RandomEngine::RandFloatInRange(m_ParticleTemplate.SizeBegin.Val1.x, m_ParticleTemplate.SizeBegin.Val2.x);
				particle.SizeBegin.y = particle.SizeBegin.x;
				break;
			default:
				break;
			}
			switch (m_ParticleTemplate.SizeEnd.VariationType)
			{
			case ParticleVariationType::Constant:
				particle.SizeEnd = m_ParticleTemplate.SizeEnd.Val1;
				break;
			case ParticleVariationType::RandomInRange:
				particle.SizeEnd.x = RandomEngine::RandFloatInRange(m_ParticleTemplate.SizeEnd.Val1.x, m_ParticleTemplate.SizeEnd.Val2.x);
				particle.SizeEnd.y = RandomEngine::RandFloatInRange(m_ParticleTemplate.SizeEnd.Val1.y, m_ParticleTemplate.SizeEnd.Val2.y);
				break;
			case ParticleVariationType::UniformInRange:
				particle.SizeEnd.x = RandomEngine::RandFloatInRange(m_ParticleTemplate.SizeEnd.Val1.x, m_ParticleTemplate.SizeEnd.Val2.x);
				particle.SizeEnd.y = particle.SizeEnd.x;
				break;
			default:
				break;
			}
		}

		// Velocity
		{
			switch (m_ParticleTemplate.InitialVelocity.VariationType)
			{
			case ParticleVariationType::Constant:
				particle.Velocity = m_ParticleTemplate.InitialVelocity.Val1;
				break;
			case ParticleVariationType::RandomInRange:
				particle.Velocity.x = RandomEngine::RandFloatInRange(m_ParticleTemplate.InitialVelocity.Val1.x, m_ParticleTemplate.InitialVelocity.Val2.x);
				particle.Velocity.y = RandomEngine::RandFloatInRange(m_ParticleTemplate.InitialVelocity.Val1.y, m_ParticleTemplate.InitialVelocity.Val2.y);
				break;
			default:
				break;
			}
			m_ParticleTemplate.InheritVelocity = glm::clamp(m_ParticleTemplate.InheritVelocity, { 0.0f, 0.0f }, { 1.0f, 1.0f });
		}

		// Color
		{
			switch (m_ParticleTemplate.ColorBegin.VariationType)
			{
			case ParticleVariationType::Constant:
				particle.ColorBegin = m_ParticleTemplate.ColorBegin.Val1;
				break;
			case ParticleVariationType::RandomInRange:
				particle.ColorBegin.r = RandomEngine::RandFloatInRange(m_ParticleTemplate.ColorBegin.Val1.r, m_ParticleTemplate.ColorBegin.Val2.r);
				particle.ColorBegin.g = RandomEngine::RandFloatInRange(m_ParticleTemplate.ColorBegin.Val1.g, m_ParticleTemplate.ColorBegin.Val2.g);
				particle.ColorBegin.b = RandomEngine::RandFloatInRange(m_ParticleTemplate.ColorBegin.Val1.b, m_ParticleTemplate.ColorBegin.Val2.b);
				particle.ColorBegin.a = RandomEngine::RandFloatInRange(m_ParticleTemplate.ColorBegin.Val1.a, m_ParticleTemplate.ColorBegin.Val2.a);
				break;
			default:
				break;
			}
			switch (m_ParticleTemplate.ColorEnd.VariationType)
			{
			case ParticleVariationType::Constant:
				particle.ColorEnd = m_ParticleTemplate.ColorEnd.Val1;
				break;
			case ParticleVariationType::RandomInRange:
				particle.ColorEnd.r = RandomEngine::RandFloatInRange(m_ParticleTemplate.ColorEnd.Val1.r, m_ParticleTemplate.ColorEnd.Val2.r);
				particle.ColorEnd.g = RandomEngine::RandFloatInRange(m_ParticleTemplate.ColorEnd.Val1.g, m_ParticleTemplate.ColorEnd.Val2.g);
				particle.ColorEnd.b = RandomEngine::RandFloatInRange(m_ParticleTemplate.ColorEnd.Val1.b, m_ParticleTemplate.ColorEnd.Val2.b);
				particle.ColorEnd.a = RandomEngine::RandFloatInRange(m_ParticleTemplate.ColorEnd.Val1.a, m_ParticleTemplate.ColorEnd.Val2.a);
				break;
			default:
				break;
			}
		}

		// Life
		{
			switch (m_ParticleTemplate.Lifetime.VariationType)
			{
			case ParticleVariationType::Constant:
				particle.Lifetime = m_ParticleTemplate.Lifetime.Val1;
				break;
			case ParticleVariationType::RandomInRange:
				particle.Lifetime = RandomEngine::RandFloatInRange(m_ParticleTemplate.Lifetime.Val1, m_ParticleTemplate.Lifetime.Val2);
				break;
			default:
				break;
			}
			particle.LifeRemaining = particle.Lifetime;
		}

		// Texture
		{
			auto count = m_ParticleTemplate.SubImageSize.x * m_ParticleTemplate.SubImageSize.y;
			m_UVAnimationInterval = count == 0 ? 0.0f : particle.Lifetime / count;
			// Initialize uv animation start point to left-up corner
			particle.UvOffset.x = 0;
			particle.UvOffset.y = static_cast<float>(m_ParticleTemplate.SubImageSize.y) - 1.0f;
		}
	}

	void ParticleSystem::Emit()
	{
		uint32_t OldPoolIndex = m_PoolIndex;
		// Iterate until we find an available (inactive) particle to activate
		while (m_ParticlePool[m_PoolIndex].bActive)
		{
			CalculateNextPoolIndex();
			if (m_PoolIndex == OldPoolIndex)
			{
				// Fail to find one, skip emitting
				return;
			}
		}

		Particle& particle = m_ParticlePool[m_PoolIndex];
		particle.bActive = true;
		EvaluateParticleProperties(particle);

		CalculateNextPoolIndex();
	}

	void ParticleSystem::CalculateNextPoolIndex()
	{
		if (m_PoolIndex == 0)
		{
			m_PoolIndex = m_ParticleTemplate.MaxDrawParticles - 1;
		}
		else
		{
			m_PoolIndex = --m_PoolIndex % m_ParticlePool.size();
		}
	}

	void ParticleSystem::OnUpdate(DeltaTime dt)
	{
		if (m_bPendingDestroy)
			return;
		if (!m_bStartUpdate)
			return;

		m_Time += dt;
		if (m_bActive)
		{
			if (m_Time - m_LoopStartTime >= m_ParticleTemplate.LoopDuration)
			{
				if (m_LoopCount != 0)
				{
					--m_LoopCount;
				}
				m_LoopStartTime = m_Time;
				m_BurstTime = m_Time;
				for (const auto& burstData : m_ParticleTemplate.BurstList)
				{
					if (burstData.Amount.VariationType == ParticleVariationType::RandomInRange)
					{
						m_BurstList[burstData.Time] = static_cast<int32_t>(RandomEngine::RandFloatInRange(static_cast<float>(burstData.Amount.Val1), static_cast<float>(burstData.Amount.Val2)));
					}
				}
				for (auto& [data, amount] : m_BurstList)
				{
					const_cast<BurstTimeData&>(data).bProcessed = false;
				}
			}

			if ((m_bInfiniteLoop || m_LoopCount != 0))
			{
				// Process SpawnRate
				if (m_SpawnRate != 0.0f && (m_SpawnTime == 0.0f || m_Time - m_SpawnTime > 1.0f / m_SpawnRate))
				{
					uint32_t i = 0;
					uint32_t imax = static_cast<uint32_t>(m_SpawnRate * dt);
					do 
					{
						Emit();
						++i;
					} while (i < imax);
					m_SpawnTime = m_Time;
				}
				// Process BurstList
				for (auto& [data, amount] : m_BurstList)
				{
					if (m_Time - m_BurstTime >= data.Time * m_ParticleTemplate.LoopDuration)
					{
						if (!data.bProcessed && amount > 0)
						{
							const_cast<BurstTimeData&>(data).bProcessed = true;
							for (uint32_t i = 0; i < amount; ++i)
							{
								Emit();
							}
						}
					}
				}
			}
		}

		m_bSystemComplete = true;
		for (auto& particle : m_ParticlePool)
		{
			if (!particle.bActive)
				continue;

			if (particle.LifeRemaining <= 0.0f)
			{
				particle.bActive = false;
				continue;
			}

			m_bSystemComplete = false;

			particle.LifeRemaining -= dt;
			glm::vec2 velocity{ 0.0f, 0.0f };
			if (m_Parent)
			{
				velocity = m_ParticleTemplate.bIsLocalSpace ?
					m_Parent->GetVelocity() + particle.Velocity :
					m_Parent->GetVelocity() * m_ParticleTemplate.InheritVelocity + particle.Velocity;
			}
			else
			{
				velocity = particle.Velocity;
			}
			particle.Position += velocity * static_cast<float>(dt);
			particle.Rotation += particle.RotationRate * dt;
			float lifeRatio = particle.LifeRemaining / particle.Lifetime;
			particle.Size = glm::lerp(particle.SizeEnd, particle.SizeBegin, lifeRatio);
			particle.Color = glm::lerp(particle.ColorEnd, particle.ColorBegin, lifeRatio);
			// Process UV animation
			if (m_UVAnimationInterval != 0.0f && m_Time - m_UVAnimationTime >= m_UVAnimationInterval)
			{
				// When animation reaches the last sub-image, deactivate this particle immediately
				if (particle.UvOffset.x == m_ParticleTemplate.SubImageSize.x - 1.0f && particle.UvOffset.y == 0.0f)
				{
					particle.bActive = false;
					continue;
				}
				// Next column
				particle.UvOffset.x += 1.0f;
				if (particle.UvOffset.x == m_ParticleTemplate.SubImageSize.x)
				{
					particle.UvOffset.x = 0.0f;
					// Next row
					particle.UvOffset.y -= 1.0f;
				}
				m_UVAnimationTime = m_Time;
			}
		}

		m_bSystemComplete = m_bSystemComplete && ((!m_bInfiniteLoop && m_LoopCount == 0) || !m_bActive);

#if WITH_EDITOR
		// If this completed particle system is previewed in ParticleSystem Editor, just restart it after a while
		if (m_bSystemComplete && m_bIsInParticleEditor)
		{
			if (m_bFiniteLoopPrepareToRestart)
			{
				m_bFiniteLoopPrepareToRestart = false;
				m_FiniteLoopRestartTime = m_Time;
			}
			if (m_Time - m_FiniteLoopRestartTime >= m_FiniteLoopRestartInterval)
			{
				Resimulate();
				return;
			}
		}
#endif

		if (m_bAutoDestroy && m_bSystemComplete)
		{
			m_bPendingDestroy = true;
			m_OnSystemFinished();
		}
	}

	void ParticleSystem::OnRender()
	{
		if (m_bPendingDestroy)
			return;

		m_bStartUpdate = true;

		m_ActiveParticleCount = 0;
		for (auto& particle : m_ParticlePool)
		{
			if (!particle.bActive)
				continue;

			++m_ActiveParticleCount;
			if (m_ParticleTemplate.Texture)
			{
				Renderer2D::DrawRotatedQuad(particle.Position, particle.Size, glm::radians(particle.Rotation), m_ParticleTemplate.Texture, m_TilingFactor, particle.UvOffset, particle.Color);
			}
			else
			{
				Renderer2D::DrawRotatedQuad(particle.Position, particle.Size, glm::radians(particle.Rotation), particle.Color);
			}
		}
	}

#if WITH_EDITOR
	void ParticleSystem::OnParticleViewImGuiRender()
	{
		// The last item should be full windowed framebuffer texture
		const ImVec2 particleViewStartDrawPos = ImGui::GetItemRectMin();

		char particleCount[16];
		_itoa(m_ActiveParticleCount, particleCount, 10);
		const ImVec2 textSize = ImGui::CalcTextSize(particleCount);
		// Display particle count at the top right corner of Particle View window
		ImGui::GetForegroundDrawList()->AddText(ImVec2(ImGui::GetItemRectMax().x - textSize.x - 15.0f, particleViewStartDrawPos.y), IM_COL32(255, 255, 0, 255), particleCount);

		if (m_bSystemComplete)
		{
			static const ImVec2 textSize = ImGui::CalcTextSize("Completed");
			const float indent = (ImGui::GetWindowSize().x - textSize.x) / 2.0f;
			// Display completed text at the top center of Particle View window
			ImGui::GetForegroundDrawList()->AddText(ImVec2(particleViewStartDrawPos.x + indent, particleViewStartDrawPos.y), IM_COL32_WHITE, "Completed");
		}
	}
#endif

	void ParticleSystem::OnDeserialized()
	{
		EvaluateEmitterProperties();
		Resimulate();
	}

	void ParticleSystem::Activate()
	{
		if (m_bPendingDestroy || m_bActive)
			return;

		m_bActive = true;
		m_LoopCount = m_ParticleTemplate.LoopCount;
		m_Time = m_LoopStartTime = m_SpawnTime = m_BurstTime = m_UVAnimationTime = 0.0f;
	}

	void ParticleSystem::Deactivate()
	{
		m_bActive = false;
	}

	void ParticleSystem::Resimulate()
	{
		m_LoopCount = m_ParticleTemplate.LoopCount;
		m_Time = m_LoopStartTime = m_SpawnTime = m_BurstTime = m_UVAnimationTime = 0.0f;
#if WITH_EDITOR
		m_bFiniteLoopPrepareToRestart = true;
#endif
	}

	ParticleLibrary::~ParticleLibrary()
	{
		for (const auto& [path, ps] : m_ParticleSystems)
		{
			delete ps;
		}
		m_ParticleSystems.clear();
	}

	void ParticleLibrary::Add(const std::string& path, ParticleSystem* ps)
	{
		ZE_CORE_ASSERT(ps);
		if (!Exists(path))
		{
			m_ParticleSystems[path] = ps;
		}
	}

	void ParticleLibrary::Add(ParticleSystem* ps)
	{
		ZE_CORE_ASSERT(ps);
		const std::string& path = ps->GetPath();
		Add(path, ps);
	}

	ParticleSystem* ParticleLibrary::Load(const std::string& filePath)
	{
		std::string result;
		if (!Serializer::Get().ValidateFile(filePath, "ParticleSystem", result))
			return nullptr;

		ParticleSystem* ps = new ParticleSystem(filePath, result);
		Add(ps);
		return ps;
	}

	ParticleSystem* ParticleLibrary::Load(const std::string& path, const std::string& filePath)
	{
		std::string result;
		if (!Serializer::Get().ValidateFile(filePath, "ParticleSystem", result))
			return nullptr;

		ParticleSystem* ps = new ParticleSystem(filePath, result);
		Add(path, ps);
		return ps;
	}

	ParticleSystem* ParticleLibrary::GetOrLoad(const std::string& path)
	{
		if (Exists(path))
		{
			return m_ParticleSystems[path];
		}
		else
		{
			return Load(path);
		}
	}

	ParticleSystem* ParticleLibrary::Get(const std::string& path)
	{
		ZE_CORE_ASSERT_INFO(Exists(path), "Partile system not found!");
		return m_ParticleSystems[path];
	}

	bool ParticleLibrary::Exists(const std::string& path) const
	{
		return m_ParticleSystems.find(path) != m_ParticleSystems.end();
	}


	ParticleManager::~ParticleManager()
	{
		CleanUp();
	}

	void ParticleManager::OnUpdate(DeltaTime dt)
	{
		for (uint32_t i = 0; i < m_ParticleSystems.size(); ++i)
		{
			if (m_ParticleSystems[i]->m_bPendingDestroy)
			{
				delete m_ParticleSystems[i];
				m_ParticleSystems.erase(m_ParticleSystems.cbegin() + i);
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

	void ParticleManager::CleanUp()
	{
		for (auto* ps : m_ParticleSystems)
		{
			delete ps;
		}
		m_ParticleSystems.clear();
	}

}
