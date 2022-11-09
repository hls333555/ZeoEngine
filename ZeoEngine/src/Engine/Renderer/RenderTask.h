#pragma once

namespace ZeoEngine {

	class Drawable;
	class RenderStep;

	class SubRenderTask
	{
	public:
		explicit SubRenderTask(const Drawable* drawable);

		void Execute() const;

	private:
		const Drawable* m_Drawable;
	};

	class RenderTask
	{
	public:
		explicit RenderTask(const RenderStep* step);

		const RenderStep* GetRenderStep() const { return m_Step; }

		void AddSubTask(SubRenderTask subTask);
		void Execute() const;

	private:
		const RenderStep* m_Step;
		std::vector<SubRenderTask> m_SubTasks;
	};

}
