#pragma once

#include "Editors/EditorBase.h"

namespace ZeoEngine {

	class ParticleTemplateAsset;

	class ParticleEditor : public EditorBase
	{
	public:
		using EditorBase::EditorBase;

		virtual void OnAttach() override;

		virtual std::string GetAssetPath() const override;

	private:
		virtual AssetTypeId GetAssetTypeId() const override;
		virtual void LoadAssetImpl(const std::string& path) override;
		virtual void SaveAssetImpl(const std::string& path) override;

		void ReloadParticleTemplateData();
	};

}
