#pragma once

#include <glm/glm.hpp>

#include "Engine/Renderer/Texture.h"
#include "Engine/Core/DeltaTime.h"
#include "Engine/Core/Delegate.h"
#include "Engine/Utils/EngineUtils.h"

namespace ZeoEngine {

	class GameObject;

	enum class ParticleVariationType
	{
		Constant,
		RandomInRange,
		UniformInRange
	};

	template<typename T>
	struct ParticleVariation
	{
		~ParticleVariation();
		bool operator==(const ParticleVariation<T>& other) const;

		T Val1, Val2;
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

	struct ParticleTemplate
	{
	public:
		ParticleTemplate() = default;
		ParticleTemplate(const std::string& path)
			: Path(path)
			, Name(GetNameFromPath(path))
		{
		}

		const std::string& GetPath() const { return Path; }
		const std::string& GetName() const { return Name; }

		bool bIsLocalSpace = false;

		/** <= 0 means infinite loop */
		int32_t LoopCount = 0;
		float LoopDuration = 1.0f;

		/** Number of particles to spawn per second in total. < 0 means spawn at fps rate */
		ParticleFloat SpawnRate;
		/** List of burst of particles to spawn instantaneously per time. The time should be within (0.0, 1.0) */
		std::vector<BurstData> BurstList;

		// TODO: Support 3D
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
		glm::vec2 SubImageSize{ 0.0f };

		uint32_t MaxDrawParticles{ 500 };

		// TODO: PreviewThumbnail
		Ref<Texture2D> PreviewThumbnail;

	private:
		std::string Path;
		std::string Name;
	};

	class ParticleSystem
	{
		friend class ParticleLibrary;
		friend class ParticleManager;
		friend class Level;

		using SystemFinishedDel = Delegate<void()>;

	private:
		/** Construct a particle system from zparticle file. */
		ParticleSystem(const std::string& filePath, const std::string& processedSrc);
		ParticleSystem(const Ref<ParticleTemplate>& particleTemplate, const glm::vec2& position = glm::vec2(0.0f), bool bAutoDestroy = true);
		ParticleSystem(const Ref<ParticleTemplate>& particleTemplate, GameObject* attachToParent = nullptr, bool bAutoDestroy = true);

	public:
#if WITH_EDITOR
		//void PostPropertyValueEditChange(const rttr::property* prop, const rttr::property* outerProp);
#endif

		bool GetAutoDestroy() const { return m_bAutoDestroy; }
		void SetAutoDestroy(bool bValue) { m_bAutoDestroy = bValue; }

		void DeserializeProperties(const std::string& processedSrc);

		const Ref<ParticleTemplate>& GetParticleTemplate() const { return m_ParticleTemplate; }

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

		Ref<ParticleTemplate> m_ParticleTemplate;

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

	};

	class ParticleLibrary
	{
	public:
		static ParticleLibrary& Get()
		{
			static ParticleLibrary instance;
			return instance;
		}
	private:
		ParticleLibrary() = default;
	public:
		ParticleLibrary(const ParticleLibrary&) = delete;
		ParticleLibrary& operator=(const ParticleLibrary&) = delete;

		void Add(const std::string& path, const Ref<ParticleTemplate>& pTemplate);
		void Add(const Ref<ParticleTemplate>& pTemplate);
		Ref<ParticleTemplate> Load(const std::string& path);

		Ref<ParticleTemplate> GetOrLoad(const std::string& path);

		Ref<ParticleTemplate> Get(const std::string& path);

		bool Exists(const std::string& path) const;

		const std::unordered_map<std::string, Ref<ParticleTemplate>>& GetParticleTemplatesMap() const { return m_ParticleTemplates; }

	private:
		std::unordered_map<std::string, Ref<ParticleTemplate>> m_ParticleTemplates;

	};

}
