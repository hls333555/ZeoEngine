#include "Engine/GameFramework/Components.h"

ENTT_REFL_REGISTRATION
{
	using namespace ZeoEngine;
	entt::meta<TransformComponent>().type("TransformComponent"_hs)
		.data<&TransformComponent::Transform, entt::as_ref_t>("Transform"_hs);

	entt::meta<TagComponent>().type("TagComponent"_hs)
		.data<&TagComponent::Tag, entt::as_ref_t>("Tag"_hs);

	entt::meta<SpriteRendererComponent>().type("SpriteRendererComponent"_hs)
		.data<&SpriteRendererComponent::TintColor, entt::as_ref_t>("TintColor"_hs)
		.data<&SpriteRendererComponent::Texture, entt::as_ref_t>("Texture"_hs)
		.data<&SpriteRendererComponent::TextureTiling, entt::as_ref_t>("TextureTiling"_hs);

	entt::meta<CameraComponent>().type("CameraComponent"_hs)
		.data<&CameraComponent::bIsPrimary>("IsPrimary"_hs)
		.data<&CameraComponent::bFixedAspectRatio>("FixedAspectRatio"_hs);
}
