#pragma once

#include "Engine/Core/Layer.h"

namespace ZeoEngine {

	class Scene;

	class EngineLayer : public Layer
	{
	public:
		EngineLayer(const std::string& name = "Engine");

		const Ref<Scene>& GetActiveGameScene() const { return m_ActiveGameScene; }
		void SetActiveGameScene(const Ref<Scene>& scene) { m_ActiveGameScene = scene; }

		// TODO:
		virtual void OpenLevel(const std::filesystem::path& path) const {}

	private:
		Ref<Scene> m_ActiveGameScene;

	};

}
