#pragma once

#include <entt.hpp>
#define DEBUG_DRAW_EXPLICIT_CONTEXT
#include <debug_draw.hpp>

#include "Engine/Core/Asset.h"
#include "Engine/Core/Core.h"
#include "Engine/Core/DeltaTime.h"
#include "Engine/Events/Event.h"
#include "Engine/Core/AssetLibrary.h"
#include "Engine/GameFramework/Components.h"

namespace ZeoEngine {

	 class EditorCamera;
	 class SystemBase;

	/** A scene context which shares among copied scenes. */
	 struct SceneContext
	 {
		 dd::ContextHandle DebugDrawContext = nullptr;
	 };

	class Scene : public std::enable_shared_from_this<Scene>
	{
		friend class Entity;
		friend class SceneSerializer;
		friend class EditorViewPanelBase;
		friend class SceneOutlinePanel;
		friend class ISystem;

	public:
		Scene() = default;
		virtual ~Scene();

		virtual void OnAttach() {}
		void OnUpdate(DeltaTime dt) const;
		virtual void OnEvent(Event& e) {}

		/** Copy function which processes member copy. */
		virtual void Copy(const Ref<Scene>& other);

		const auto& GetSystems() const { return m_Systems; }
		const auto& GetContext() const { return m_Context;}

		// We cannot copy Scene through copy constructor as it is a deleted function
		// So we construct a new scene and call our "copy" function instead
		template<typename T, typename ... Args>
		Ref<T> Copy(Args&& ... args)
		{
			Ref<T> newScene = CreateRef<T>(std::forward<Args>(args)...);
			newScene->Copy(shared_from_this());
			return newScene;
		}

		Entity CreateEntity(const std::string& name = "Entity", const glm::vec3& translation = glm::vec3(0.0f));
		Entity CreateEntityWithUUID(UUID uuid, const std::string& name = "Entity", const glm::vec3& translation = glm::vec3(0.0f));
		Entity DuplicateEntity(Entity entity);
		void DestroyEntity(Entity entity);

		size_t GetEntityCount() const { return m_Registry.alive(); }

		/** Only implemented for the level editor. */
		virtual Entity GetSelectedEntity() const;

		/** Called after all data have been loaded. */
		virtual void PostLoad() {}

	protected:
		template<typename T, typename ... Args>
		void RegisterSystem(Args&& ... args)
		{
			static_assert(std::is_base_of_v<SystemBase, T>, "System type must be a SystemBase type!");

			Ref<T> system = CreateRef<T>(std::forward<Args>(args)...);
			system->OnCreate();
			m_Systems.emplace_back(std::move(system));
		}

	private:
		void SortEntities();

	public:
		entt::sink<entt::sigh<void(const Ref<Scene>&)>> m_OnSceneCopied{ m_OnSceneCopiedDel };
	protected:
		entt::registry m_Registry;
	private:
		/** Systems are shared among copied scenes. */
		std::vector<Ref<SystemBase>> m_Systems;
		Ref<SceneContext> m_Context = CreateRef<SceneContext>();

		uint32_t m_CurrentEntityIndex = 0;
		entt::sigh<void(const Ref<Scene>&)> m_OnSceneCopiedDel;
	};

	class Level : public AssetBase<Level>
	{
	public:
		explicit Level(std::string ID)
			: AssetBase(std::move(ID)) {}

		static Ref<Level> Create(std::string ID);

		static constexpr const char* GetTemplatePath() { return "assets/editor/levels/NewLevel.zasset"; }

		const Ref<Scene>& GetScene() const { return m_Scene; }
		/** Update scene reference and deserialize scene data. */
		void UpdateScene(const Ref<Scene>& scene) { m_Scene = scene; Deserialize(); }

		virtual void Serialize(const std::string& path) override;
		virtual void Deserialize() override;

	private:
		Ref<Scene> m_Scene;
	};

	REGISTER_ASSET(Level,
	Ref<Level> operator()(std::string ID, bool bIsReload) const
	{
		return Level::Create(std::move(ID));
	},
	static AssetHandle<Level> GetDefaultEmptyLevel()
	{
		return Get().LoadAsset(Level::GetTemplatePath());
	})

}
