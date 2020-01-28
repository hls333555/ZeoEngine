#pragma once

#include <glm/glm.hpp>

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
		ParticleVariationType variationType;
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
	};

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

	struct ParticleTemplate
	{
		void AddBurstData(float time, int32_t amount)
		{
			burstList.emplace(time, amount);
		}
		void AddBurstData(float time, int32_t amountLow, int32_t amountHigh)
		{
			burstList.emplace(time, ParticleInt{ amountLow, amountHigh });
		}

		bool bIsLocalSpace = false;

		/** <= 0 means infinite loop */
		int32_t loopCount = 0;
		float loopDuration = 1.0f;

		/** Number of particles to spawn per second in total. < 0 means spawn at fps rate */
		ParticleFloat spawnRate;
		/** List of burst of particles to spawn instantaneously per time. The time should be within (0.0, 1.0) */
		std::map<float, ParticleInt> burstList;

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
	};

	class ParticleSystem
	{
		friend class ParticleManager;

	public:
		ParticleSystem() = default;
		ParticleSystem(const ParticleTemplate& particleTemplate, GameObject* attachToParent = nullptr, bool bAutoDestroy = true);

		void OnUpdate(DeltaTime dt);
		void OnRender();

		void Activate();
		void Deactivate();

	private:
		void Emit();

	public:
		/** Called when this particle system is about to be destroyed */
		std::function<void()> OnSystemFinished;

	private:
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

		std::vector<Particle> m_ParticlePool;
		uint32_t m_PoolIndex = 999;

		ParticleTemplate m_ParticleTemplate;
		/** Time between emitting two particles, which equals to 1 / ParticleTemplate.spawnRate */
		float m_SpawnRate;
		std::map<BurstTimeData, uint32_t> m_BurstList;
		int32_t m_LoopCount;
		bool m_bInfiniteLoop;
		glm::vec2 m_TilingFactor{ 1.0f, 1.0f };
		/** This equals to lifetime / (subImageSize.x * subImageSize.y) */
		float m_UVAnimationInterval = 0.0f;
		float m_Time = 0.0f, m_LoopStartTime = 0.0f, m_SpawnTime = 0.0f, m_BurstTime = 0.0f, m_UVAnimationTime = 0.0f;

		GameObject* m_Parent;
		bool m_bAutoDestroy;
		/** Used to guarantee that initial emitted particles can be rendered properly */
		bool m_bStartUpdate = false;
		bool m_bActive = true;
		bool m_bPendingDestroy = false;

	};

	// TODO: Consider singleton like Input class?
	class ParticleManager
	{
	public:
		~ParticleManager();

		void OnUpdate(DeltaTime dt);
		void OnRender();

		void AddParticleSystem(ParticleSystem* particleSystem);

	private:
		std::vector<ParticleSystem*> m_ParticleSystems;
	};

}
