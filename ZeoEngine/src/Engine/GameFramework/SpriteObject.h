#pragma once

#include "Engine/GameFramework/GameObject.h"
#include "Engine/GameFramework/Level.h"
#include "Engine/Renderer/Texture.h"

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

		void SetTextureTiling(const glm::vec2& tilling) { m_TextureTilling = tilling; }
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

}
