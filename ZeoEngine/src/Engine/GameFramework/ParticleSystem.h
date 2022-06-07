#pragma once

#include <glm/glm.hpp>

#include "Engine/Renderer/Texture.h"
#include "Engine/Core/DeltaTime.h"
#include "Engine/Core/Asset.h"
#include "Engine/Core/AssetLibrary.h"

namespace ZeoEngine {

	struct ParticleSystemComponent;
	class Entity;

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

	using ParticleInt = ParticleVariation<I32>;
	using ParticleFloat = ParticleVariation<float>;
	using ParticleVec2 = ParticleVariation<Vec2>;
	using ParticleVec3 = ParticleVariation<Vec3>;
	using ParticleColor = ParticleVariation<Vec4>;

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

	class ParticleTemplate : public AssetBase<ParticleTemplate>
	{
	public:
		explicit ParticleTemplate(const std::string& path);

		static Ref<ParticleTemplate> Create(const std::string& path);

		SizeT GetParticleSystemInstanceCount() const { return ParticleSystemInstances.size(); }

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

		void ResimulateAllParticleSystemInstances() const;

		virtual void Serialize(const std::string& path) override;
		virtual void Deserialize() override;

		void Reload();

	public:
		bool bIsLocalSpace = false;

		/** <= 0 means infinite loop */
		I32 LoopCount = 0;
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
		Vec3 InheritVelocityRatio{ 0.0f };

		ParticleColor ColorBegin, ColorEnd;

		ParticleFloat Lifetime;

		AssetHandle<Texture2D> Texture;
		/**
		 * Defines how to divide texture into sub-images for UV animation.
		 * This variable contains number of columns in x and number of rows in y.
		 * By default, subUV animation will animate from left-up sub-image to right-down sub-image uniformly during particle's lifetime.
		 * Thus, you can change lifetime to control the animation speed.
		 */
		Vec2 SubImageSize{ 0.0f };

		U32 MaxParticles = 500;

	private:
		/** Caches all alive instances this template has instantiated, used to sync updates on value change */
		std::vector<Ref<ParticleSystemInstance>> ParticleSystemInstances;

	};

	REGISTER_ASSET(ParticleTemplate,
	Ref<ParticleTemplate> operator()(const std::string& path) const
	{
		return ParticleTemplate::Create(path);
	},
	static AssetHandle<ParticleTemplate> GetDefaultParticleTemplate()
	{
		// TODO: Move to disk asset
		return Get().LoadAsset("ZID_DefaultParticleTemplate");
	})

	class ParticleSystemInstance
	{
		friend class ParticleEditorViewPanel;
		friend class ParticleTemplate;
		friend class ParticleTemplateDataWidget;

	private:
		ParticleSystemInstance(const AssetHandle<ParticleTemplate>& particleTemplate, Entity* ownerEntity, const Vec3& positionOffset = Vec3{ 0.0f });

	public:
		/**
		 * Create a particle system instance.
		 * It will first remove old particle system instance from reference list and create a new one and add that to reference list.
		 * @param particleComp - Component to retrieve template and instance
		 */
		static void Create(ParticleSystemComponent& particleComp);

		const AssetHandle<ParticleTemplate>& GetParticleTemplate() { return m_ParticleTemplate; }

		void OnUpdate(DeltaTime dt);
		void OnRender();

		void Activate();
		void Deactivate();

		Entity* GetOwnerEntity() const;

	private:
		void TogglePause();
		bool IsPause() const { return m_bPauseUpdate; }
		void Resimulate();
		void ResetParticlePool();
		void Reset();

		void Reevaluate();
		void EvaluateEmitterProperties();
		void ReevaluateBurstList();
		struct Particle;
		void EvaluateParticleProperties(Particle& particle);

		bool Emit();
		void CalculateNextPoolIndex();

	private:
		entt::sigh<void()> m_OnSystemFinishedDel;
	public:
		/** Called when this particle system is about to be destroyed */
		entt::sink<entt::sigh<void()>> m_OnSystemFinished{ m_OnSystemFinishedDel };

	private:
		// Burst data specification
		struct BurstDataSpec
		{
			float Time;
			I32 Amount;
			bool bIsProcessed;
		};

		// Emitter specification
		struct EmitterSpec
		{
			bool bIsLocalSpace;
			I32 LoopCount;
			bool bIsInfiniteLoop;
			float LoopDuration;
			float SpawnRate;
			std::vector<BurstDataSpec> BurstList;
			AssetHandle<Texture2D> Texture;
			Vec2 SubImageSize{ 0.0f };
			Vec2 TilingFactor{ 1.0f };
			Vec3 InheritVelocityRatio{ 0.0f };
			U32 MaxParticles;
		};

		// Particle runtime properties
		struct Particle
		{
			Vec3 Position{ 0.0f };

			Vec3 Rotation{ 0.0f };
			Vec3 RotationRate{ 0.0f };

			Vec3 SizeBegin{ 1.0f }, SizeEnd{ 1.0f };
			Vec3 Size{ 1.0f };

			Vec3 Velocity{ 0.0f };

			Vec4 ColorBegin{ 1.0f }, ColorEnd{ 1.0f };
			Vec4 Color{ 1.0f };

			float Lifetime = 1.0f;
			float LifeRemaining = 1.0f;

			Vec2 UvOffset{ 0.0f };

			bool bActive = false;
		};

		struct Impl;
		Scope<Impl> m_Impl;

		AssetHandle<ParticleTemplate> m_ParticleTemplate;

		EmitterSpec m_EmitterSpec;
		std::vector<Particle> m_ParticlePool;
		I32 m_PoolIndex;

		U32 m_ActiveParticleCount = 0;

		/** Particle's spawn offset from owner entity's translation */
		Vec3 m_PositionOffset{ 0.0f };
		
		Vec3 m_OwnerLastPosition{ 0.0f };

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

}
