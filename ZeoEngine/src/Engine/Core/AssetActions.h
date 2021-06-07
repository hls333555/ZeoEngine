#pragma once

#include <string>

namespace ZeoEngine {

	class IAssetActions
	{
	public:
		virtual void OpenAsset(const std::string& path) const = 0;
		virtual void DeleteAsset(const std::string& path) const = 0;
	};

	class AssetActionsBase : public IAssetActions
	{
	public:
		virtual void DeleteAsset(const std::string& path) const override;
	};

	class SceneAssetActions final : public AssetActionsBase
	{
	public:
		virtual void OpenAsset(const std::string& path) const override;
	};

	class ParticleAssetActions : public AssetActionsBase
	{
	public:
		virtual void OpenAsset(const std::string& path) const override;
	};

	class Texture2DAssetActions final : public AssetActionsBase
	{
	public:
		virtual void OpenAsset(const std::string& path) const override {}
		virtual void DeleteAsset(const std::string& path) const override;
	};

}
