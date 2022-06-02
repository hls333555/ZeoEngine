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

		bool HasSameMaterial(const RenderStep* step) const;
		void AddSubTask(SubRenderTask subTask);
		void Execute() const;

	private:
		const RenderStep* m_Step;
		std::vector<SubRenderTask> m_SubTasks;
	};

}
