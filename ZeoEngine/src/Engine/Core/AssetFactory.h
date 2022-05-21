#pragma once

#include <string>

#include "Engine/Core/EngineTypes.h"

namespace ZeoEngine {

	/** Asset factories contain functions used to construct an asset by creating or importing. */
	class IAssetFactory
	{
	public:
		virtual const char* GetAssetTypeName() const = 0;
		virtual const char* GetAssetExtension() const { return ""; }
		virtual bool ShouldShowInContextMenu() const = 0;
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
	};

	class SceneAssetFactory : public AssetFactoryBase
	{
	public:
		virtual const char* GetAssetTypeName() const override { return "Scene"; }
		virtual bool ShouldShowInContextMenu() const override { return true; }
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
	};

	class MaterialAssetFactory : public AssetFactoryBase
	{
	public:
		virtual const char* GetAssetTypeName() const override { return "Material"; }
		virtual bool ShouldShowInContextMenu() const override { return true; }
	};

	class ResourceAssetFactoryBase : public AssetFactoryBase
	{
	public:
		virtual const char* GetResourceTemplatePath() const = 0;
		virtual bool ShouldShowInContextMenu() const override { return true; }
		virtual void CreateAsset(const std::string& path) const override;
	};

	class ShaderAssetFactory : public ResourceAssetFactoryBase
	{
	public:
		virtual const char* GetAssetTypeName() const override { return "Shader"; }
		virtual const char* GetAssetExtension() const override { return ".glsl"; }
		virtual const char* GetResourceTemplatePath() const override { return "assets/editor/shaders/Template.glsl"; }
	};

}
