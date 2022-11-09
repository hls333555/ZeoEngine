#pragma once

#include "Engine/Renderer/Texture.h"

#include <glm/glm.hpp>

namespace ZeoEngine {

	class SubTexture2D
	{
	public:
		SubTexture2D(const Ref<Texture2D>& texture, const Vec2& min, const Vec2& max);

		static Ref<SubTexture2D> CreateFromCoords(const Ref<Texture2D>& texture, const Vec2& coords, const Vec2& cellSize, const Vec2& spriteSize = { 1, 1 });

		const Ref<Texture2D>& GetTexture() const { return m_Texture; }
		const Vec2* GetTexCoords() const { return m_TexCoords; }

	private:
		Ref<Texture2D> m_Texture;
		Vec2 m_TexCoords[4];
	};

}
