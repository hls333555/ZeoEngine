#pragma once

#include "Engine/Core/DeltaTime.h"
#include "Engine/GameFramework/Scene.h"

namespace ZeoEngine {

	template<typename... Component>
	inline constexpr entt::exclude_t<Component...> ExcludeComponents{};

	class ISystem
	{
	public:
		explicit ISystem(Scene* scene);

		virtual void OnCreate() {}
		virtual void OnUpdate(DeltaTime dt) = 0;
		virtual void OnDestroy() {}

	protected:
		template<typename... Component, typename... Exclude, typename Func>
		void ForEachView(Func&& func, entt::exclude_t<Exclude...> exclude = {})
		{
			m_Scene->m_Registry.view<Component...>(exclude).each(std::forward<Func>(func));
		}

	protected:
		Scene* m_Scene = nullptr;
	};

	class RenderSystem : public ISystem
	{
	public:
		using ISystem::ISystem;
		
		virtual void OnUpdate(DeltaTime dt) override;
		void OnRenderEditor(const EditorCamera& camera);
		void OnRenderRuntime();
		virtual void OnDestroy() override;

	private:
		void OnRender();
	};

	class NativeScriptSystem : public ISystem
	{
	public:
		using ISystem::ISystem;

		virtual void OnUpdate(DeltaTime dt) override;
		void OnEvent(Event& e);
	};

}
