#include "ZEpch.h"
#include "Engine/Renderer/Material.h"

#include "Engine/Renderer/Shader.h"
#include "Engine/Core/Serializer.h"

namespace ZeoEngine {

	Material::Material(const std::string& path)
	{
		m_Shader = ShaderAssetLibrary::GetDefaultShaderAsset();
	}

	Ref<Shader> Material::GetShader() const
	{
		return m_Shader ? m_Shader->GetShader() : Ref<Shader>{};
	}

	MaterialAsset::MaterialAsset(const std::string& path)
		: AssetBase(path)
	{
	}

	Ref<MaterialAsset> MaterialAsset::Create(const std::string& path)
	{
		class MaterialAssetEnableShared : public MaterialAsset
		{
		public:
			explicit MaterialAssetEnableShared(const std::string& path)
				: MaterialAsset(path) {}
		};

		auto asset = CreateRef<MaterialAssetEnableShared>(path);
		asset->Reload();
		return asset;
	}

	void MaterialAsset::Reload()
	{
		m_Material = CreateRef<Material>(GetPath());
		Deserialize();
	}

	void MaterialAsset::Serialize(const std::string& path)
	{
		if (path.empty()) return;

		if (path != GetPath())
		{
			SetPath(path);
		}
		AssetSerializer::Serialize(GetPath(), TypeId(), MaterialPreviewComponent{ SharedFromBase<MaterialAsset>() });
	}

	void MaterialAsset::Deserialize()
	{
		if (GetPath().empty()) return;

		AssetSerializer::Deserialize(GetPath(), TypeId(), MaterialPreviewComponent{ SharedFromBase<MaterialAsset>() });
	}

}
