#pragma once

#include "Engine/Asset/Asset.h"

namespace ZeoEngine {

	struct AssetMetadata;

	/** Asset factories contain functions used to construct an asset by creating or importing. */
	class IAssetFactory
	{
	public:
		virtual const char* GetAssetTypeName() const = 0;
		virtual const char* GetResourceExtension() const { return ""; }
		virtual bool ShouldShowInContextMenu() const = 0;
		virtual const char* GetAssetTemplatePath() const { return ""; }
		virtual void CreateAssetFile(const std::filesystem::path& path) const = 0;
		virtual void ImportAsset(const std::filesystem::path& srcPath, const std::filesystem::path& destPath) const {}
		virtual Ref<IAsset> CreateAsset(const Ref<AssetMetadata>& metadata) const = 0;
	};

	class AssetFactoryBase : public IAssetFactory
	{
		friend class AssetManager;

	public:
		virtual void CreateAssetFile(const std::filesystem::path& path) const override;

	protected:
		AssetTypeID m_TypeID;
	};

	class ImportableAssetFactoryBase : public AssetFactoryBase
	{
	public:
		virtual bool ShouldShowInContextMenu() const override { return false; }
		virtual void ImportAsset(const std::filesystem::path& srcPath, const std::filesystem::path& destPath) const override;

	private:
		void CreateAsset(const std::filesystem::path& path, const std::filesystem::path& resourcePath) const;
	};

	class LevelAssetFactory : public AssetFactoryBase
	{
	public:
		virtual const char* GetAssetTypeName() const override { return "Level"; }
		virtual bool ShouldShowInContextMenu() const override { return true; }
		virtual const char* GetAssetTemplatePath() const override;
		virtual Ref<IAsset> CreateAsset(const Ref<AssetMetadata>& metadata) const override;
	};

	class ParticleTemplateAssetFactory : public AssetFactoryBase
	{
	public:
		virtual const char* GetAssetTypeName() const override { return "Particle Template"; }
		virtual bool ShouldShowInContextMenu() const override { return true; }
		virtual Ref<IAsset> CreateAsset(const Ref<AssetMetadata>& metadata) const override;
	};

	class Texture2DAssetFactory : public ImportableAssetFactoryBase
	{
	public:
		virtual const char* GetAssetTypeName() const override { return "Texture2D"; }
		virtual Ref<IAsset> CreateAsset(const Ref<AssetMetadata>& metadata) const override;
	};

	class MeshAssetFactory : public ImportableAssetFactoryBase
	{
	public:
		virtual const char* GetAssetTypeName() const override { return "Mesh"; }
		virtual void ImportAsset(const std::filesystem::path& srcPath, const std::filesystem::path& destPath) const override;
		virtual Ref<IAsset> CreateAsset(const Ref<AssetMetadata>& metadata) const override;
	};

	class MaterialAssetFactory : public AssetFactoryBase
	{
	public:
		virtual const char* GetAssetTypeName() const override { return "Material"; }
		virtual bool ShouldShowInContextMenu() const override { return true; }
		virtual const char* GetAssetTemplatePath() const override;
		virtual Ref<IAsset> CreateAsset(const Ref<AssetMetadata>& metadata) const override;
	};

	class ResourceAssetFactoryBase : public AssetFactoryBase
	{
	public:
		virtual bool ShouldShowInContextMenu() const override { return true; }
		virtual const char* GetResourceTemplatePath() const = 0;
		virtual void CreateAssetFile(const std::filesystem::path& path) const override;
	};

	class ShaderAssetFactory : public ResourceAssetFactoryBase
	{
	public:
		virtual const char* GetAssetTypeName() const override { return "Shader"; }
		virtual const char* GetResourceExtension() const override { return ".glsl"; }
		virtual const char* GetAssetTemplatePath() const override;
		virtual const char* GetResourceTemplatePath() const override;
		virtual Ref<IAsset> CreateAsset(const Ref<AssetMetadata>& metadata) const override;
	};

}
