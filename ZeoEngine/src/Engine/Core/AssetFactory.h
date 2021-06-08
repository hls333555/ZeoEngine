#pragma once

#include <string>
#include <IconsFontAwesome5.h>

#include "Engine/Core/EngineTypes.h"

namespace ZeoEngine {

	class IAssetFactory
	{
	public:
		virtual const char* GetAssetTypeName() const = 0;
		virtual bool ShouldShowInContextMenu() const = 0;
		virtual void CreateAsset(const std::string& path) = 0;
		virtual void ImportAsset(const std::string& srcPath, const std::string& destPath) = 0;
	};

	class AssetFactoryBase : public IAssetFactory
	{
		friend class AssetManager;

	public:
		virtual void CreateAsset(const std::string& path) override;
		virtual void ImportAsset(const std::string& srcPath, const std::string& destPath) override;

	protected:
		AssetTypeId m_TypeId;
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

	class Texture2DAssetFactory : public AssetFactoryBase
	{
	public:
		virtual const char* GetAssetTypeName() const override { return "Texture2D"; }
		virtual bool ShouldShowInContextMenu() const override { return false; }
	};

}
