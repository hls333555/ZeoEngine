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
		VarianceTexture,
		ScreenSpaceShadowMap,
		SceneTexture,

		DiffuseTexture,
		SpecularTexture,

		Material = DiffuseTexture,
	};

	enum class TextureFormat
	{
		None = 0,

		// Color
		R8,
		RGB8,
		SRGB8,
		RGBA8,
		SRGBA8,
		RGBA16F, // This format can store negative values
		RG32F,
		RED_INTEGER,

		_DEPTH_START_,
		// Depth
		DEPTH32F,
		// Depth/stencil
		DEPTH24STENCIL8,
	};

	class Texture : public Bindable
	{
	public:
		virtual U32 GetWidth() const = 0;
		virtual U32 GetHeight() const = 0;
		virtual TextureFormat GetFormat() const = 0;
		virtual bool HasAlpha() const = 0;
		virtual bool bIsSRGB() const = 0;
		virtual void SetSRGB(bool bSRGB) = 0;
		virtual SamplerType GetSamplerType() const = 0;
		virtual void ChangeSampler(SamplerType type) = 0;
		virtual bool ShouldGenerateMipmaps() const = 0;
		virtual void SetGenerateMipmaps(bool bGenerate) = 0;
		virtual U32 GetMipmapLevels() const = 0;

		virtual void* GetTextureID() const = 0;
		virtual void* GetTextureViewID(U32 index) const = 0;

		/** Upload a block of memory with texture data to GPU. */
		virtual void SetData(void* data, U32 size) = 0;

		virtual void SetBindingSlot(U32 slot) = 0;
		virtual void BindAsImage(U32 slot, bool bReadOrWrite) const = 0;

		virtual bool operator==(const Texture& other) const = 0;
	};

	class Texture2D : public Texture, public AssetBase<Texture2D>
	{
	public:
		explicit Texture2D(std::string ID)
			: AssetBase(std::move(ID)) {}

		/** Construct a texture from memory. */
		static Ref<Texture2D> Create(std::string ID, U32 width, U32 height, TextureFormat format = TextureFormat::RGBA8, std::optional<U32> bindingSlot = {}, SamplerType type = SamplerType::None);
		/** Construct a 1x1 solid-color-texture from memory.  */
		static Ref<Texture2D> Create(std::string ID, U32 hexColor, bool bIsSRGB = true, std::optional<U32> bindingSlot = {});
		/** Load a texture from disk. */
		static Ref<Texture2D> Create(const std::string& path, std::optional<U32> bindingSlot = {});

		virtual void Serialize(const std::string& path) override;
		virtual void Deserialize() override;
	};

	REGISTER_ASSET(Texture2D,
	Ref<Texture2D> operator()(std::string ID, U32 hexColor, bool bIsSRGB = true) const
	{
		return Texture2D::Create(std::move(ID), hexColor, bIsSRGB);
	}
	Ref<Texture2D> operator()(const std::string& path) const
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
	}
	static AssetHandle<Texture2D> GetCheckerboardTexture()
	{
		return Get().LoadAsset("assets/editor/textures/Checkerboard.png");
	})

	class Texture2DArray : public Texture
	{
	public:
		/** Used for constructing a texture from memory. */
		static Ref<Texture2DArray> Create(U32 width, U32 height, U32 arraySize, TextureFormat format = TextureFormat::RGBA8, std::optional<U32> bindingSlot = {}, SamplerType type = SamplerType::None);
	};

}
