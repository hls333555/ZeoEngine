#pragma once

#include "Engine/Core/Core.h"
#include "Engine/Core/Asset.h"
#include "Engine/Core/AssetLibrary.h"

namespace ZeoEngine {

	class Shader;
	class ShaderAsset;

	class Material
	{
	public:
		explicit Material(const std::string& path);

		const AssetHandle<ShaderAsset>& GetShaderAsset() const { return m_Shader; }
		void SetShaderAsset(const AssetHandle<ShaderAsset>& shader) { m_Shader = shader; }

		Ref<Shader> GetShader() const;

	private:
		AssetHandle<ShaderAsset> m_Shader;
	};

	class MaterialAsset : public AssetBase<MaterialAsset>
	{
	private:
		explicit MaterialAsset(const std::string& path);

	public:
		static Ref<MaterialAsset> Create(const std::string& path);

		const Ref<Material>& GetMaterial() const { return m_Material; }
		Ref<Shader> GetShader() const { return m_Material ? m_Material->GetShader() : Ref<Shader>{}; }

		virtual void Serialize(const std::string& path) override;
		virtual void Deserialize() override;

		virtual void Reload() override;

	private:
		Ref<Material> m_Material;
	};

	struct MaterialAssetLoader final : AssetLoader<MaterialAssetLoader, MaterialAsset>
	{
		AssetHandle<MaterialAsset> load(const std::string& path) const
		{
			return MaterialAsset::Create(path);
		}
	};

	class MaterialAssetLibrary : public AssetLibrary<MaterialAssetLibrary, MaterialAsset, MaterialAssetLoader>
	{
	public:
		static AssetHandle<MaterialAsset> GetDefaultMaterialAsset()
		{
			return MaterialAssetLibrary::Get().LoadAsset("assets/editor/materials/Default.zasset");
		}
	};

}
