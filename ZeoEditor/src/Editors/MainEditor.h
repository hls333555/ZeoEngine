#pragma once

#include "Editors/EditorBase.h"

namespace ZeoEngine {

	class SceneAsset;

	class MainEditor : public EditorBase
	{
	public:
		using EditorBase::EditorBase;

		virtual void OnAttach() override;

		virtual std::string GetAssetPath() const override;

	private:
		virtual void PostSceneCreate(bool bIsFromLoad) override;

		virtual AssetTypeId GetAssetTypeId() const override;
		virtual void LoadAssetImpl(const std::string& path) override;
		virtual void SaveAssetImpl(const std::string& path) override;

		void ClearSelectedEntity();

	private:
		AssetHandle<SceneAsset> m_SceneAsset;
	};

}
