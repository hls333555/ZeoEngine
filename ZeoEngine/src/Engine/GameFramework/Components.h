#pragma once

#include <glm/glm.hpp>

#include "Engine/Renderer/Texture.h"
#include "Engine/GameFramework/SceneCamera.h"
#include "Engine/GameFramework/ScriptableEntity.h"

#define TEST 1

namespace ZeoEngine {

	struct TagComponent
	{
		std::string Tag;
		bool bIsInternal{ false };
#if TEST
		uint8_t ui8Var;
		uint32_t ui32Var;
		uint64_t ui64Var;
		int8_t i8Var = 25;
		int32_t i32Var;
		int64_t i64Var;
		float floatVar;
		double doubleVar;
		glm::vec2 vec2Var;
		glm::vec3 vec3Var;
		glm::vec4 colorVar;

#endif

		TagComponent() = default;
		TagComponent(const TagComponent&) = default;
		TagComponent(const std::string& tag)
			:Tag(tag) {}

	};

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
		// NOTE: Function pointer does not support lambda with captures
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
