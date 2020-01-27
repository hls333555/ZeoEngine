#pragma once

#include "Engine/Core/Core.h"

namespace ZeoEngine {

	class Texture
	{
	public:
		virtual ~Texture() = default;

		virtual uint32_t GetWidth() const = 0;
		virtual uint32_t GetHeight() const = 0;
		virtual const std::string& GetPath() const = 0;
		virtual bool HasAlpha() const = 0;

		/** Upload a block of memory with texture data to GPU. */
		virtual void SetData(void* data, uint32_t size) = 0;

		virtual void Bind(uint32_t slot = 0) const = 0;
	};

	class Texture2D : public Texture
	{
	public:
		/** Used for constructing a texture from memory. */
		static Ref<Texture2D> Create(uint32_t width, uint32_t height);
		/** Used for loading a texture from disk. */
		static Ref<Texture2D> Create(const std::string& path);

	};

	class Texture2DLibrary
	{
	public:
		void Add(const std::string& path, const Ref<Texture2D>& texture);
		void Add(const Ref<Texture2D>& texture);
		Ref<Texture2D> Load(const std::string& filePath);
		Ref<Texture2D> Load(const std::string& path, const std::string& filePath);

		Ref<Texture2D> Get(const std::string& path);

		bool Exists(const std::string& path) const;

	private:
		std::unordered_map<std::string, Ref<Texture2D>> m_Textures;

	};

}
