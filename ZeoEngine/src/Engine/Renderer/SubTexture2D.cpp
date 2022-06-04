#include "ZEpch.h"
#include "Engine/Renderer/SubTexture2D.h"

namespace ZeoEngine {

	SubTexture2D::SubTexture2D(const Ref<Texture2D>& texture, const Vec2& min, const Vec2& max)
		: m_Texture(texture)
	{
		m_TexCoords[0] = { min.x, min.y };
		m_TexCoords[1] = { max.x, min.y };
		m_TexCoords[2] = { max.x, max.y };
		m_TexCoords[3] = { min.x, max.y };
	}

	Ref<SubTexture2D> SubTexture2D::CreateFromCoords(const Ref<Texture2D>& texture, const Vec2& coords, const Vec2& cellSize, const Vec2& spriteSize)
	{
		if (!texture)
			return Ref<SubTexture2D>();

		Vec2 min = { (coords.x * cellSize.x) / texture->GetWidth(), (coords.y * cellSize.y) / texture->GetHeight() };
		Vec2 max = { ((coords.x + spriteSize.x) * cellSize.x) / texture->GetWidth(), ((coords.y + spriteSize.y) * cellSize.y) / texture->GetHeight() };
		return CreateRef<SubTexture2D>(texture, min, max);
	}

}
