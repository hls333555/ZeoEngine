#pragma once

#include <string>
#include <unordered_map>

#include "Engine/Core/Core.h"
#include "Engine/Renderer/Bindable.h"
#include "Engine/Core/Asset.h"
#include "Engine/Core/AssetLibrary.h"
#include "Engine/Core/EngineTypes.h"
#include "Engine/Renderer/Sampler.h"

namespace ZeoEngine {

	enum class TextureBinding
	{
		// TODO:
		// Reserved texture binding slots
		ShadowMap = 0,
		PcfShadowMap,
		BlueNoise,
		ScreenSpaceShadowMap,

		DiffuseTexture,
		SpecularTexture,

		Material = DiffuseTexture,
	};

	enum class TextureFormat
	{
		None = 0,

		// Color
		RGBA8,
		RGBA16F, // This format can store negative values
		RED_INTEGER,

		_DEPTH_START_,
		// Depth
		DEPTH32F,
		// Depth/stencil
		DEPTH24STENCIL8,
	};

	class Texture
	{
	public:
		virtual ~Texture() = default;

		virtual uint32_t GetWidth() const = 0;
		virtual uint32_t GetHeight() const = 0;
		virtual bool HasAlpha() const = 0;
		virtual void* GetTextureID() const = 0;
		virtual void* GetTextureViewID(uint32_t index) const = 0;

		/** Upload a block of memory with texture data to GPU. */
		virtual void SetData(void* data, uint32_t size) = 0;
		virtual void ChangeSampler(SamplerType type) = 0;

		// TODO: Let slot to be a member variable? And inherit from Bindable?
		virtual void Bind(uint32_t slot) const = 0;
		virtual void Unbind(uint32_t slot) const = 0;

		virtual bool operator==(const Texture& other) const = 0;
	};

	class Texture2D : public Texture
	{
	public:
		/** Used for constructing a texture from memory. */
		static Ref<Texture2D> Create(uint32_t width, uint32_t height, TextureFormat format = TextureFormat::RGBA8, SamplerType type = SamplerType::None);
		/** Used for loading a texture from disk. */
		static Ref<Texture2D> Create(const std::string& path, bool bAutoGenerateMipmaps = false);

		static Ref<Texture2D> GetDefaultTexture();
		static Ref<Texture2D> GetAssetBackgroundTexture();
	};

	class Texture2DArray : public Texture
	{
	public:
		/** Used for constructing a texture from memory. */
		static Ref<Texture2DArray> Create(uint32_t width, uint32_t height, uint32_t arraySize, TextureFormat format = TextureFormat::RGBA8, SamplerType type = SamplerType::None);
	};

	class Texture2DAsset : public AssetBase<Texture2DAsset>
	{
	private:
		explicit Texture2DAsset(const std::string& path);

	public:
		static Ref<Texture2DAsset> Create(const std::string& path);

		const Ref<Texture2D>& GetTexture() const { return m_Texture; }

		void Bind(uint32_t slot = 0) const { m_Texture->Bind(slot); }

		virtual void Serialize(const std::string& path) override;
		virtual void Deserialize() override;

		virtual void Reload(bool bIsCreate) override;

	private:
		Ref<Texture2D> m_Texture;
	};

	struct Texture2DAssetLoader final
	{
		using result_type = Ref<Texture2DAsset>;

		Ref<Texture2DAsset> operator()(const std::string& path) const
		{
			return Texture2DAsset::Create(path);
		}
	};

	class Texture2DAssetLibrary : public AssetLibrary<Texture2DAssetLibrary, Texture2DAsset, Texture2DAssetLoader>{};

}
