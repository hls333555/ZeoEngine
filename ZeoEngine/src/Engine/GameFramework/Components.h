#pragma once

#include <glm/glm.hpp>

#include "Engine/Renderer/Texture.h"
#include "Engine/GameFramework/SceneCamera.h"
#include "Engine/GameFramework/ScriptableEntity.h"

#define ENTT_CAT_IMPL(a, b) a##b
#define ENTT_CAT(a, b) ENTT_CAT_IMPL(a, b)
#define ENTT_REFL_REGISTRATION                                                      \
static void entt_auto_register_reflection_function_();                              \
namespace                                                                           \
{                                                                                   \
    struct entt__auto__register__                                                   \
    {                                                                               \
        entt__auto__register__()                                                    \
        {                                                                           \
            entt_auto_register_reflection_function_();                              \
        }                                                                           \
    };                                                                              \
}                                                                                   \
static const entt__auto__register__ ENTT_CAT(auto_register__, __LINE__);            \
static void entt_auto_register_reflection_function_()

namespace ZeoEngine {

	struct TransformComponent
	{
		glm::mat4 Transform{ 1.0f };

		TransformComponent() = default;
		TransformComponent(const TransformComponent&) = default;
		TransformComponent(const glm::mat4& transform)
			: Transform(transform) {}

		operator glm::mat4&() { return Transform; }
		operator const glm::mat4&() const { return Transform; }

	};

	struct TagComponent
	{
		std::string Tag;
		bool bIsInternal{ false };

		TagComponent() = default;
		TagComponent(const TagComponent&) = default;
		TagComponent(const std::string& tag)
			:Tag(tag) {}

	};

	struct SpriteRendererComponent
	{
		glm::vec4 TintColor{ 1.0f, 1.0f, 1.0f, 1.0f };
		Ref<Texture2D> Texture;
		glm::vec2 TextureTiling{ 1.0f };

		SpriteRendererComponent() = default;
		SpriteRendererComponent(const SpriteRendererComponent&) = default;
		SpriteRendererComponent(const glm::vec4& color)
			: TintColor(color) {}
		SpriteRendererComponent(const Ref<Texture2D>& texture, glm::vec4& tintColor = glm::vec4(1.0f), const glm::vec2& textureTiling = { 1.0f, 1.0f })
			: Texture(texture), TintColor(tintColor), TextureTiling(textureTiling) {}

	};

	struct CameraComponent
	{
		SceneCamera Camera;
		bool bIsPrimary = true;
		bool bFixedAspectRatio = false;

		CameraComponent() = default;
		CameraComponent(const CameraComponent&) = default;

	};

	struct NativeScriptComponent
	{
		// Function pointer does not support lambda with captures
		using InstantiateScriptDef = std::function<ScriptableEntity*()>;
		using DestroyScriptDef = void(*)(NativeScriptComponent*);

		ScriptableEntity* Instance = nullptr;

		InstantiateScriptDef InstantiateScript;
		DestroyScriptDef DestroyScript;

		template<typename T, typename ... Args>
		void Bind(Args&& ... args)
		{
			InstantiateScript = [=]() mutable { return static_cast<ScriptableEntity*>(new T(std::forward<Args>(args)...)); };
			DestroyScript = [](NativeScriptComponent* nsc) { delete nsc->Instance; nsc->Instance = nullptr; };
		}
	};

}
