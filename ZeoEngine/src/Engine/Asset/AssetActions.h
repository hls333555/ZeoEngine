#pragma once

#include <string>

namespace ZeoEngine {

	/** Defines a series of actions available for an existing asset. */
	class IAssetActions
	{
	public:
		virtual void OpenAsset(const std::string& path, bool bIsFromAssetBrowser) const = 0;
		virtual void RenameAsset(const std::string& oldPath, const std::string& newPath) const = 0;
		virtual void DeleteAsset(const std::string& path) const = 0;
		virtual void ReimportAsset(const std::string& path) const {}
	};

	class AssetActionsBase : public IAssetActions
	{
	public:
		virtual void RenameAsset(const std::string& oldPath, const std::string& newPath) const override;
		virtual void DeleteAsset(const std::string& path) const override;
	};

	class ResourceAssetActionsBase : public AssetActionsBase
	{
	public:
		virtual void RenameAsset(const std::string& oldPath, const std::string& newPath) const override;
		virtual void DeleteAsset(const std::string& path) const override;
	};

	class ImportableAssetActionsBase : public ResourceAssetActionsBase
	{
	public:
		virtual void ReimportAsset(const std::string& path) const override;
	};

	class LevelAssetActions final : public AssetActionsBase
	{
	public:
		virtual void OpenAsset(const std::string& path, bool bIsFromAssetBrowser) const override;
	};

	class ParticleTemplateAssetActions : public AssetActionsBase
	{
	public:
		virtual void OpenAsset(const std::string& path, bool bIsFromAssetBrowser) const override;
	};

	class Texture2DAssetActions final : public ImportableAssetActionsBase
	{
	public:
		virtual void OpenAsset(const std::string& path, bool bIsFromAssetBrowser) const override;
	};

	class MeshAssetActions final : public ImportableAssetActionsBase
	{
	public:
		virtual void OpenAsset(const std::string& path, bool bIsFromAssetBrowser) const override;
	};

	class MaterialAssetActions final : public AssetActionsBase
	{
	public:
		virtual void OpenAsset(const std::string& path, bool bIsFromAssetBrowser) const override;
	};

	class ShaderAssetActions final : public ResourceAssetActionsBase
	{
	public:
		virtual void OpenAsset(const std::string& path, bool bIsFromAssetBrowser) const override;
	};

	class PhysicsMaterialAssetActions : public AssetActionsBase
	{
	public:
		virtual void OpenAsset(const std::string& path, bool bIsFromAssetBrowser) const override;
	};

}
