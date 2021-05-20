#pragma once

#include <string>
#include <unordered_map>

#include "Engine/Core/Core.h"
#include "Engine/Core/AssetLibrary.h"
#include "Engine/Core/EngineTypes.h"

namespace ZeoEngine {

	class Texture
	{
	public:
		virtual ~Texture() = default;

		virtual uint32_t GetWidth() const = 0;
		virtual uint32_t GetHeight() const = 0;
		virtual const std::string& GetPath() const = 0; // This path is canonical.
		virtual const std::string& GetFileName() const = 0;
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
		static Ref<Texture2D> Create(const std::string& path);

	};

	struct Texture2DLoader final : AssetLoader<Texture2DLoader, Texture2D>
	{
		Asset<Texture2D> load(const std::string& path) const
		{
			return Texture2D::Create(path);
		}
	};

	class Texture2DLibrary : public AssetLibrary<Texture2DLibrary, Texture2D, Texture2DLoader>{};

}
