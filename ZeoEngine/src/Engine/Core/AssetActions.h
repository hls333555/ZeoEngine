#pragma once

#include <string>

namespace ZeoEngine {

	class IAssetActions
	{
	public:
		virtual void OpenAsset(const std::string& path) const = 0;
	};

	class SceneAssetActions final : public IAssetActions
	{
	public:
		virtual void OpenAsset(const std::string& path) const override;
	};

	class ParticleAssetActions : public IAssetActions
	{
	public:
		virtual void OpenAsset(const std::string& path) const override;
	};

	class Texture2DAssetActions final : public IAssetActions
	{
	public:
		virtual void OpenAsset(const std::string& path) const override {}
	};

}
