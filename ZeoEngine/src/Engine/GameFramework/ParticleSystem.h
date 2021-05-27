#pragma once

#include <glm/glm.hpp>

#include "Engine/Renderer/Texture.h"
#include "Engine/Core/DeltaTime.h"
#include "Engine/Utils/PathUtils.h"
#include "Engine/GameFramework/Entity.h"
#include "Engine/Core/AssetLibrary.h"

namespace ZeoEngine {

	enum class ParticleVariationType
	{
		Constant,
		RandomInRange,
		UniformInRange
	};

	template<typename T>
	struct ParticleVariation
	{
	public:
		ParticleVariation();

		bool operator==(const ParticleVariation<T>& other) const;

		void SetConstant(T value);
		void SetUniform(T valueLow, T valueHigh);
		void SetRandom(T valueLow, T valueHigh);

		T Evaluate() const;
	private:
		T EvaluateRandom() const;
		T EvaluateUniform() const;

	public:
		T Val1 = T(), Val2 = T(); // Initialization is very important! Or unexpected behavior will happen when a new element of this type is added to the container via the "Insert" button
		ParticleVariationType VariationType = ParticleVariationType::Constant;
	};

	using ParticleInt = ParticleVariation<int32_t>;
	using ParticleFloat = ParticleVariation<float>;
	using ParticleVec2 = ParticleVariation<glm::vec2>;
	using ParticleVec3 = ParticleVariation<glm::vec3>;
	using ParticleColor = ParticleVariation<glm::vec4>;

	struct BurstData
	{
		bool operator==(const BurstData& other) const // <- This "const" is required!
		{
			return Time == other.Time && Amount == other.Amount;
		}

		float Time;
		ParticleInt Amount;
	};

	class ParticleSystemInstance;

	struct ParticleTemplate : public std::enable_shared_from_this<ParticleTemplate>
	{
	private:
		explicit ParticleTemplate(const std::string& path)
			: Path(PathUtils::GetRelativePath(path))
			, Name(PathUtils::GetNameFromPath(path))
		{
		}

	public:
		static Ref<ParticleTemplate> Create(const std::string& path = {});

		const std::string& GetPath() const { return Path; }
		const std::string& GetName() const { return Name; }

		void Reload();

		size_t GetParticleSystemInstanceCount() const { return ParticleSystemInstances.size(); }

		void AddParticleSystemInstance(const Ref<ParticleSystemInstance>& psInstance)
		{
			ParticleSystemInstances.push_back(psInstance);
		}

		void RemoveParticleSystemInstance(const Ref<ParticleSystemInstance>& psInstance)
		{
			auto it = std::find(ParticleSystemInstances.cbegin(), ParticleSystemInstances.cend(), psInstance);
			if (it != ParticleSystemInstances.cend())
			{
				ParticleSystemInstances.erase(it);
			}
		}

		void ResimulateAllParticleSystemInstances();

	private:
		void Deserialize();

	public:
		bool bIsLocalSpace = false;

		/** <= 0 means infinite loop */
		int32_t LoopCount = 0;
		float LoopDuration = 1.0f;

		/** Number of particles to spawn per second in total, < 0 means nothing */
		ParticleFloat SpawnRate;
		/** List of particles to spawn instantaneously per time. The time should be within [0.0, 1.0] */
		std::vector<BurstData> BurstList;

		ParticleVec3 InitialPosition;

		ParticleVec3 InitialRotation;
		ParticleVec3 RotationRate;

		ParticleVec3 SizeBegin, SizeEnd;

		ParticleVec3 InitialVelocity;
		/**
		 * How much velocity one particle will inherit from emitter. This param is only useful when bIsLocalSpace is false.
		 * This value should be in [(0.0, 0.0), (1.0, 1.0)]
		 */
		glm::vec3 InheritVelocityRatio{ 0.0f };

		ParticleColor ColorBegin, ColorEnd;

		ParticleFloat Lifetime;

		Asset<Texture2D> Texture;
		/**
		 * Defines how to divide texture into sub-images for UV animation.
		 * This variable contains number of columns in x and number of rows in y.
		 * By default, subUV animation will animate from left-up sub-image to right-down sub-image uniformly during particle's lifetime.
		 * Thus, you can change lifetime to control the animation speed.
		 */
		glm::vec2 SubImageSize{ 0.0f };

		uint32_t MaxParticles = 500;

	private:
		std::string Path;
		std::string Name;

		/** Caches all alive instances this template has instantiated, used to sync updates on value change */
		std::vector<Ref<ParticleSystemInstance>> ParticleSystemInstances;

	};

	struct ParticleSystemComponent;

	class ParticleSystemInstance
	{
		friend class ParticleViewportPanel;
		friend struct ParticleTemplate;
		friend class ParticleTemplateDataWidget;

	private:
		ParticleSystemInstance(const Asset<ParticleTemplate>& particleTemplate, Entity ownerEntity, const glm::vec3& positionOffset = glm::vec3{ 0.0f });

	public:
		/**
		 * Create a particle system instance.
		 * It will first remove old particle system instance from reference list and create a new one and add that to reference list.
		 * @param particleComp - Component to retrieve template and instance
		 */
		static void Create(ParticleSystemComponent& particleComp);

		Asset<ParticleTemplate>& GetParticleTemplate() { return m_ParticleTemplate; }

		void OnUpdate(DeltaTime dt);
		void OnRender();

		void Activate();
		void Deactivate();

	private:
		// Burst data specification
		struct BurstDataSpec
		{
			float Time;
			int32_t Amount;
			bool bIsProcessed;
		};

		// Emitter specification
		struct EmitterSpec
		{
			bool bIsLocalSpace;
			int32_t LoopCount;
			bool bIsInfiniteLoop;
			float LoopDuration;
			float SpawnRate;
			std::vector<BurstDataSpec> BurstList;
			Asset<Texture2D> Texture;
			glm::vec2 SubImageSize{ 0.0f };
			glm::vec2 TilingFactor{ 1.0f };
			glm::vec3 InheritVelocityRatio{ 0.0f };
			uint32_t MaxParticles;
		};

		// Particle runtime properties
		struct Particle
		{
			glm::vec3 Position{ 0.0f };

			glm::vec3 Rotation{ 0.0f };
			glm::vec3 RotationRate{ 0.0f };

			glm::vec3 SizeBegin{ 1.0f }, SizeEnd{ 1.0f };
			glm::vec3 Size{ 1.0f };

			glm::vec3 Velocity{ 0.0f };

			glm::vec4 ColorBegin{ 1.0f }, ColorEnd{ 1.0f };
			glm::vec4 Color{ 1.0f };

			float Lifetime = 1.0f;
			float LifeRemaining = 1.0f;

			glm::vec2 UvOffset{ 0.0f };

			bool bActive = false;
		};

		void TogglePause();
		bool IsPause() const { return m_bPauseUpdate; }
		void Resimulate();
		void ResetParticlePool();
		void Reset();

		void Reevaluate();
		void EvaluateEmitterProperties();
		void ReevaluateBurstList();
		void EvaluateParticleProperties(Particle& particle);

		bool Emit();
		void CalculateNextPoolIndex();

	private:
		entt::sigh<void()> m_OnSystemFinishedDel;
	public:
		/** Called when this particle system is about to be destroyed */
		entt::sink<void()> m_OnSystemFinished{ m_OnSystemFinishedDel };

	private:
		Asset<ParticleTemplate> m_ParticleTemplate;

		EmitterSpec m_EmitterSpec;
		std::vector<Particle> m_ParticlePool;
		int32_t m_PoolIndex;

		uint32_t m_ActiveParticleCount = 0;

		/** Particle's spawn offset from owner entity's translation */
		glm::vec3 m_PositionOffset{ 0.0f };
		/** Entity that contains the ParticleSystemComponent or ParticleSystemPreviewComponent */
		Entity m_OwnerEntity;
		glm::vec3 m_OwnerLastPosition{ 0.0f };

		/** This equals to Lifetime / (SubImageSize.x * SubImageSize.y) */
		float m_UvAnimationInterval = 0.0f;
		float m_Time = 0.0f, m_LoopStartTime = 0.0f, m_SpawnTime = 0.0f, m_BurstTime = 0.0f, m_UvAnimationTime = 0.0f;

		/** Used to guarantee that initial emitted particles can be rendered properly */
		bool m_bStartUpdate = false;
		bool m_bPauseUpdate = false;
		bool m_bActive = true;
		bool m_bSystemComplete = false;
		bool m_bPendingDestroy = false;

		bool m_bIsPreview = false;
#if ZE_WITH_EDITOR
		bool m_bFiniteLoopPrepareToRestart = true;
		float m_FiniteLoopRestartInterval = 1.0f;
		float m_FiniteLoopRestartTime = 0.0f;
#endif

	};

	struct ParticleLoader final : AssetLoader<ParticleLoader, ParticleTemplate>
	{
		Asset<ParticleTemplate> load(const std::string& path) const
		{
			return ParticleTemplate::Create(path);
		}
	};
	
	class ParticleLibrary : public AssetLibrary<ParticleLibrary, ParticleTemplate, ParticleLoader>
	{
	public:
		virtual Asset<ParticleTemplate> ReloadAsset(AssetPath path) override
		{
			if (path.IsEmpty()) return {};

			auto pTemplate = GetAsset(path);
			pTemplate->Reload();
			return pTemplate;
		}
	};

}
