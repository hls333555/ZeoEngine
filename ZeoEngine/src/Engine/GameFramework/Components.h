#pragma once

#include <glm/glm.hpp>

#include "Engine/Renderer/Texture.h"
#include "Engine/GameFramework/SceneCamera.h"
#include "Engine/GameFramework/ScriptableEntity.h"

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

	struct NativeScriptCompont
	{
		ScriptableEntity* Instance = nullptr;

		std::function<void()> InstantiateFunc;
		std::function<void()> DestroyInstanceFunc;

		std::function<void(ScriptableEntity*)> OnCreateFunc;
		std::function<void(ScriptableEntity*)> OnDestroyFunc;
		std::function<void(ScriptableEntity*, DeltaTime)> OnUpdateFunc;

		template<typename T>
		void Bind()
		{
			InstantiateFunc = [&]() { Instance = new T(); };
			DestroyInstanceFunc = [&]() { delete static_cast<T*>(Instance); Instance = nullptr; };

			OnCreateFunc = [](ScriptableEntity* instance) { static_cast<T*>(instance)->OnCreate(); };
			OnDestroyFunc = [](ScriptableEntity* instance) { static_cast<T*>(instance)->OnDestroy(); };
			OnUpdateFunc = [](ScriptableEntity* instance, DeltaTime dt) { static_cast<T*>(instance)->OnUpdate(dt); };
		}
	};

}
