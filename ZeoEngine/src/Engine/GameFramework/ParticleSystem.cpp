#include "ZEpch.h"
#include "Engine/GameFramework/ParticleSystem.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/compatibility.hpp>
#include <imgui.h>

#include "Engine/Renderer/Renderer2D.h"
#include "Engine/Core/RandomEngine.h"
#include "Engine/GameFramework/GameObject.h"
#include "Engine/Core/SceneSerializer.h"
#include "Engine/GameFramework/Components.h"

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
		case ParticleVariationType::Constant:
			return Val1;
		case ParticleVariationType::RandomInRange:
			return EvaluateRandom();
		case ParticleVariationType::UniformInRange:
			return EvaluateUniform();
		default:
			break;
		}
		return T();
	}
	template<>
	float ParticleVariation<float>::EvaluateRandom() const
	{
		return RandomEngine::RandFloatInRange(Val1, Val2);
	}
	template<>
	int32_t ParticleVariation<int32_t>::EvaluateRandom() const
	{
		return RandomEngine::RandIntInRange(Val1, Val2);
	}
	template<>
	glm::vec2 ParticleVariation<glm::vec2>::EvaluateRandom() const
	{
		float x = RandomEngine::RandFloatInRange(Val1.x, Val2.x);
		float y = RandomEngine::RandFloatInRange(Val1.y, Val2.y);
		return { x, y };
	}
	template<>
	glm::vec3 ParticleVariation<glm::vec3>::EvaluateRandom() const
	{
		float x = RandomEngine::RandFloatInRange(Val1.x, Val2.x);
		float y = RandomEngine::RandFloatInRange(Val1.y, Val2.y);
		float z = RandomEngine::RandFloatInRange(Val1.z, Val2.z);
		return { x, y, z };
	}
	template<>
	glm::vec4 ParticleVariation<glm::vec4>::EvaluateRandom() const
	{
		float x = RandomEngine::RandFloatInRange(Val1.x, Val2.x);
		float y = RandomEngine::RandFloatInRange(Val1.y, Val2.y);
		float z = RandomEngine::RandFloatInRange(Val1.z, Val2.z);
		float w = RandomEngine::RandFloatInRange(Val1.w, Val2.w);
		return { x, y, z, w };
	}
	template<typename T>
	T ParticleVariation<T>::EvaluateUniform() const
	{
		// Not implemented
		ZE_CORE_ASSERT(false);
		return T();
	}

	// Template explicit specialization
	template struct ParticleVariation<int32_t>;
	template struct ParticleVariation<float>;
	template struct ParticleVariation<glm::vec2>;
	template struct ParticleVariation<glm::vec3>;
	template struct ParticleVariation<glm::vec4>;
	// Trying to instantiate other types will cause linking error!

	ParticleSystem::ParticleSystem(const Ref<ParticleTemplate>& particleTemplate, bool bIsPreview)
		: m_ParticleTemplate(particleTemplate)
		, m_SpawnPosition({ 0.0f })
		, m_bIsPreview(bIsPreview)
		, m_bAutoDestroy(false)
	{
		ResizeParticlePool();
		EvaluateEmitterProperties();
	}

	//ParticleSystem::ParticleSystem(const Ref<ParticleTemplate>& particleTemplate, const glm::vec2& position, bool bAutoDestroy)
	//	: m_PoolIndex(particleTemplate->MaxDrawParticles - 1)
	//	, m_ParticleTemplate(particleTemplate)
	//	, m_SpawnPosition(position)
	//	, m_bAutoDestroy(bAutoDestroy)
	//	, m_SpawnRate(30.0f)
	//{
	//	ResizeParticlePool();
	//	EvaluateEmitterProperties();
	//}

	//ParticleSystem::ParticleSystem(const Ref<ParticleTemplate>& particleTemplate, GameObject* attachToParent, bool bAutoDestroy)
	//	: m_PoolIndex(particleTemplate->MaxDrawParticles - 1)
	//	, m_ParticleTemplate(particleTemplate)
	//	, m_Parent(attachToParent)
	//	, m_bAutoDestroy(bAutoDestroy)
	//	, m_SpawnRate(30.0f)
	//{
	//	ResizeParticlePool();
	//	EvaluateEmitterProperties();
	//}

#if WITH_EDITOR
	//void ParticleSystem::PostPropertyValueEditChange(const rttr::property* prop, const rttr::property* outerProp)
	//{
	//	ZE_CORE_ASSERT(prop);

	//	if (prop->get_name() == "LoopCount" ||
	//		outerProp->get_name() == "SpawnRate" ||
	//		outerProp->get_name() == "BurstList" /* e.g. BurstData.Time */ || outerProp->get_declaring_type().get_name() == "BurstData" /* e.g. BurstData.Amount.Value */ ||
	//		prop->get_name() == "SubImageSize")
	//	{
	//		EvaluateEmitterProperties();
	//		Resimulate();
	//	}
	//	else if (prop->get_name() == "MaxDrawParticles")
	//	{
	//		ResizeParticlePool();
	//	}
	//}
#endif

	void ParticleSystem::ResizeParticlePool()
	{
		m_ParticlePool.resize(m_ParticleTemplate->MaxDrawParticles);
		m_PoolIndex = m_ParticleTemplate->MaxDrawParticles - 1;
	}

	void ParticleSystem::EvaluateEmitterProperties()
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
				int32_t value = burstData.Amount.Evaluate();
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
			m_EmitterSpec.InheritVelocity = glm::clamp(m_EmitterSpec.InheritVelocity, glm::vec2(0.0f), glm::vec2(1.0f));
		}

		m_EmitterSpec.MaxDrawParticles = m_ParticleTemplate->MaxDrawParticles;
	}

	void ParticleSystem::ReevaluateBurstList()
	{
		for (int32_t i = 0; i < m_ParticleTemplate->BurstList.size(); ++i)
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

	void ParticleSystem::EvaluateParticleProperties(Particle& particle)
	{
		// TODO: Position
		{
			particle.Position = m_ParticleTemplate->InitialPosition.Evaluate();
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
			m_UVAnimationInterval = count == 0 ? 0.0f : particle.Lifetime / count;
			// Initialize uv animation start point to left-up corner
			particle.UvOffset.x = 0;
			particle.UvOffset.y = m_ParticleTemplate->SubImageSize.y - 1.0f;
		}
	}

	void ParticleSystem::Emit()
	{
		uint32_t oldPoolIndex = m_PoolIndex;
		// Iterate until we find an available (inactive) particle to activate
		while (m_ParticlePool[m_PoolIndex].bActive)
		{
			CalculateNextPoolIndex();
			if (m_PoolIndex == oldPoolIndex)
			{
				// Failed to find one, skip emitting
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
			m_PoolIndex = m_EmitterSpec.MaxDrawParticles - 1;
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
					uint32_t i = 0;
					uint32_t imax = static_cast<uint32_t>(m_EmitterSpec.SpawnRate * dt);
					do 
					{
						Emit();
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
							for (int32_t i = 0; i < burstSpec.Amount; ++i)
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
				velocity = m_EmitterSpec.bIsLocalSpace ?
					m_Parent->GetVelocity() + particle.Velocity :
					m_Parent->GetVelocity() * m_EmitterSpec.InheritVelocity + particle.Velocity;
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
				m_UVAnimationTime = m_Time;
			}
		}

		m_bSystemComplete = m_bSystemComplete && ((!m_EmitterSpec.bIsInfiniteLoop && m_EmitterSpec.LoopCount == 0) || !m_bActive);

#if WITH_EDITOR
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
				Resimulate();
				return;
			}
		}
#endif

		if (m_bAutoDestroy && m_bSystemComplete)
		{
			m_bPendingDestroy = true;
			m_OnSystemFinishedDel.publish();
		}
	}

	void ParticleSystem::OnRender()
	{
		if (m_bPendingDestroy)
			return;

		m_bStartUpdate = true;
		m_ActiveParticleCount = 0;
		for (const auto& particle : m_ParticlePool)
		{
			if (!particle.bActive)
				continue;

			++m_ActiveParticleCount;
			if (m_EmitterSpec.Texture)
			{
				Renderer2D::DrawRotatedQuad(particle.Position, particle.Size, glm::radians(particle.Rotation), m_EmitterSpec.Texture, m_EmitterSpec.TilingFactor, particle.UvOffset, particle.Color);
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
			const float indent = (ImGui::GetContentRegionAvail().x - textSize.x) * 0.5f;
			// Display completed text at the top center of Particle View window
			ImGui::GetForegroundDrawList()->AddText(ImVec2(particleViewStartDrawPos.x + indent, particleViewStartDrawPos.y), IM_COL32_WHITE, "Completed");
		}
	}
#endif

	void ParticleSystem::Activate()
	{
		if (m_bPendingDestroy || m_bActive)
			return;

		m_bActive = true;
		m_EmitterSpec.LoopCount = m_ParticleTemplate->LoopCount;
		m_Time = m_LoopStartTime = m_SpawnTime = m_BurstTime = m_UVAnimationTime = 0.0f;
	}

	void ParticleSystem::Deactivate()
	{
		m_bActive = false;
	}

	void ParticleSystem::Resimulate()
	{
		m_EmitterSpec.LoopCount = m_ParticleTemplate->LoopCount;
		m_Time = m_LoopStartTime = m_SpawnTime = m_BurstTime = m_UVAnimationTime = 0.0f;
#if WITH_EDITOR
		m_bFiniteLoopPrepareToRestart = true;
#endif
	}

	void ParticleLibrary::Add(const std::string& path, const Ref<ParticleTemplate>& pTemplate)
	{
		if (!Exists(path))
		{
			m_ParticleTemplates[GetCanonicalPath(path)] = pTemplate;
		}
	}

	void ParticleLibrary::Add(const Ref<ParticleTemplate>& pTemplate)
	{
		const std::string& path = pTemplate->GetPath();
		Add(path, pTemplate);
	}

	Ref<ParticleTemplate> ParticleLibrary::Load(const std::string& path)
	{
		auto pTemplate = CreateRef<ParticleTemplate>(path);
		TypeSerializer serializer(path);
		serializer.Deserialize(ParticleSystemPreviewComponent{ pTemplate }, AssetType::ParticleTemplate);
		Add(pTemplate);
		return pTemplate;
	}

	Ref<ParticleTemplate> ParticleLibrary::GetOrLoad(const std::string& path)
	{
		if (Exists(path))
		{
			return m_ParticleTemplates[GetCanonicalPath(path)];
		}
		else
		{
			return Load(path);
		}
	}

	Ref<ParticleTemplate> ParticleLibrary::Get(const std::string& path)
	{
		ZE_CORE_ASSERT_INFO(Exists(path), "Particle template not found!");
		return m_ParticleTemplates[GetCanonicalPath(path)];
	}

	bool ParticleLibrary::Exists(const std::string& path) const
	{
		return m_ParticleTemplates.find(GetCanonicalPath(path)) != m_ParticleTemplates.end();
	}

}
