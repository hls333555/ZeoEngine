#pragma once

#include "Editors/EditorBase.h"

namespace ZeoEngine {

	class MainEditor : public EditorBase
	{
	public:
		using EditorBase::EditorBase;

		virtual void OnAttach() override;

	private:
		virtual AssetType GetAssetType() const override { return AssetType::Scene; }
		virtual void Serialize(const std::string& filePath) override;
		virtual void Deserialize(const std::string& filePath) override;

		void ClearSelectedEntity();
	};

}
