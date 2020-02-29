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
		.property("VariationType", &ParticleVariation::variationType);

	registration::class_<ParticleInt>("ParticleInt")
		.property("Value", &ParticleInt::val1)
		.property("ValueHigh", &ParticleInt::val2)
		(
			metadata(PropertyMeta::HideCondition, "VariationType==Constant")
		);

	registration::class_<ParticleFloat>("ParticleFloat")
		.property("Value", &ParticleFloat::val1)
		.property("ValueHigh", &ParticleFloat::val2)
		(
			metadata(PropertyMeta::HideCondition, "VariationType==Constant")
		);

	registration::class_<ParticleVec2>("ParticleVec2")
		.property("Value", &ParticleVec2::val1)
		(
			policy::prop::bind_as_ptr
		)
		.property("ValueHigh", &ParticleVec2::val2)
		(
			policy::prop::bind_as_ptr,
			metadata(PropertyMeta::HideCondition, "VariationType==Constant")
		);

	registration::class_<ParticleVec3>("ParticleVec3")
		.property("Value", &ParticleVec3::val1)
		(
			policy::prop::bind_as_ptr
		)
		.property("ValueHigh", &ParticleVec3::val2)
		(
			policy::prop::bind_as_ptr,
			metadata(PropertyMeta::HideCondition, "VariationType==Constant")
		);

	registration::class_<ParticleColor>("ParticleColor")
		.property("Value", &ParticleColor::val1)
		(
			policy::prop::bind_as_ptr
		)
		.property("ValueHigh", &ParticleColor::val2)
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
		.property("LoopCount", &ParticleTemplate::loopCount)
		(
			metadata(PropertyMeta::Category, "Emitter"),
			metadata(PropertyMeta::Tooltip, u8"循环次数。若小于等于0, 则为无限循环")
		)
		.property("LoopDuration", &ParticleTemplate::loopDuration)
		(
			metadata(PropertyMeta::Category, "Emitter"),
			metadata(PropertyMeta::Tooltip, u8"每次循环的时长")
		)
		.property("SpawnRate", &ParticleTemplate::spawnRate)
		(
			policy::prop::bind_as_ptr,
			metadata(PropertyMeta::Category, "Emitter"),
			metadata(PropertyMeta::Tooltip, u8"每秒总共生成的粒子数，该变量决定粒子的生成速度。若小于0，生成速度由帧数决定")
		)
		.property("BurstList", &ParticleTemplate::burstList)
		(
			policy::prop::bind_as_ptr,
			metadata(PropertyMeta::Category, "Emitter"),
			metadata(PropertyMeta::Tooltip, u8"每个时间点一次性生成的粒子数")
		)
		.property("InitialPosition", &ParticleTemplate::initialPosition)
		(
			policy::prop::bind_as_ptr,
			metadata(PropertyMeta::Category, "Particle: Location"),
			metadata(PropertyMeta::Tooltip, u8"粒子的初始位置")
		)
		.property("InitialRotation", &ParticleTemplate::initialRotation)
		(
			policy::prop::bind_as_ptr,
			metadata(PropertyMeta::Category, "Particle: Rotation"),
			metadata(PropertyMeta::Tooltip, u8"粒子的初始旋转")
		)
		.property("RotationRate", &ParticleTemplate::rotationRate)
		(
			policy::prop::bind_as_ptr,
			metadata(PropertyMeta::Category, "Particle: Rotation"),
			metadata(PropertyMeta::Tooltip, u8"决定粒子的旋转速度")
		)
		.property("SizeBegin", &ParticleTemplate::sizeBegin)
		(
			policy::prop::bind_as_ptr,
			metadata(PropertyMeta::Category, "Particle: Size"),
			metadata(PropertyMeta::Tooltip, u8"决定粒子的初始大小")
		)
		.property("SizeEnd", &ParticleTemplate::sizeEnd)
		(
			policy::prop::bind_as_ptr,
			metadata(PropertyMeta::Category, "Particle: Size"),
			metadata(PropertyMeta::Tooltip, u8"决定粒子的最终大小")
		)
		.property("InitialVelocity", &ParticleTemplate::initialVelocity)
		(
			policy::prop::bind_as_ptr,
			metadata(PropertyMeta::Category, "Particle: Velocity"),
			metadata(PropertyMeta::Tooltip, u8"决定粒子的初始速度")
		)
		.property("InheritVelocity", &ParticleTemplate::inheritVelocity)
		(
			policy::prop::bind_as_ptr,
			metadata(PropertyMeta::Category, "Particle: Velocity"),
			metadata(PropertyMeta::Tooltip, u8"决定粒子所能继承的速度。该变量只有在非局部空间有效")
		)
		.property("ColorBegin", &ParticleTemplate::colorBegin)
		(
			policy::prop::bind_as_ptr,
			metadata(PropertyMeta::Category, "Particle: Color"),
			metadata(PropertyMeta::Tooltip, u8"决定粒子的初始颜色")
		)
		.property("ColorEnd", &ParticleTemplate::colorEnd)
		(
			policy::prop::bind_as_ptr,
			metadata(PropertyMeta::Category, "Particle: Color"),
			metadata(PropertyMeta::Tooltip, u8"决定粒子的最终颜色")
		)
		.property("LifeTime", &ParticleTemplate::lifeTime)
		(
			policy::prop::bind_as_ptr,
			metadata(PropertyMeta::Category, "Particle: Base"),
			metadata(PropertyMeta::Tooltip, u8"决定粒子的生命周期")
		)
		.property("Texture", &ParticleTemplate::texture)
		(
			metadata(PropertyMeta::Category, "Renderer: Texture"),
			metadata(PropertyMeta::Tooltip, u8"粒子的材质贴图")
		)
		.property("SubImageSize", &ParticleTemplate::subImageSize)
		(
			policy::prop::bind_as_ptr,
			metadata(PropertyMeta::Category, "Renderer: Texture"),
			metadata(PropertyMeta::Tooltip, u8"决定如何分割贴图来用于UV动画。x为列数，y为行数"),
			metadata(PropertyMeta::Min, 0)
		);

	registration::class_<ParticleSystem>("ParticleSystem")
#if WITH_EDITOR
		.method("OnPropertyValueEditChange", &ParticleSystem::OnPropertyValueEditChange)
#endif
		.property("ParticleEmitter", &ParticleSystem::m_ParticleTemplate)
		(
			policy::prop::bind_as_ptr
		);
}

namespace ZeoEngine {

	const char* ParticleSystem::ParticleSystemFileToken = "ParticleSystem";

	ParticleSystem::ParticleSystem(const std::string& filePath, const std::string& processedSrc)
		: m_PoolIndex(MAX_PARTICLE_COUNT - 1)
		, m_Path(filePath)
		, m_SpawnRate(1.0f / 30.0f)
	{
		m_ParticlePool.resize(MAX_PARTICLE_COUNT);

		// Extract name from file path
		// "assets/particles/Particle.zparticle" -> "Particle.zparticle"
		auto lastSlash = m_Path.find_last_of("/\\"); // find_last_of() will find ANY of the provided characters
		lastSlash = lastSlash == std::string::npos ? 0 : lastSlash + 1;
		auto count = m_Path.size() - lastSlash;
		m_FileName = m_Path.substr(lastSlash, count);

		Serializer::Get().Deserialize<ParticleSystem*>(processedSrc, [&]() {
			return rttr::variant(this);
		});
		EvaluateEmitterProperties();
	}

	ParticleSystem::ParticleSystem(const ParticleTemplate& particleTemplate, const glm::vec2& position, bool bAutoDestroy)
		: m_PoolIndex(MAX_PARTICLE_COUNT - 1)
		, m_ParticleTemplate(particleTemplate)
		, m_SpawnPosition(position)
		, m_bAutoDestroy(bAutoDestroy)
		, m_SpawnRate(1.0f / 30.0f)
	{
		m_ParticlePool.resize(MAX_PARTICLE_COUNT);
		EvaluateEmitterProperties();
	}

	ParticleSystem::ParticleSystem(const ParticleTemplate& particleTemplate, GameObject* attachToParent, bool bAutoDestroy, bool bIsInParticleEditor)
		: m_PoolIndex(MAX_PARTICLE_COUNT - 1)
		, m_ParticleTemplate(particleTemplate)
		, m_Parent(attachToParent)
		, m_bAutoDestroy(bAutoDestroy)
		, m_SpawnRate(1.0f / 30.0f)
#if WITH_EDITOR
		, m_bIsInParticleEditor(bIsInParticleEditor)
		, m_FiniteLoopRestartInterval(1.0f)
#endif
	{
		m_ParticlePool.resize(MAX_PARTICLE_COUNT);
		EvaluateEmitterProperties();
	}

#if WITH_EDITOR
	void ParticleSystem::OnPropertyValueEditChange(const rttr::property* prop, const rttr::property* outerProp)
	{
		if (outerProp)
		{
			if (prop->get_name() == "LoopCount" ||
				outerProp->get_name() == "SpawnRate" ||
				outerProp->get_name() == "BurstList" || outerProp->get_declaring_type().get_name() == "BurstData" ||
				prop->get_name() == "SubImageSize")
			{
				EvaluateEmitterProperties();
				Resimulate();
			}
		}
	}
#endif

	void ParticleSystem::EvaluateEmitterProperties()
	{
		m_LoopCount = m_ParticleTemplate.loopCount;
		m_bInfiniteLoop = m_LoopCount <= 0;

		// Spawn rate
		{
			float evaluated = 0.0f;
			switch (m_ParticleTemplate.spawnRate.variationType)
			{
			case ParticleVariationType::Constant:
				evaluated = m_ParticleTemplate.spawnRate.val1;
				break;
			case ParticleVariationType::RandomInRange:
				evaluated = RandomEngine::RandFloatInRange(m_ParticleTemplate.spawnRate.val1, m_ParticleTemplate.spawnRate.val2);
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
			m_BurstList.clear();
			for (const auto& burstData : m_ParticleTemplate.burstList)
			{
				switch (burstData.Amount.variationType)
				{
				case ParticleVariationType::Constant:
					m_BurstList.emplace(burstData.Time, burstData.Amount.val1);
					break;
				case ParticleVariationType::RandomInRange:
					m_BurstList.emplace(burstData.Time, (int32_t)RandomEngine::RandFloatInRange((float)burstData.Amount.val1, (float)burstData.Amount.val2));
					break;
				default:
					break;
				}
			}
		}

		// Texture
		{
			float x = m_ParticleTemplate.subImageSize.x == 0 ? 1.0f : 1.0f / m_ParticleTemplate.subImageSize.x;
			float y = m_ParticleTemplate.subImageSize.y == 0 ? 1.0f : 1.0f / m_ParticleTemplate.subImageSize.y;
			m_TilingFactor = { x, y };
		}
	}

	void ParticleSystem::EvaluateParticleProperties(Particle& particle)
	{
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
			if (m_SpawnPosition != glm::vec2(0.0f))
			{
				particle.position += m_SpawnPosition;
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
	}

	void ParticleSystem::Emit()
	{
		Particle& particle = m_ParticlePool[m_PoolIndex];
		particle.bActive = true;

		EvaluateParticleProperties(particle);

		if (m_PoolIndex == 0)
		{
			m_PoolIndex = MAX_PARTICLE_COUNT - 1;
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

		m_bSystemComplete = true;
		for (auto& particle : m_ParticlePool)
		{
			if (!particle.bActive)
				continue;

			if (particle.lifeRemaining <= 0.0f)
			{
				particle.bActive = false;
				continue;
			}

			m_bSystemComplete = false;

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
			if (m_OnSystemFinished)
			{
				m_OnSystemFinished();
			}
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
			if (m_ParticleTemplate.texture)
			{
				Renderer2D::DrawRotatedQuad(particle.position, particle.size, glm::radians(particle.rotation), m_ParticleTemplate.texture, m_TilingFactor, particle.uvOffset, particle.color);
			}
			else
			{
				Renderer2D::DrawRotatedQuad(particle.position, particle.size, glm::radians(particle.rotation), particle.color);
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
		m_LoopCount = m_ParticleTemplate.loopCount;
		m_Time = m_LoopStartTime = m_SpawnTime = m_BurstTime = m_UVAnimationTime = 0.0f;
	}

	void ParticleSystem::Deactivate()
	{
		m_bActive = false;
	}

	void ParticleSystem::Resimulate()
	{
		m_LoopCount = m_ParticleTemplate.loopCount;
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

	void ParticleManager::CleanUp()
	{
		for (auto* ps : m_ParticleSystems)
		{
			delete ps;
		}
		m_ParticleSystems.clear();
	}

}
