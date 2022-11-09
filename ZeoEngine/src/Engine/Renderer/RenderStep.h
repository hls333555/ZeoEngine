#pragma once

namespace ZeoEngine {

	class Bindable;
	class Drawable;
	class RenderQueuePass;
	struct SceneContext;
	class Material;

	class RenderStep
	{
		friend class RenderStepInstance;

	public:
		explicit RenderStep(std::string renderQueuePassName);
		RenderStep(const RenderStep&) = delete;
		RenderStep(RenderStep&&) = default;

		RenderStep& operator=(const RenderStep&) = delete;
		RenderStep& operator=(RenderStep&&) = default;

		void AddBindable(const Ref<Bindable>& bindable);
		void Bind() const;
		void Unbind() const;

	private:
		std::string m_RenderQueuePassName;
		std::vector<Ref<Bindable>> m_Bindables;
	};

	class RenderStepInstance
	{
	public:
		RenderStepInstance(const RenderStep* step, const SceneContext* sceneContext);
		void Submit(const Drawable& drawable) const;

	private:
		const RenderStep* m_Step;
		RenderQueuePass* m_RenderQueuePass = nullptr;
	};

}
