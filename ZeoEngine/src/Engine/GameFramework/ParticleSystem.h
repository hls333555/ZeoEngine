#pragma once

#include <glm/glm.hpp>
#include <rttr/registration.h>
#include <rttr/registration_friend.h>

#include "Engine/Renderer/Texture.h"
#include "Engine/Core/DeltaTime.h"

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
		ParticleVariationType variationType = ParticleVariationType::Constant;

		RTTR_ENABLE()
	};

	struct ParticleInt : public ParticleVariation
	{
		ParticleInt(int32_t constant)
			: val1(constant)
		{
			variationType = ParticleVariationType::Constant;
		}
		ParticleInt(int32_t low, int32_t high)
			: val1(low)
			, val2(high)
		{
			variationType = ParticleVariationType::RandomInRange;
		}

		void SetConstant(int32_t constant)
		{
			variationType = ParticleVariationType::Constant;
			val1 = constant;
		}
		void SetRandom(int32_t low, int32_t high)
		{
			variationType = ParticleVariationType::RandomInRange;
			val1 = low;
			val2 = high;
		}

		int32_t val1 = 0, val2 = 0;

		RTTR_ENABLE(ParticleVariation)
	};

	struct ParticleFloat : public ParticleVariation
	{
		void SetConstant(float constant)
		{
			variationType = ParticleVariationType::Constant;
			val1 = constant;
		}
		void SetRandom(float low, float high)
		{
			variationType = ParticleVariationType::RandomInRange;
			val1 = low;
			val2 = high;
		}

		float val1 = 0.0f, val2 = 0.0f;

		RTTR_ENABLE(ParticleVariation)
	};

	struct ParticleVec2 : public ParticleVariation
	{
		void SetConstant(const glm::vec2& constant)
		{
			variationType = ParticleVariationType::Constant;
			val1 = constant;
		}
		/** Get a random x value and a random y value repectively. */
		void SetRandom(const glm::vec2& low, const glm::vec2& high)
		{
			variationType = ParticleVariationType::RandomInRange;
			val1 = low;
			val2 = high;
		}
		/** Get a random x, y value uniformly. */
		void SetRandom(float low, float high)
		{
			variationType = ParticleVariationType::UniformInRange;
			val1 = { low, low };
			val2 = { high, high };
		}

		glm::vec2 val1{ 0.0f, 0.0f }, val2{ 0.0f, 0.0f };

		RTTR_ENABLE(ParticleVariation)
	};

	struct ParticleVec3 : public ParticleVariation
	{
		void SetConstant(const glm::vec3& constant)
		{
			variationType = ParticleVariationType::Constant;
			val1 = constant;
		}
		/** Get a random x value, a random y value and a random z value repectively. */
		void SetRandom(const glm::vec3& low, const glm::vec3& high)
		{
			variationType = ParticleVariationType::RandomInRange;
			val1 = low;
			val2 = high;
		}
		/** Get a random x, y, z value uniformly. */
		void SetRandom(float low, float high)
		{
			variationType = ParticleVariationType::UniformInRange;
			val1 = { low, low, low };
			val2 = { high, high, high };
		}

		glm::vec3 val1{ 0.0f, 0.0f, 0.0f }, val2{ 0.0f, 0.0f, 0.0f };

		RTTR_ENABLE(ParticleVariation)
	};

	struct ParticleColor : public ParticleVariation
	{
		void SetConstant(const glm::vec4& constant)
		{
			variationType = ParticleVariationType::Constant;
			val1 = constant;
		}
		void SetRandom(const glm::vec4& low, const glm::vec4& high)
		{
			variationType = ParticleVariationType::RandomInRange;
			val1 = low;
			val2 = high;
		}

		glm::vec4 val1{ 1.0f, 1.0f, 1.0f, 1.0f }, val2{ 1.0f, 1.0f, 1.0f, 1.0f };

		RTTR_ENABLE(ParticleVariation)
	};

	struct BurstData
	{
		BurstData() = default;
		BurstData(float time, ParticleInt amount)
			: Time(time)
			, Amount(amount)
		{}
		BurstData(float time, int32_t amount)
			: Time(time)
			, Amount(amount)
		{}
		BurstData(float time, int32_t amountLow, int32_t amountHigh)
			: Time(time)
			, Amount(ParticleInt(amountLow, amountHigh))
		{}

		float Time = 0.0f;
		ParticleInt Amount = 0;

		RTTR_ENABLE()
	};

	struct ParticleTemplate
	{
		void AddBurstData(float time, int32_t amount)
		{
			burstList.emplace_back(time, amount);
		}
		void AddBurstData(float time, int32_t amountLow, int32_t amountHigh)
		{
			burstList.emplace_back(time, amountLow, amountHigh);
		}

		bool bIsLocalSpace = false;

		/** <= 0 means infinite loop */
		int32_t loopCount = 0;
		float loopDuration = 1.0f;

		/** Number of particles to spawn per second in total. < 0 means spawn at fps rate */
		ParticleFloat spawnRate;
		/** List of burst of particles to spawn instantaneously per time. The time should be within (0.0, 1.0) */
		std::vector<BurstData> burstList;

		ParticleVec2 initialPosition;

		ParticleFloat initialRotation;
		ParticleFloat rotationRate;

		ParticleVec2 sizeBegin, sizeEnd;

		ParticleVec2 initialVelocity;
		/**
		 * How much velocity one particle will inherit. This param is only useful when bIsLocalSpace is false.
		 * This value will be clamp between (0.0f, 0.0f) and (1.0f, 1.0f)
		 */
		glm::vec2 inheritVelocity{ 0.0f, 0.0f };

		ParticleColor colorBegin, colorEnd;

		ParticleFloat lifeTime;

		Ref<Texture2D> texture;
		/**
		 * Defines how to divide texture into sub-images for use by UV animation.
		 * This variable contains number of columns in x and number of rows in y.
		 * By default, subUV animation will animate from left-up sub-image to right-down sub-image uniformly during particle's lifetime.
		 * So, you can change lifetime to control the animation speed.
		 */
		glm::i32vec2 subImageSize{ 0, 0 };

		RTTR_ENABLE()
	};

#define MAX_PARTICLE_COUNT 1000

	class ParticleSystem
	{
		friend class ParticleLibrary;
		friend class ParticleManager;

	public:
		ParticleSystem() = default;
	private:
		ParticleSystem(const std::string& filePath, const std::string& processedSrc);
	public:
		ParticleSystem(const ParticleTemplate& particleTemplate, const glm::vec2& position = glm::vec2(0.0f), bool bAutoDestroy = true);
		ParticleSystem(const ParticleTemplate& particleTemplate, GameObject* attachToParent = nullptr, bool bAutoDestroy = true, bool bIsInParticleEditor = false);

#if WITH_EDITOR
		void OnPropertyValueEditChange(const rttr::property* prop, const rttr::property* outerProp);
#endif

		const std::string& GetPath() const { return m_Path; }
		const std::string& GetFileName() const { return m_FileName; }

		const ParticleTemplate& GetParticleTemplate() const { return m_ParticleTemplate; }

	private:
		// Particle properties
		struct Particle
		{
			glm::vec2 position{ 0.0f, 0.0f };

			float rotation = 0.0f;
			float rotationRate = 0.0f;

			glm::vec2 sizeBegin{ 1.0f, 1.0f }, sizeEnd{ 1.0f, 1.0f };
			glm::vec2 size{ 1.0f, 1.0f };

			glm::vec2 velocity{ 0.0f, 0.0f };

			glm::vec4 colorBegin{ 1.0f, 1.0f, 1.0f, 1.0f }, colorEnd{ 1.0f, 1.0f, 1.0f, 1.0f };
			glm::vec4 color{ 1.0f, 1.0f, 1.0f, 1.0f };

			float lifeTime = 1.0f;
			float lifeRemaining = 1.0f;

			glm::vec2 uvOffset{ 0.0f, 0.0f };

			bool bActive = false;
		};

	public:
		void EvaluateEmitterProperties();
		void EvaluateParticleProperties(Particle& particle);

		void OnUpdate(DeltaTime dt);
		void OnRender();
#if WITH_EDITOR
		void OnParticleViewImGuiRender();
#endif

		void OnDeserialized();

		void Activate();
		void Deactivate();

		void Resimulate();

	private:
		void Emit();

	public:
		/** Called when this particle system is about to be destroyed */
		std::function<void()> m_OnSystemFinished;

		static const char* ParticleSystemFileToken;

	private:
		std::vector<Particle> m_ParticlePool;
		uint32_t m_PoolIndex;

		uint32_t m_ActiveParticleCount = 0;

		std::string m_Path;
		std::string m_FileName;

		ParticleTemplate m_ParticleTemplate;

		/** Particle's origin in world space */
		glm::vec2 m_SpawnPosition{ 0.0f, 0.0f };
		/** Parent GameObject this particle system attaches to, particle's position is affected by parent's position */
		GameObject* m_Parent = nullptr;
		bool m_bAutoDestroy = true;

		// Emitter properties
		/** Time between emitting two particles, which equals to 1 / ParticleTemplate.spawnRate */
		float m_SpawnRate;
		struct BurstTimeData
		{
			BurstTimeData(float _time)
				: time(_time)
				, bProcessed(false)
			{}

			bool operator<(const BurstTimeData& other) const
			{
				return time < other.time;
			}

			float time;
			bool bProcessed;
		};
		std::map<BurstTimeData, uint32_t> m_BurstList;
		int32_t m_LoopCount;
		bool m_bInfiniteLoop;
		glm::vec2 m_TilingFactor{ 1.0f, 1.0f };

		/** This equals to lifetime / (subImageSize.x * subImageSize.y) */
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
		float m_FiniteLoopRestartInterval = 0.0f;
		float m_FiniteLoopRestartTime = 0.0f;
#endif

		RTTR_ENABLE()
		RTTR_REGISTRATION_FRIEND
	};

	// TODO: Consider singleton like Input class?
	class ParticleLibrary
	{
	public:
		~ParticleLibrary();

		void Add(const std::string& path, ParticleSystem* ps);
		void Add(ParticleSystem* ps);
		ParticleSystem* Load(const std::string& filePath);
		ParticleSystem* Load(const std::string& path, const std::string& filePath);

		ParticleSystem* GetOrLoad(const std::string& path);

		ParticleSystem* Get(const std::string& path);

		bool Exists(const std::string& path) const;

		const std::unordered_map<std::string, ParticleSystem*>& GetParticlesMap() const { return m_ParticleSystems; }

	private:
		/** The containing particle systems will not be used during gameplay, instead they are regarded as templates for instantiating */
		std::unordered_map<std::string, ParticleSystem*> m_ParticleSystems;

	};

	class ParticleManager
	{
	public:
		ParticleManager()
		{
			ZE_CORE_TRACE("Particle manager initialized!");
		}
		~ParticleManager();

		void OnUpdate(DeltaTime dt);
		void OnRender();

		void AddParticleSystem(ParticleSystem* particleSystem);

		void CleanUp();

	private:
		std::vector<ParticleSystem*> m_ParticleSystems;
	};

}
