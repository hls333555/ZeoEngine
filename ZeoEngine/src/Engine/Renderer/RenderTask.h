#pragma once

namespace ZeoEngine {

	class Drawable;
	class RenderStep;

	class RenderTask
	{
	public:
		RenderTask(const Drawable* drawable, const RenderStep* step);
		void Execute() const;

	private:
		const Drawable* m_Drawable;
		const RenderStep* m_Step;
	};

}
