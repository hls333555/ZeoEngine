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
		RGB8,
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

	class Texture2D : public Texture, public AssetBase<Texture2D>
	{
	public:
		explicit Texture2D(std::string ID)
			: AssetBase(std::move(ID)) {}

		/** Construct a texture from memory. */
		static Ref<Texture2D> Create(std::string ID, uint32_t width, uint32_t height, TextureFormat format = TextureFormat::RGBA8, SamplerType type = SamplerType::None);
		/** Construct a 1x1 solid-color-texture from memory.  */
		static Ref<Texture2D> Create(std::string ID, uint32_t hexColor);
		/** Load a texture from disk. */
		static Ref<Texture2D> Create(const std::string& path, bool bAutoGenerateMipmaps = false);

		virtual void Serialize(const std::string& path) override;
		virtual void Deserialize() override;
	};

	REGISTER_ASSET(Texture2D,
	Ref<Texture2D> operator()(std::string ID, bool bIsReload, uint32_t hexColor) const
	{
		return Texture2D::Create(std::move(ID), hexColor);
	}
	Ref<Texture2D> operator()(const std::string& path, bool bIsReload) const
	{
		return Texture2D::Create(path);
	},
	static AssetHandle<Texture2D> GetWhiteTexture()
	{
		return Get().LoadAsset("ZID_WhiteTexture", 0xffffff);
	}
	static AssetHandle<Texture2D> GetDefaultMaterialTexture()
	{
		return Get().LoadAsset("ZID_DefaultMaterialTexture", 0x808080);
	}
	static AssetHandle<Texture2D> GetAssetBackgroundTexture()
	{
		return Get().LoadAsset("ZID_AssetBackgroundTexture", 0x151414);
	})

	class Texture2DArray : public Texture
	{
	public:
		/** Used for constructing a texture from memory. */
		static Ref<Texture2DArray> Create(uint32_t width, uint32_t height, uint32_t arraySize, TextureFormat format = TextureFormat::RGBA8, SamplerType type = SamplerType::None);
	};

}
