#pragma once

#include "Engine/GameFramework/GameObject.h"
#include "Engine/GameFramework/Level.h"
#include "Engine/Renderer/Texture.h"
#include "Engine/Renderer/SubTexture2D.h"

namespace ZeoEngine {

	/**
	 * Base class for renderable objects.
	 * If texture is not assigned, it will draw a quad by default.
	 * If you want to load the texture in code, be sure to do it before SpriteObject::Init() being invoked!
	 */
	class SpriteObject : public GameObject
	{
	public:
		SpriteObject() = default;

		virtual void Init() override;
		virtual void OnRender() override;
		virtual bool IsTranslucent() const override { return m_bIsTranslucent; }

		const glm::vec2& GetTextureTiling() const { return m_TextureTilling; }
		void SetTextureTiling(const glm::vec2& tilling) { m_TextureTilling = tilling; }
		const glm::vec4& GetTintColor() const { return m_TintColor; }
		void SetTintColor(const glm::vec4& color) { m_TintColor = color; }

	protected:
		Ref<Texture2D> m_SpriteTexture;

	private:
		glm::vec2 m_TextureTilling{ 1.0f };
		glm::vec4 m_TintColor{ 1.0f };
		bool m_bIsTranslucent = false;

		RTTR_ENABLE(GameObject)
		RTTR_REGISTRATION_FRIEND
	};

	class SubSpriteObject : public SpriteObject
	{
	public:
		SubSpriteObject() = default;

		virtual void Init() override;
		virtual void OnRender() override;

#if WITH_EDITOR
		virtual void PostPropertyValueEditChange(const rttr::property* prop, const rttr::property* outerProp);
#endif

	private:
		void GenerateSubTexture();

	private:
		Ref<SubTexture2D> m_SubSpriteTexture;
		glm::i32vec2 m_Coordinates{ 0 };
		glm::i32vec2 m_CellSize{ 128 };
		glm::i32vec2 m_SpriteSize{ 1 };

		RTTR_ENABLE(SpriteObject)
		RTTR_REGISTRATION_FRIEND
	};

}
