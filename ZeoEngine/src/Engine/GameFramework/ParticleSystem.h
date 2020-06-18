#pragma once

#include <glm/glm.hpp>
#include <rttr/registration.h>
#include <rttr/registration_friend.h>

#include "Engine/Renderer/Texture.h"
#include "Engine/Core/DeltaTime.h"
#include "Engine/Core/Delegate.h"

namespace ZeoEngine {

	class GameObject;

	enum class ParticleVariationType
	{
		Constant,
		RandomInRange,
		UniformInRange
	};

	struct ParticleVariation
	{
		virtual ~ParticleVariation() = 0 {}

		ParticleVariationType VariationType = ParticleVariationType::Constant;

		RTTR_ENABLE()
	};

	struct ParticleInt : public ParticleVariation
	{
		ParticleInt(int32_t constant = 0)
		{
			SetConstant(constant);
		}
		ParticleInt(int32_t low, int32_t high)
		{
			SetRandom(low, high);
		}

		void SetConstant(int32_t constant)
		{
			VariationType = ParticleVariationType::Constant;
			Val1 = constant;
		}
		void SetRandom(int32_t low, int32_t high)
		{
			VariationType = ParticleVariationType::RandomInRange;
			Val1 = low;
			Val2 = high;
		}

		int32_t Val1 = 0, Val2 = 0;

		RTTR_ENABLE(ParticleVariation)
	};

	struct ParticleFloat : public ParticleVariation
	{
		ParticleFloat(float constant = 0.0f)
		{
			SetConstant(constant);
		}
		ParticleFloat(float low, float high)
		{
			SetRandom(low, high);
		}

		void SetConstant(float constant)
		{
			VariationType = ParticleVariationType::Constant;
			Val1 = constant;
		}
		void SetRandom(float low, float high)
		{
			VariationType = ParticleVariationType::RandomInRange;
			Val1 = low;
			Val2 = high;
		}

		float Val1 = 0.0f, Val2 = 0.0f;

		RTTR_ENABLE(ParticleVariation)
	};

	struct ParticleVec2 : public ParticleVariation
	{
		ParticleVec2(const glm::vec2& constant = { 0.0f, 0.0f })
		{
			SetConstant(constant);
		}
		ParticleVec2(const glm::vec2& low, const glm::vec2& high)
		{
			SetRandom(low, high);
		}
		ParticleVec2(float low, float high)
		{
			SetRandom(low, high);
		}

		void SetConstant(const glm::vec2& constant)
		{
			VariationType = ParticleVariationType::Constant;
			Val1 = constant;
		}
		/** Get a random x value and a random y value repectively. */
		void SetRandom(const glm::vec2& low, const glm::vec2& high)
		{
			VariationType = ParticleVariationType::RandomInRange;
			Val1 = low;
			Val2 = high;
		}
		/** Get a random x, y value uniformly. */
		void SetRandom(float low, float high)
		{
			VariationType = ParticleVariationType::UniformInRange;
			Val1 = { low, low };
			Val2 = { high, high };
		}

		glm::vec2 Val1{ 0.0f }, Val2{ 0.0f };

		RTTR_ENABLE(ParticleVariation)
	};

	struct ParticleVec3 : public ParticleVariation
	{
		ParticleVec3(const glm::vec3& constant = { 0.0f, 0.0f, 0.0f })
		{
			SetConstant(constant);
		}
		ParticleVec3(const glm::vec3& low, const glm::vec3& high)
		{
			SetRandom(low, high);
		}
		ParticleVec3(float low, float high)
		{
			SetRandom(low, high);
		}

		void SetConstant(const glm::vec3& constant)
		{
			VariationType = ParticleVariationType::Constant;
			Val1 = constant;
		}
		/** Get a random x value, a random y value and a random z value repectively. */
		void SetRandom(const glm::vec3& low, const glm::vec3& high)
		{
			VariationType = ParticleVariationType::RandomInRange;
			Val1 = low;
			Val2 = high;
		}
		/** Get a random x, y, z value uniformly. */
		void SetRandom(float low, float high)
		{
			VariationType = ParticleVariationType::UniformInRange;
			Val1 = { low, low, low };
			Val2 = { high, high, high };
		}

		glm::vec3 Val1{ 0.0f }, Val2{ 0.0f };

		RTTR_ENABLE(ParticleVariation)
	};

	struct ParticleColor : public ParticleVariation
	{
		ParticleColor(const glm::vec4& constant = { 1.0f, 1.0f, 1.0f, 1.0f })
		{
			SetConstant(constant);
		}
		ParticleColor(const glm::vec4& low, const glm::vec4& high)
		{
			SetRandom(low, high);
		}

		void SetConstant(const glm::vec4& constant)
		{
			VariationType = ParticleVariationType::Constant;
			Val1 = constant;
		}
		void SetRandom(const glm::vec4& low, const glm::vec4& high)
		{
			VariationType = ParticleVariationType::RandomInRange;
			Val1 = low;
			Val2 = high;
		}

		glm::vec4 Val1{ 1.0f }, Val2{ 1.0f };

		RTTR_ENABLE(ParticleVariation)
	};

	struct BurstData
	{
		BurstData() = default;
		BurstData(float time, ParticleInt amount)
			: Time(time)
			, Amount(amount)
		{
		}
		BurstData(float time, int32_t amount)
			: Time(time)
			, Amount(amount)
		{
		}
		BurstData(float time, int32_t amountLow, int32_t amountHigh)
			: Time(time)
			, Amount(ParticleInt(amountLow, amountHigh))
		{
		}

		float Time = 0.0f;
		ParticleInt Amount = 0;

		RTTR_ENABLE()
	};

	struct ParticleTemplate
	{
		void AddBurstData(float time, int32_t amount)
		{
			BurstList.emplace_back(time, amount);
		}
		void AddBurstData(float time, int32_t amountLow, int32_t amountHigh)
		{
			BurstList.emplace_back(time, amountLow, amountHigh);
		}

		bool bIsLocalSpace = false;

		/** <= 0 means infinite loop */
		int32_t LoopCount = 0;
		float LoopDuration = 1.0f;

		/** Number of particles to spawn per second in total. < 0 means spawn at fps rate */
		ParticleFloat SpawnRate;
		/** List of burst of particles to spawn instantaneously per time. The time should be within (0.0, 1.0) */
		std::vector<BurstData> BurstList;

		ParticleVec2 InitialPosition;

		ParticleFloat InitialRotation;
		ParticleFloat RotationRate;

		ParticleVec2 SizeBegin, SizeEnd;

		ParticleVec2 InitialVelocity;
		/**
		 * How much velocity one particle will inherit. This param is only useful when bIsLocalSpace is false.
		 * This value will be clamp between (0.0f, 0.0f) and (1.0f, 1.0f)
		 */
		glm::vec2 InheritVelocity{ 0.0f };

		ParticleColor ColorBegin, ColorEnd;

		ParticleFloat Lifetime;

		Ref<Texture2D> Texture;
		/**
		 * Defines how to divide texture into sub-images for use by UV animation.
		 * This variable contains number of columns in x and number of rows in y.
		 * By default, subUV animation will animate from left-up sub-image to right-down sub-image uniformly during particle's lifetime.
		 * So, you can change lifetime to control the animation speed.
		 */
		glm::i32vec2 SubImageSize{ 0 };

		uint32_t MaxDrawParticles{ 500 };

		RTTR_ENABLE()
	};

	/**
	 * Particle system instances are generally stored in ParticleLibrary and ParticleManager.
	 * ParticleLibrary stores templated particle systems for preview purposes which are generated upon loading particle system assets.
	 * ParticleManager stores runtime particle systems which are generated from particle templates and will be destroyed on end play.
	 */
	class ParticleSystem
	{
		friend class ParticleLibrary;
		friend class ParticleManager;
		friend class Level;

		using SystemFinishedDel = Delegate<void()>;

	private:
		/** Construct a particle system from zparticle file. */
		ParticleSystem(const std::string& filePath, const std::string& processedSrc);
		ParticleSystem(const ParticleTemplate& particleTemplate, const glm::vec2& position = glm::vec2(0.0f), bool bAutoDestroy = true);
		ParticleSystem(const ParticleTemplate& particleTemplate, GameObject* attachToParent = nullptr, bool bAutoDestroy = true);

	public:
#if WITH_EDITOR
		void PostPropertyValueEditChange(const rttr::property* prop, const rttr::property* outerProp);
#endif

		const std::string& GetPath() const { return m_Path; }
		const std::string& GetFileName() const { return m_FileName; }
		bool GetAutoDestroy() const { return m_bAutoDestroy; }
		void SetAutoDestroy(bool bValue) { m_bAutoDestroy = bValue; }

		void DeserializeProperties(const std::string& processedSrc);

		const ParticleTemplate& GetParticleTemplate() const { return m_ParticleTemplate; }

		static ParticleSystem* CreateDefaultParticleSystem();

	private:
		void ResizeParticlePool();

	private:
		// Particle properties
		struct Particle
		{
			glm::vec2 Position{ 0.0f };

			float Rotation = 0.0f;
			float RotationRate = 0.0f;

			glm::vec2 SizeBegin{ 1.0f }, SizeEnd{ 1.0f };
			glm::vec2 Size{ 1.0f };

			glm::vec2 Velocity{ 0.0f };

			glm::vec4 ColorBegin{ 1.0f }, ColorEnd{ 1.0f };
			glm::vec4 Color{ 1.0f };

			float Lifetime = 1.0f;
			float LifeRemaining = 1.0f;

			glm::vec2 UvOffset{ 0.0f };

			bool bActive = false;
		};

	public:
		void EvaluateEmitterProperties();
		void EvaluateParticleProperties(Particle& particle);

		void OnUpdate(DeltaTime dt);
		void OnRender();
#if WITH_EDITOR
		void OnParticleViewImGuiRender();

		void SetParticleEditorPreviewMode(bool bIsInParticleEditor, bool bAutoDestroy)
		{
			m_bIsInParticleEditor = bIsInParticleEditor;
			m_bAutoDestroy = bAutoDestroy;
		}
#endif

		void OnDeserialized();

		void Activate();
		void Deactivate();

		void Resimulate();

	private:
		void Emit();

		void CalculateNextPoolIndex();

	public:
		/** Called when this particle system is about to be destroyed */
		SystemFinishedDel m_OnSystemFinished;

		static constexpr const char* ParticleSystemFileToken = "ParticleSystem";

	private:
		std::vector<Particle> m_ParticlePool;
		uint32_t m_PoolIndex;

		uint32_t m_ActiveParticleCount = 0;

		std::string m_Path;
		std::string m_FileName;

		ParticleTemplate m_ParticleTemplate;

		/** Particle's origin in world space */
		glm::vec2 m_SpawnPosition{ 0.0f };
		/** Parent GameObject this particle system attaches to, particle's position is affected by parent's position */
		GameObject* m_Parent = nullptr;
		bool m_bAutoDestroy = true;

		// Emitter properties
		float m_SpawnRate;
		struct BurstTimeData
		{
			BurstTimeData(float time)
				: Time(time)
				, bProcessed(false)
			{
			}

			bool operator<(const BurstTimeData& other) const
			{
				return Time < other.Time;
			}

			float Time;
			bool bProcessed;
		};
		std::map<BurstTimeData, uint32_t> m_BurstList;
		int32_t m_LoopCount;
		bool m_bInfiniteLoop;
		glm::vec2 m_TilingFactor{ 1.0f };

		/** This equals to Lifetime / (SubImageSize.x * SubImageSize.y) */
		float m_UVAnimationInterval = 0.0f;
		float m_Time = 0.0f, m_LoopStartTime = 0.0f, m_SpawnTime = 0.0f, m_BurstTime = 0.0f, m_UVAnimationTime = 0.0f;

		/** Used to guarantee that initial emitted particles can be rendered properly */
		bool m_bStartUpdate = false;
		bool m_bActive = true;
		bool m_bSystemComplete = false;
		bool m_bPendingDestroy = false;

#if WITH_EDITOR
		bool m_bIsInParticleEditor = false;
		bool m_bFiniteLoopPrepareToRestart = true;
		float m_FiniteLoopRestartInterval = 1.0f;
		float m_FiniteLoopRestartTime = 0.0f;
#endif

		RTTR_ENABLE()
		RTTR_REGISTRATION_FRIEND
	};

	class ParticleLibrary
	{
		friend class EngineLayer;

	private:
		ParticleLibrary() = default;
	public:
		ParticleLibrary(const ParticleLibrary&) = delete;
		ParticleLibrary& operator=(const ParticleLibrary&) = delete;
		~ParticleLibrary();

		void Add(ParticleSystem* ps);
	private:
		void Add(const std::string& path, ParticleSystem* ps);
	public:
		ParticleSystem* Load(const std::string& filePath);
		ParticleSystem* GetOrLoad(const std::string& path);
		ParticleSystem* UpdateOrLoad(const std::string& path);

		ParticleSystem* Get(const std::string& path);

		bool Exists(const std::string& path) const;

		const std::unordered_map<std::string, ParticleSystem*>& GetParticlesMap() const { return m_ParticleSystems; }

	private:
		/** The containing particle systems will not be used during gameplay, instead they are regarded as templates for instantiating */
		std::unordered_map<std::string, ParticleSystem*> m_ParticleSystems;

	};

	class ParticleManager
	{
		friend class Level;

	private:
		ParticleManager()
		{
			ZE_CORE_TRACE("Particle manager initialized!");
		}
	public:
		ParticleManager(const ParticleManager&) = delete;
		ParticleManager& operator=(const ParticleManager&) = delete;
		~ParticleManager();

		void OnUpdate(DeltaTime dt);
		void OnRender();

		void AddParticleSystem(ParticleSystem* particleSystem);

		void CleanUp();

	private:
		std::vector<ParticleSystem*> m_ParticleSystems;
	};

}
