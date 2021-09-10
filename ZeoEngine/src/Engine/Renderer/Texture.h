#pragma once

#include <string>
#include <unordered_map>

#include "Engine/Core/Core.h"
#include "Engine/Core/Asset.h"
#include "Engine/Core/AssetLibrary.h"
#include "Engine/Core/EngineTypes.h"

namespace ZeoEngine {

	class Texture
	{
	public:
		virtual ~Texture() = default;

		virtual uint32_t GetWidth() const = 0;
		virtual uint32_t GetHeight() const = 0;
		virtual bool HasAlpha() const = 0;
		virtual void* GetTextureID() const = 0;

		/** Upload a block of memory with texture data to GPU. */
		virtual void SetData(void* data, uint32_t size) = 0;

		virtual void Bind(uint32_t slot = 0) const = 0;

		virtual bool operator==(const Texture& other) const = 0;
	};

	class Texture2D : public Texture
	{
	public:
		/** Used for constructing a texture from memory. */
		static Ref<Texture2D> Create(uint32_t width, uint32_t height);
		/** Used for loading a texture from disk. */
		static Ref<Texture2D> Create(const std::string& path, bool bAutoGenerateMipmaps = false);

		static Ref<Texture2D> s_DefaultBackgroundTexture;
	};

	class Texture2DAsset : public AssetBase<Texture2DAsset>
	{
	private:
		explicit Texture2DAsset(const std::string& path);

	public:
		static Ref<Texture2DAsset> Create(const std::string& path);

		const Ref<Texture2D>& GetTexture() const { return m_Texture; }

		virtual void Serialize(const std::string& path) override;
		virtual void Deserialize() override;

		virtual void Reload() override;

	private:
		Ref<Texture2D> m_Texture;
	};

	struct Texture2DAssetLoader final : AssetLoader<Texture2DAssetLoader, Texture2DAsset>
	{
		AssetHandle<Texture2DAsset> load(const std::string& path) const
		{
			return Texture2DAsset::Create(path);
		}
	};

	class Texture2DAssetLibrary : public AssetLibrary<Texture2DAssetLibrary, Texture2DAsset, Texture2DAssetLoader>{};

}
