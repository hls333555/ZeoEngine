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

	registration::class_<SubSpriteObject>("SubSpriteObject")
		(
			metadata(ClassMeta::Abstract, true)
		)
		.property("Coordinates", &SubSpriteObject::m_Coordinates)
		(
			policy::prop::bind_as_ptr,
			metadata(PropertyMeta::Category, "SubTexture"),
			metadata(PropertyMeta::Min, 0)
		)
		.property("CellSize", &SubSpriteObject::m_CellSize)
		(
			policy::prop::bind_as_ptr,
			metadata(PropertyMeta::Category, "SubTexture"),
			metadata(PropertyMeta::Min, 0)
		)
		.property("SpriteSize", &SubSpriteObject::m_SpriteSize)
		(
			policy::prop::bind_as_ptr,
			metadata(PropertyMeta::Category, "SubTexture"),
			metadata(PropertyMeta::Min, 0)
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
		if (m_SpriteTexture)
		{
			Renderer2D::DrawRotatedQuad(GetPosition(), GetScale(), GetRotation(true), m_SpriteTexture, m_TextureTilling, { 0.0f, 0.0f }, m_TintColor);
		}
		else
		{
			Renderer2D::DrawRotatedQuad(GetPosition(), GetScale(), GetRotation(true), m_TintColor);
		}
	}

	void SubSpriteObject::OnDeserialized()
	{
		Super::OnDeserialized();

		GenerateSubTexture();
	}

	void SubSpriteObject::OnRender()
	{
		if (m_SubSpriteTexture)
		{
			Renderer2D::DrawRotatedQuad(GetPosition(), GetScale(), GetRotation(true), m_SubSpriteTexture, GetTextureTiling(), { 0.0f, 0.0f }, GetTintColor());
		}
		else
		{
			Renderer2D::DrawRotatedQuad(GetPosition(), GetScale(), GetRotation(true), GetTintColor());
		}
	}

	void SubSpriteObject::GenerateSubTexture()
	{
		m_SubSpriteTexture = SubTexture2D::CreateFromCoords(m_SpriteTexture, m_Coordinates, m_CellSize, m_SpriteSize);
	}

#if WITH_EDITOR
	void SubSpriteObject::PostPropertyValueEditChange(const rttr::property* prop, const rttr::property* outerProp)
	{
		Super::PostPropertyValueEditChange(prop, outerProp);

		if (prop->get_name() == "SpriteTexture" ||
			prop->get_name() == "Coordinates" || prop->get_name() == "CellSize" || prop->get_name() == "SpriteSize")
		{
			GenerateSubTexture();
		}
	}
#endif

}
