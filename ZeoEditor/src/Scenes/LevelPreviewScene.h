#pragma once

#include "Engine/GameFramework/Scene.h"

namespace ZeoEngine {

	class LevelPreviewScene : public Scene
	{
	public:
		~LevelPreviewScene();

		virtual void OnAttach(const Ref<WorldBase>& world) override;
		virtual void OnEvent(Event& e) override;

		void OnRuntimeStart() const;
		void OnRuntimeStop() const;

		virtual void PostLoad() override;

	};

}
