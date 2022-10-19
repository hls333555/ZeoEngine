#pragma once

#include "Engine/Renderer/RenderStep.h"

namespace ZeoEngine {

	class Drawable;
	class Scene;

	class RenderTechnique
	{
	public:
		explicit RenderTechnique(std::string name, bool bStartActive = true);

		const std::string& GetName() const { return m_Name; }
		bool IsActive() const { return m_bActive; }

		const auto& GetRenderSteps() const { return m_Steps; }

		void AddStep(RenderStep step);

	private:
		std::string m_Name;
		bool m_bActive = true;
		std::vector<RenderStep> m_Steps;
	};

}
