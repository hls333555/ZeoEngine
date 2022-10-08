#pragma once

#include <optional>

#include "Engine/Core/Core.h"
#include "Engine/Renderer/Bindable.h"
#include "Engine/Asset/Asset.h"
#include "Engine/Core/EngineTypes.h"
#include "Engine/Renderer/Sampler.h"

namespace ZeoEngine {

	namespace TextureBinding
	{
		static constexpr U32 ShadowMap = 0;
		static constexpr U32 ShadowMapPcf = 1;

		static constexpr U32 ScreenSpaceShadowMap = 0;
		static constexpr U32 DiffuseTexture = 1;
		static constexpr U32 SpecularTexture = 2;

		static constexpr U32 SceneTexture = 0;

	}

	enum class TextureFormat
	{
		None = 0,

		// Color
		R8,
		RGB8,
		SRGB8,
		RGBA8,
		SRGBA8,
		RGBA16F, // This format can store negative values, [-65504, 65504]
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
		friend struct TexturePreviewComponent;

	public:
		bool IsSRGB() const { return m_bIsSRGB; }
		void SetSRGB(bool bSRGB) { m_bIsSRGB = bSRGB; Invalidate(); }
		const Ref<Sampler>& GetSampler() const { return m_Sampler; }
		void ChangeSampler(SamplerType type) { m_Sampler = SamplerLibrary::GetOrAddSampler(type); }
		bool ShouldGenerateMipmaps() const { return m_bShouldGenerateMipmaps; }
		void SetGenerateMipmaps(bool bGenerate) { m_bShouldGenerateMipmaps = bGenerate; Invalidate(); }

		virtual U32 GetWidth() const = 0;
		virtual U32 GetHeight() const = 0;
		virtual TextureFormat GetFormat() const = 0;
		virtual bool HasAlpha() const = 0;
		virtual U32 GetMipmapLevels() const = 0;

		virtual void* GetTextureID() const = 0;
		virtual void* GetTextureViewID(U32 index) const = 0;

		virtual void Invalidate() = 0;

		/** Upload a block of memory with texture data to GPU. */
		virtual void SetData(void* data, U32 size) = 0;

		virtual void SetBindingSlot(U32 slot) = 0;
		virtual void BindAsImage(U32 slot, bool bReadOrWrite) const = 0;

	private:
		bool m_bIsSRGB = true;
		Ref<Sampler> m_Sampler;
		bool m_bShouldGenerateMipmaps = true;
	};

	class Texture2D : public Texture, public AssetBase<Texture2D>
	{
	public:
		/** Construct a texture from memory. */
		static Ref<Texture2D> Create(U32 width, U32 height, TextureFormat format = TextureFormat::RGBA8, std::optional<U32> bindingSlot = {}, SamplerType type = SamplerType::None);
		/** Construct a 1x1 solid-color-texture from memory.  */
		static Ref<Texture2D> Create(U32 hexColor, bool bIsSRGB = true, std::optional<U32> bindingSlot = {});
		/** Load a texture from disk. */
		static Ref<Texture2D> Create(std::string resourcePath, std::optional<U32> bindingSlot = {});

		static Ref<Texture2D> GetWhiteTexture();
		static Ref<Texture2D> GetDefaultMaterialTexture();
		static Ref<Texture2D> GetAssetBackgroundTexture();
		static Ref<Texture2D> GetCheckerboardTexture();
	};

	class Texture2DArray : public Texture
	{
	public:
		/** Used for constructing a texture from memory. */
		static Ref<Texture2DArray> Create(U32 width, U32 height, U32 arraySize, TextureFormat format = TextureFormat::RGBA8, std::optional<U32> bindingSlot = {}, SamplerType type = SamplerType::None);
	};

}
