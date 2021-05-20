#pragma once

#include <string>

namespace ZeoEngine {

	class AssetActions
	{
	public:
		virtual void OpenAsset(const std::string& path) = 0;
	};

	class SceneAssetActions final : public AssetActions
	{
	public:
		virtual void OpenAsset(const std::string& path) override;
	};

	class Texture2DAssetActions final : public AssetActions
	{
	public:
		virtual void OpenAsset(const std::string& path) override {}
	};

	class ParticleAssetActions : public AssetActions
	{
	public:
		virtual void OpenAsset(const std::string& path) override;
	};

}
