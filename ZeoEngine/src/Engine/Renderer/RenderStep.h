#pragma once

namespace ZeoEngine {

	class Bindable;
	class Drawable;
	class RenderQueuePass;

	class RenderStep
	{
	public:
		explicit RenderStep(std::string renderQueuePassName);
		RenderStep(RenderStep&&) = default;
		RenderStep& operator=(const RenderStep&) = delete;
		RenderStep& operator=(RenderStep&&) = delete;

		const std::string& GetRenderQueuePassName() const { return m_RenderQueuePassName; }

		void AddBindable(const Ref<Bindable>& bindable);
		void LinkRenderQueuePass();
		void Bind() const;
		void Submit(const Drawable& drawable) const;

	private:
		std::string m_RenderQueuePassName;
		RenderQueuePass* m_RenderQueuePass = nullptr;
		std::vector<Ref<Bindable>> m_Bindables;
	};

}
