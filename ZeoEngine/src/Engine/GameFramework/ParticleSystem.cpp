#include "ZEpch.h"
#include "Engine/GameFramework/ParticleSystem.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/compatibility.hpp>

#include "Engine/Renderer/Renderer2D.h"
#include "Engine/Core/RandomEngine.h"
#include "Engine/Core/Serializer.h"
#include "Engine/GameFramework/Components.h"
#include "Engine/GameFramework/Entity.h"

namespace ZeoEngine {

	// Template definition
	// Ctor or dtor is required!
	template<typename T>
	ParticleVariation<T>::ParticleVariation() {}

	template<typename T>
	bool ParticleVariation<T>::operator==(const ParticleVariation<T>& other) const
	{
		return Val1 == other.Val1 && Val2 == other.Val2 && VariationType == other.VariationType;
	}

	template<typename T>
	void ParticleVariation<T>::SetConstant(T value)
	{
		VariationType = ParticleVariationType::Constant;
		Val1 = value;
	}
	template<typename T>
	void ParticleVariation<T>::SetUniform(T valueLow, T valueHigh)
	{
		VariationType = ParticleVariationType::UniformInRange;
		Val1 = valueLow;
		Val2 = valueHigh;
	}
	template<typename T>
	void ParticleVariation<T>::SetRandom(T valueLow, T valueHigh)
	{
		VariationType = ParticleVariationType::RandomInRange;
		Val1 = valueLow;
		Val2 = valueHigh;
	}

	template<typename T>
	T ParticleVariation<T>::Evaluate() const
	{
		switch (VariationType)
		{
			case ParticleVariationType::Constant:		return Val1;
			case ParticleVariationType::RandomInRange:	return EvaluateRandom();
			case ParticleVariationType::UniformInRange:	return EvaluateUniform();
		}
		return T();
	}
	template<>
	I32 ParticleVariation<I32>::EvaluateRandom() const
	{
		return RandomEngine::RandIntInRange(Val1, Val2);
	}
	template<>
	float ParticleVariation<float>::EvaluateRandom() const
	{
		return RandomEngine::RandFloatInRange(Val1, Val2);
	}
	template<>
	Vec2 ParticleVariation<Vec2>::EvaluateRandom() const
	{
		float x = RandomEngine::RandFloatInRange(Val1.x, Val2.x);
		float y = RandomEngine::RandFloatInRange(Val1.y, Val2.y);
		return { x, y };
	}
	template<>
	Vec3 ParticleVariation<Vec3>::EvaluateRandom() const
	{
		float x = RandomEngine::RandFloatInRange(Val1.x, Val2.x);
		float y = RandomEngine::RandFloatInRange(Val1.y, Val2.y);
		float z = RandomEngine::RandFloatInRange(Val1.z, Val2.z);
		return { x, y, z };
	}
	template<>
	Vec4 ParticleVariation<Vec4>::EvaluateRandom() const
	{
		float x = RandomEngine::RandFloatInRange(Val1.x, Val2.x);
		float y = RandomEngine::RandFloatInRange(Val1.y, Val2.y);
		float z = RandomEngine::RandFloatInRange(Val1.z, Val2.z);
		float w = RandomEngine::RandFloatInRange(Val1.w, Val2.w);
		return { x, y, z, w };
	}
	template<>
	I32 ParticleVariation<I32>::EvaluateUniform() const
	{
		return RandomEngine::RandIntInRange(Val1, Val2);
	}
	template<>
	float ParticleVariation<float>::EvaluateUniform() const
	{
		return RandomEngine::RandFloatInRange(Val1, Val2);
	}
	template<>
	Vec2 ParticleVariation<Vec2>::EvaluateUniform() const
	{
		float x = RandomEngine::RandFloatInRange(Val1.x, Val2.x);
		return { x, x };
	}
	template<>
	Vec3 ParticleVariation<Vec3>::EvaluateUniform() const
	{
		float x = RandomEngine::RandFloatInRange(Val1.x, Val2.x);
		return { x, x, x };
	}
	template<>
	Vec4 ParticleVariation<Vec4>::EvaluateUniform() const
	{
		float x = RandomEngine::RandFloatInRange(Val1.x, Val2.x);
		return { x, x, x, x };
	}

	// Template explicit specialization
	// NOTE: Trying to instantiate other types will cause linking error!
	template struct ParticleVariation<I32>;
	template struct ParticleVariation<float>;
	template struct ParticleVariation<Vec2>;
	template struct ParticleVariation<Vec3>;
	template struct ParticleVariation<Vec4>;

	ParticleTemplate::ParticleTemplate(const std::string& path)
		: AssetBase(path.empty() ? "ZID_DefaultParticleTemplate" : path)
	{
		// Default data
		Lifetime.SetRandom(0.75f, 1.5f);
		SpawnRate.SetConstant(30.0f);
		InitialRotation.SetRandom(Vec3{ 0.0f }, { 0.0f, 0.0f, 360.0f });
		RotationRate.SetRandom(Vec3{ 0.0f, 0.0f, 10.0f }, Vec3{ 0.0f, 0.0f, 50.0f });
		InitialVelocity.SetRandom({ -0.5f, 0.5f, 0.0f }, { 0.5f, 2.0f, 0.0f });
		SizeBegin.SetRandom({ 0.1f, 0.1f, 0.0f }, { 0.2f, 0.2f, 0.0f });
		SizeEnd.SetConstant(Vec3{ 0.0f });
		ColorBegin.SetConstant(Vec4{ 1.0f });
		ColorEnd.SetConstant(Vec4{ 0.0f });
	}

	Ref<ParticleTemplate> ParticleTemplate::Create(const std::string& path)
	{
		auto particleTemplate = CreateRef<ParticleTemplate>(path);
		particleTemplate->Deserialize(); // NOTE: Do not call it in constructor as it contains shared_from_this()!
		return particleTemplate;
	}

	// TODO:
	void ParticleTemplate::Reload(bool bIsCreate)
	{
		Deserialize();
		ResimulateAllParticleSystemInstances();
	}

	void ParticleTemplate::Serialize(const std::string& path)
	{
		std::string assetPath = PathUtils::GetNormalizedAssetPath(path);
		if (!PathUtils::DoesPathExist(assetPath)) return;

		SetID(std::move(assetPath));
		AssetSerializer::Serialize(GetID(), TypeId(), ParticleSystemPreviewComponent{ GetAssetHandle() });
	}

	void ParticleTemplate::Deserialize()
	{
		if (!PathUtils::DoesPathExist(GetID())) return;

		AssetSerializer::Deserialize(GetID(), TypeId(), ParticleSystemPreviewComponent{ GetAssetHandle() });
	}

	void ParticleTemplate::ResimulateAllParticleSystemInstances() const
	{
		for (const auto& psInstance : ParticleSystemInstances)
		{
			psInstance->Reevaluate();
		}
	}

	struct ParticleSystemInstance::Impl
	{
		/** Entity that contains the particle component */
		Entity OwnerEntity;

		ParticleSystemInstance::Impl(Entity ownerEntity)
			: OwnerEntity(ownerEntity) {}
	};

	ParticleSystemInstance::ParticleSystemInstance(const AssetHandle<ParticleTemplate>& particleTemplate, Entity* ownerEntity, const Vec3& positionOffset)
		: m_Impl(CreateScope<Impl>(*ownerEntity))
		, m_ParticleTemplate(particleTemplate)
		, m_PositionOffset(positionOffset)
	{
		m_bIsPreview = ownerEntity->HasComponent<ParticleSystemPreviewComponent>();
		Reevaluate();
	}

	void ParticleSystemInstance::Create(ParticleSystemComponent& particleComp)
	{
		// A way to allow std::make_shared() to access ParticleSystem's private constructor
		class ParticleSystemEnableShared : public ParticleSystemInstance
		{
		public:
			ParticleSystemEnableShared(const AssetHandle<ParticleTemplate>& particleTemplate, Entity* ownerEntity, const Vec3& positionOffset)
				: ParticleSystemInstance(particleTemplate, ownerEntity, positionOffset) {}
		};

		if (particleComp.ParticleTemplateAsset)
		{
			auto psInstance = CreateRef<ParticleSystemEnableShared>(particleComp.ParticleTemplateAsset, particleComp.ComponentHelper->GetOwnerEntity(), particleComp.PositionOffset);
			if (particleComp.Instance)
			{
				// Get old template from instance as template in component may have been updated
				particleComp.Instance->m_ParticleTemplate->RemoveParticleSystemInstance(particleComp.Instance);
			}
			particleComp.Instance = psInstance;
			particleComp.ParticleTemplateAsset->AddParticleSystemInstance(psInstance);
		}
	}

	void ParticleSystemInstance::Reevaluate()
	{
		EvaluateEmitterProperties();
		Resimulate();
	}

	void ParticleSystemInstance::EvaluateEmitterProperties()
	{
		m_EmitterSpec.bIsLocalSpace = m_ParticleTemplate->bIsLocalSpace;

		// Loop
		{
			m_EmitterSpec.LoopCount = m_ParticleTemplate->LoopCount;
			m_EmitterSpec.bIsInfiniteLoop = m_EmitterSpec.LoopCount <= 0;
			m_EmitterSpec.LoopDuration = m_ParticleTemplate->LoopDuration;
		}

		// TODO: Spawn rate
		{
			float spawnRate = m_ParticleTemplate->SpawnRate.Evaluate();
			m_EmitterSpec.SpawnRate = spawnRate < 0.0f ? 0.0f : spawnRate;
		}

		// Burst list
		{
			m_EmitterSpec.BurstList.clear();
			for (const auto& burstData : m_ParticleTemplate->BurstList)
			{
				I32 value = burstData.Amount.Evaluate();
				m_EmitterSpec.BurstList.push_back({ burstData.Time, value, false });
			}
		}

		// Texture
		{
			m_EmitterSpec.Texture = m_ParticleTemplate->Texture;
			m_EmitterSpec.SubImageSize = m_ParticleTemplate->SubImageSize;
			float x = m_ParticleTemplate->SubImageSize.x == 0 ? 1.0f : 1.0f / m_ParticleTemplate->SubImageSize.x;
			float y = m_ParticleTemplate->SubImageSize.y == 0 ? 1.0f : 1.0f / m_ParticleTemplate->SubImageSize.y;
			m_EmitterSpec.TilingFactor = { x, y };
		}

		// Velocity
		{
			m_EmitterSpec.InheritVelocityRatio = glm::clamp(m_ParticleTemplate->InheritVelocityRatio, Vec3{ 0.0f }, Vec3{ 1.0f });
		}

		m_EmitterSpec.MaxParticles = m_ParticleTemplate->MaxParticles;

		m_OwnerLastPosition = GetOwnerEntity()->GetTranslation();
	}

	void ParticleSystemInstance::ReevaluateBurstList()
	{
		for (auto i = 0; i < m_ParticleTemplate->BurstList.size(); ++i)
		{
			const auto& amountData = m_ParticleTemplate->BurstList[i].Amount;
			if (amountData.VariationType == ParticleVariationType::Constant) continue;
			
			m_EmitterSpec.BurstList[i].Amount = amountData.Evaluate();
		}
		for (auto& burstSpec : m_EmitterSpec.BurstList)
		{
			burstSpec.bIsProcessed = false;
		}
	}

	void ParticleSystemInstance::EvaluateParticleProperties(Particle& particle)
	{
		// Position
		{
			particle.Position = m_ParticleTemplate->InitialPosition.Evaluate();
			particle.Position += m_OwnerLastPosition + m_PositionOffset;
		}

		// Rotation
		{
			particle.Rotation = m_ParticleTemplate->InitialRotation.Evaluate();
			particle.RotationRate = m_ParticleTemplate->RotationRate.Evaluate();
		}

		// Size
		{
			particle.SizeBegin = m_ParticleTemplate->SizeBegin.Evaluate();
			particle.SizeEnd = m_ParticleTemplate->SizeEnd.Evaluate();
		}

		// Velocity
		{
			particle.Velocity = m_ParticleTemplate->InitialVelocity.Evaluate();
		}

		// Color
		{
			particle.ColorBegin = m_ParticleTemplate->ColorBegin.Evaluate();
			particle.ColorEnd = m_ParticleTemplate->ColorEnd.Evaluate();
		}

		// Life
		{
			particle.Lifetime = m_ParticleTemplate->Lifetime.Evaluate();
			particle.LifeRemaining = particle.Lifetime;
		}

		// TODO: Texture
		{
			auto count = m_ParticleTemplate->SubImageSize.x * m_ParticleTemplate->SubImageSize.y;
			m_UvAnimationInterval = count == 0 ? 0.0f : particle.Lifetime / count;
			// Initialize uv animation start point to left-up corner
			particle.UvOffset.x = 0;
			particle.UvOffset.y = m_ParticleTemplate->SubImageSize.y - 1.0f;
		}
	}

	bool ParticleSystemInstance::Emit()
	{
		if (m_PoolIndex < 0) return false;

		U32 oldPoolIndex = m_PoolIndex;
		// Iterate until we find an available (inactive) particle to activate
		while (m_ParticlePool[m_PoolIndex].bActive)
		{
			CalculateNextPoolIndex();
			if (m_PoolIndex == oldPoolIndex)
			{
				// Failed to find one, skip emitting
				return false;
			}
		}

		Particle& particle = m_ParticlePool[m_PoolIndex];
		particle.bActive = true;
		EvaluateParticleProperties(particle);

		CalculateNextPoolIndex();
		return true;
	}

	void ParticleSystemInstance::CalculateNextPoolIndex()
	{
		if (m_PoolIndex == 0)
		{
			m_PoolIndex = m_EmitterSpec.MaxParticles - 1;
		}
		else
		{
			m_PoolIndex = --m_PoolIndex % m_ParticlePool.size();
		}
	}

	void ParticleSystemInstance::OnUpdate(DeltaTime dt)
	{
		if (m_bPendingDestroy || !m_bStartUpdate || m_bPauseUpdate) return;

		m_Time += dt;
		if (m_bActive)
		{
			if (m_Time - m_LoopStartTime >= m_EmitterSpec.LoopDuration)
			{
				if (m_EmitterSpec.LoopCount != 0)
				{
					--m_EmitterSpec.LoopCount;
				}
				m_LoopStartTime = m_Time;
				m_BurstTime = m_Time;
				ReevaluateBurstList();
			}

			if ((m_EmitterSpec.bIsInfiniteLoop || m_EmitterSpec.LoopCount != 0))
			{
				// Process SpawnRate
				if (m_EmitterSpec.SpawnRate != 0.0f && (m_SpawnTime == 0.0f || m_Time - m_SpawnTime > 1.0f / m_EmitterSpec.SpawnRate))
				{
					U32 i = 0;
					U32 imax = static_cast<U32>(m_EmitterSpec.SpawnRate * dt);
					do 
					{
						if (!Emit()) break;

						++i;
					} while (i < imax);
					m_SpawnTime = m_Time;
				}
				// Process BurstList
				for (auto& burstSpec : m_EmitterSpec.BurstList)
				{
					if (m_Time - m_BurstTime >= burstSpec.Time * m_EmitterSpec.LoopDuration)
					{
						if (!burstSpec.bIsProcessed && burstSpec.Amount > 0)
						{
							burstSpec.bIsProcessed = true;
							for (I32 i = 0; i < burstSpec.Amount; ++i)
							{
								Emit();
							}
						}
					}
				}
			}
		}

		m_bSystemComplete = true;

		Vec3 ownerPosition = GetOwnerEntity()->GetTranslation();
		Vec3 ownerVelocity = (ownerPosition - m_OwnerLastPosition) / static_cast<float>(dt);
		Vec3 inheritVelocity = m_EmitterSpec.bIsLocalSpace ? ownerVelocity : ownerVelocity * m_EmitterSpec.InheritVelocityRatio;
		for (auto& particle : m_ParticlePool)
		{
			if (!particle.bActive) continue;

			if (particle.LifeRemaining <= 0.0f)
			{
				particle.bActive = false;
				continue;
			}

			m_bSystemComplete = false;

			particle.LifeRemaining -= dt;
			particle.Position += (particle.Velocity + inheritVelocity) * static_cast<float>(dt);
			particle.Rotation += particle.RotationRate * static_cast<float>(dt);
			float lifeRatio = particle.LifeRemaining / particle.Lifetime;
			particle.Size = glm::lerp(particle.SizeEnd, particle.SizeBegin, lifeRatio);
			particle.Color = glm::lerp(particle.ColorEnd, particle.ColorBegin, lifeRatio);
			// Process UV animation
			if (m_UvAnimationInterval != 0.0f && m_Time - m_UvAnimationTime >= m_UvAnimationInterval)
			{
				// When animation reaches the last sub-image, deactivate this particle immediately
				if (particle.UvOffset.x == m_EmitterSpec.SubImageSize.x - 1.0f && particle.UvOffset.y == 0.0f)
				{
					particle.bActive = false;
					continue;
				}
				// Next column
				particle.UvOffset.x += 1.0f;
				if (particle.UvOffset.x == m_EmitterSpec.SubImageSize.x)
				{
					particle.UvOffset.x = 0.0f;
					// Next row
					particle.UvOffset.y -= 1.0f;
				}
				m_UvAnimationTime = m_Time;
			}
		}
		m_OwnerLastPosition = ownerPosition;

		m_bSystemComplete = m_bSystemComplete && ((!m_EmitterSpec.bIsInfiniteLoop && m_EmitterSpec.LoopCount == 0) || !m_bActive);

#if ZE_WITH_EDITOR
		// If this completed particle system is previewed in ParticleSystem Editor, just restart it after a while
		if (m_bSystemComplete && m_bIsPreview)
		{
			if (m_bFiniteLoopPrepareToRestart)
			{
				m_bFiniteLoopPrepareToRestart = false;
				m_FiniteLoopRestartTime = m_Time;
			}
			if (m_Time - m_FiniteLoopRestartTime >= m_FiniteLoopRestartInterval)
			{
				Reset();
				return;
			}
		}
#endif

		if (m_bSystemComplete && !m_bIsPreview)
		{
			m_bPendingDestroy = true;
			m_OnSystemFinishedDel.publish();
		}
	}

	void ParticleSystemInstance::OnRender()
	{
		if (m_bPendingDestroy) return;

		m_bStartUpdate = true;
		m_ActiveParticleCount = 0;
		for (const auto& particle : m_ParticlePool)
		{
			if (!particle.bActive) continue;

			++m_ActiveParticleCount;
			Mat4 rotation = glm::rotate(Mat4(1.0f), glm::radians(particle.Rotation.x), { 1, 0, 0 }) *
				glm::rotate(Mat4(1.0f), glm::radians(particle.Rotation.y), { 0, 1, 0 }) *
				glm::rotate(Mat4(1.0f), glm::radians(particle.Rotation.z), { 0, 0, 1 });
			Mat4 transform = glm::translate(Mat4(1.0f), particle.Position) *
				rotation *
				glm::scale(Mat4(1.0f), particle.Size);
			if (m_EmitterSpec.Texture)
			{
				Renderer2D::DrawQuad(transform, m_EmitterSpec.Texture, m_EmitterSpec.TilingFactor, particle.UvOffset, particle.Color, *GetOwnerEntity());
			}
			else
			{
				Renderer2D::DrawQuad(transform, particle.Color, *GetOwnerEntity());
			}
		}
	}

	Entity* ParticleSystemInstance::GetOwnerEntity() const
	{
		return &m_Impl->OwnerEntity;
	}

	void ParticleSystemInstance::ResetParticlePool()
	{
		m_ParticlePool.clear();
		m_ParticlePool.resize(m_EmitterSpec.MaxParticles);
		m_PoolIndex = m_EmitterSpec.MaxParticles - 1;
	}

	void ParticleSystemInstance::TogglePause()
	{
		m_bPauseUpdate = !m_bPauseUpdate;
	}

	void ParticleSystemInstance::Resimulate()
	{
		ResetParticlePool();
		Reset();
	}

	void ParticleSystemInstance::Reset()
	{
		m_bPendingDestroy = false;
		m_EmitterSpec.LoopCount = m_ParticleTemplate->LoopCount;
		m_Time = m_LoopStartTime = m_SpawnTime = m_BurstTime = m_UvAnimationTime = 0.0f;
#if ZE_WITH_EDITOR
		m_bFiniteLoopPrepareToRestart = true;
#endif
	}

	void ParticleSystemInstance::Activate()
	{
		if (m_bPendingDestroy || m_bActive)
			return;

		m_bActive = true;
		m_EmitterSpec.LoopCount = m_ParticleTemplate->LoopCount;
		m_Time = m_LoopStartTime = m_SpawnTime = m_BurstTime = m_UvAnimationTime = 0.0f;
	}

	void ParticleSystemInstance::Deactivate()
	{
		m_bActive = false;
	}

}
