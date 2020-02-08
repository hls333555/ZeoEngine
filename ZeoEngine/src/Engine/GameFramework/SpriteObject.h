#pragma once

#include "Engine/GameFramework/GameObject.h"
#include "Engine/GameFramework/Level.h"
#include "Engine/Renderer/Texture.h"

namespace ZeoEngine {

	/**
	 * Base class for renderable objects.
	 * If texture is not assigned, it will draw a quad by default.
	 * Be sure to load the texture before SpriteObject::Init() being invoked!
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
		glm::vec2 m_TextureTilling = glm::vec2(1.0f);
		glm::vec4 m_TintColor = glm::vec4(1.0f);
		bool m_bIsTranslucent = false;

		RTTR_REGISTER(SpriteObject, GameObject)
		RTTR_REGISTRATION_FRIEND
	};

}
