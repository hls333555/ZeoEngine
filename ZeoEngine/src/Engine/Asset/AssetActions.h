#pragma once

#include <filesystem>

namespace ZeoEngine {

	/** Defines a series of actions available for an existing asset. */
	class IAssetActions
	{
	public:
		virtual void OpenAsset(const std::filesystem::path& path) const = 0;
		virtual void RenameAsset(const std::filesystem::path& oldPath, const std::filesystem::path& newPath) const = 0;
		virtual void DeleteAsset(const std::filesystem::path& path) const = 0;
		virtual void ReimportAsset(const std::filesystem::path& path) const {}
	};

	class AssetActionsBase : public IAssetActions
	{
	public:
		virtual void RenameAsset(const std::filesystem::path& oldPath, const std::filesystem::path& newPath) const override;
		virtual void DeleteAsset(const std::filesystem::path& path) const override;
	};

	class ResourceAssetActionsBase : public AssetActionsBase
	{
	public:
		virtual void RenameAsset(const std::filesystem::path& oldPath, const std::filesystem::path& newPath) const override;
		virtual void DeleteAsset(const std::filesystem::path& path) const override;
	};

	class ImportableAssetActionsBase : public ResourceAssetActionsBase
	{
	public:
		virtual void ReimportAsset(const std::filesystem::path& path) const override;
	};

	class LevelAssetActions final : public AssetActionsBase
	{
	public:
		virtual void OpenAsset(const std::filesystem::path& path) const override;
	};

	class ParticleTemplateAssetActions : public AssetActionsBase
	{
	public:
		virtual void OpenAsset(const std::filesystem::path& path) const override;
	};

	class Texture2DAssetActions final : public ImportableAssetActionsBase
	{
	public:
		virtual void OpenAsset(const std::filesystem::path& path) const override;
	};

	class MeshAssetActions final : public ImportableAssetActionsBase
	{
	public:
		virtual void OpenAsset(const std::filesystem::path& path) const override;
	};

	class MaterialAssetActions final : public AssetActionsBase
	{
	public:
		virtual void OpenAsset(const std::filesystem::path& path) const override;
	};

	class ShaderAssetActions final : public ResourceAssetActionsBase
	{
	public:
		virtual void OpenAsset(const std::filesystem::path& path) const override;
	};

}
