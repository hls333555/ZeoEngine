#pragma once

#include <string>
#include <IconsFontAwesome5.h>

#include "Engine/Core/EngineTypes.h"

namespace ZeoEngine {

	class IAssetFactory
	{
	public:
		virtual const char* GetAssetTypeIcon() const = 0;
		virtual const char* GetAssetTypeName() const = 0;
		virtual bool ShouldShowInContextMenu() const = 0;
		virtual void CreateAsset(AssetTypeId typeId, const std::string& path) = 0;
	};

	class AssetFactoryBase : public IAssetFactory
	{
	public:
		virtual void CreateAsset(AssetTypeId typeId, const std::string& path) override;
	};

	class SceneAssetFactory : public AssetFactoryBase
	{
	public:
		virtual const char* GetAssetTypeIcon() const override { return ICON_FA_MOUNTAIN; }
		virtual const char* GetAssetTypeName() const override { return "Scene"; }
		virtual bool ShouldShowInContextMenu() const override { return true; }
	};

	class ParticleTemplateAssetFactory : public AssetFactoryBase
	{
	public:
		virtual const char* GetAssetTypeIcon() const override { return ICON_FA_FIRE_ALT; }
		virtual const char* GetAssetTypeName() const override { return "Particle Template"; }
		virtual bool ShouldShowInContextMenu() const override { return true; }
	};

	class Texture2DAssetFactory : public AssetFactoryBase
	{
	public:
		virtual const char* GetAssetTypeIcon() const override { return ICON_FA_IMAGE; }
		virtual const char* GetAssetTypeName() const override { return "Texture2D"; }
		virtual bool ShouldShowInContextMenu() const override { return false; }
	};

}
