#include "ZEpch.h"
#include "Engine/GameFramework/SpriteObject.h"

#include "Engine/Renderer/Renderer2D.h"

RTTR_REGISTRATION
{
	using namespace rttr;
	using namespace ZeoEngine;
	registration::class_<SpriteObject>("SpriteObject")
		(
			metadata(ClassMeta::Abstract, true)
		)
		.constructor(&SpriteObject::SpawnGameObject, policy::ctor::as_raw_ptr)
		.property("SpriteTexture", &SpriteObject::m_SpriteTexture)
		(
			metadata(PropertyMeta::Category, "Texture")
		)
		.property("TextureTilling", &SpriteObject::m_TextureTilling)
		(
			policy::prop::bind_as_ptr,
			metadata(PropertyMeta::Category, "Texture"),
			metadata(PropertyMeta::Min, 0.0f)
		)
		.property("TintColor", &SpriteObject::m_TintColor)
		(
			policy::prop::bind_as_ptr,
			metadata(PropertyMeta::Category, "Texture")
		);
}

namespace ZeoEngine {

	void SpriteObject::Init()
	{
		Super::Init();

		m_bIsTranslucent = m_SpriteTexture ? m_SpriteTexture->HasAlpha() : false;
	}

	void SpriteObject::OnRender()
	{
		Super::OnRender();

		if (m_SpriteTexture)
		{
			Renderer2D::DrawRotatedQuad(GetPosition(), GetScale(), GetRotation(true), m_SpriteTexture, m_TextureTilling, { 0.0f, 0.0f }, m_TintColor);
		}
		else
		{
			Renderer2D::DrawRotatedQuad(GetPosition(), GetScale(), GetRotation(true), m_TintColor);
		}
	}

}
