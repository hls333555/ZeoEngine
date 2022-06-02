#pragma once

#include "Engine/Core/EngineTypes.h"

namespace ZeoEngine {

	class Bindable;
	class Drawable;
	class RenderQueuePass;
	class Scene;
	class Material;

	class RenderStep
	{
	public:
		explicit RenderStep(std::string renderQueuePassName);
		RenderStep(RenderStep&&) = default;
		RenderStep(const RenderStep&) = default;
		RenderStep& operator=(const RenderStep&) = delete;
		RenderStep& operator=(RenderStep&&) = delete;

		const std::string& GetRenderQueuePassName() const { return m_RenderQueuePassName; }
		const auto& GetMaterialRef() const { return m_MaterialRef; }

		void AddBindable(const Ref<Bindable>& bindable);
		void SetContext(const Weak<Scene>& sceneContext, const AssetHandle<Material>& material);
		void Bind() const;
		void Unbind() const;
		void Submit(const Drawable& drawable);

	private:
		void LinkRenderQueuePass();

	private:
		std::string m_RenderQueuePassName;
		Weak<Scene> m_SceneContext;
		RenderQueuePass* m_RenderQueuePass = nullptr;
		std::vector<Ref<Bindable>> m_Bindables;
		AssetHandle<Material> m_MaterialRef;
	};

}
