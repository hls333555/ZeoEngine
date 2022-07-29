#pragma once

#include "Engine/GameFramework/Scene.h"

namespace ZeoEngine {

	class LevelEditor;

	class LevelEditorScene : public Scene
	{
	public:
		explicit LevelEditorScene(const Ref<LevelEditor>& levelEditor);

		virtual void OnAttach() override;
		virtual void OnEvent(Event& e) override;

		void OnRuntimeStart() const;
		void OnRuntimeStop() const;

		virtual void PostLoad() override;

		virtual Entity GetSelectedEntity() const override;

	private:
		Ref<LevelEditor> m_LevelEditor;
	};

}
