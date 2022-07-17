#pragma once

#include <string>

#include "Engine/Core/EngineTypes.h"

namespace ZeoEngine {

	/** Asset factories contain functions used to construct an asset by creating or importing. */
	class IAssetFactory
	{
	public:
		virtual const char* GetAssetTypeName() const = 0;
		virtual const char* GetResourceExtension() const { return ""; }
		virtual bool ShouldShowInContextMenu() const = 0;
		virtual const char* GetAssetTemplatePath() const { return ""; }
		virtual void CreateAsset(const std::string& path) const = 0;
		virtual void ImportAsset(const std::string& srcPath, const std::string& destPath) const {}
	};

	class AssetFactoryBase : public IAssetFactory
	{
		friend class AssetManager;

	public:
		virtual void CreateAsset(const std::string& path) const override;

	protected:
		AssetTypeId m_TypeId;
	};

	class ImportableAssetFactoryBase : public AssetFactoryBase
	{
	public:
		virtual bool ShouldShowInContextMenu() const override { return false; }
		virtual void ImportAsset(const std::string& srcPath, const std::string& destPath) const override;

	private:
		void CreateAsset(const std::string& path, const std::string& resourcePath) const;
	};

	class LevelAssetFactory : public AssetFactoryBase
	{
	public:
		virtual const char* GetAssetTypeName() const override { return "Level"; }
		virtual bool ShouldShowInContextMenu() const override { return true; }
		virtual const char* GetAssetTemplatePath() const override;
	};

	class ParticleTemplateAssetFactory : public AssetFactoryBase
	{
	public:
		virtual const char* GetAssetTypeName() const override { return "Particle Template"; }
		virtual bool ShouldShowInContextMenu() const override { return true; }
	};

	class Texture2DAssetFactory : public ImportableAssetFactoryBase
	{
	public:
		virtual const char* GetAssetTypeName() const override { return "Texture2D"; }
	};

	class MeshAssetFactory : public ImportableAssetFactoryBase
	{
	public:
		virtual const char* GetAssetTypeName() const override { return "Mesh"; }
		virtual void ImportAsset(const std::string& srcPath, const std::string& destPath) const override;
	};

	class MaterialAssetFactory : public AssetFactoryBase
	{
	public:
		virtual const char* GetAssetTypeName() const override { return "Material"; }
		virtual bool ShouldShowInContextMenu() const override { return true; }
		virtual const char* GetAssetTemplatePath() const override;
	};

	class ResourceAssetFactoryBase : public AssetFactoryBase
	{
	public:
		virtual bool ShouldShowInContextMenu() const override { return true; }
		virtual const char* GetResourceTemplatePath() const = 0;
		virtual void CreateAsset(const std::string& path) const override;
	};

	class ShaderAssetFactory : public ResourceAssetFactoryBase
	{
	public:
		virtual const char* GetAssetTypeName() const override { return "Shader"; }
		virtual const char* GetResourceExtension() const override { return ".glsl"; }
		virtual const char* GetAssetTemplatePath() const override;
		virtual const char* GetResourceTemplatePath() const override;
	};

}
