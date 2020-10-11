#pragma once

#include <glm/glm.hpp>

#include "Engine/Renderer/Texture.h"
#include "Engine/GameFramework/SceneCamera.h"
#include "Engine/GameFramework/ScriptableEntity.h"
#include "Engine/GameFramework/ParticleSystem.h"

#define ENABLE_TEST 1

namespace ZeoEngine {

#if ENABLE_TEST
	struct TestComponent
	{
		enum TestEnum
		{
			TestEnum1, TestEnum2, TestEnum3
		};

		enum class TestEnumClass
		{
			TestEnumClass1, TestEnumClass2, TestEnumClass3
		};

		bool BoolVar;
		uint8_t Ui8Var;
		uint32_t Ui32Var;
		uint64_t Ui64Var;
		int8_t I8Var = 25;
		int32_t I32Var;
		int64_t I64Var;
		float FloatVar;
		double DoubleVar;
		TestEnum enumVar;
		TestEnumClass enumClassVar;
		std::string StringVar;
		glm::vec2 Vec2Var;
		glm::vec3 Vec3Var;
		glm::vec4 ColorVar;

		TestComponent() = default;
		TestComponent(const TestComponent&) = default;
	};
#endif

	struct TagComponent
	{
		std::string Tag;
		bool bIsInternal{ false };

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

		SceneCamera::ProjectionType GetProjectionType() const { return Camera.GetProjectionType(); }
		void SetProjectionType(SceneCamera::ProjectionType type) { Camera.SetProjectionType(type); }

		float GetPerspectiveVerticalFOV() const { return Camera.GetPerspectiveVerticalFOV(); }
		void SetPerspectiveVerticalFOV(float verticalFOV) { Camera.SetPerspectiveVerticalFOV(verticalFOV); }
		float GetPerspectiveNearClip() const { return Camera.GetPerspectiveNearClip(); }
		void SetPerspectiveNearClip(float nearClip) { Camera.SetPerspectiveNearClip(nearClip); }
		float GetPerspectiveFarClip() const { return Camera.GetPerspectiveFarClip(); }
		void SetPerspectiveFarClip(float farClip) { Camera.SetPerspectiveFarClip(farClip); }

		float GetOrthographicSize() const { return Camera.GetOrthographicSize(); }
		void SetOrthographicSize(float size) { Camera.SetOrthographicSize(size); }
		float GetOrthographicNearClip() const { return Camera.GetOrthographicNearClip(); }
		void SetOrthographicNearClip(float nearClip) { Camera.SetOrthographicNearClip(nearClip); }
		float GetOrthographicFarClip() const { return Camera.GetOrthographicFarClip(); }
		void SetOrthographicFarClip(float farClip) { Camera.SetOrthographicFarClip(farClip); }

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

	struct ParticleSystemComponent
	{
		Ref<ParticleSystem> ParticleSystem;

		ParticleSystemComponent() = default;
		ParticleSystemComponent(const ParticleSystemComponent&) = default;
	};

	struct ParticleSystemPreviewComponent : public ParticleSystemComponent
	{
		using ParticleSystemComponent::ParticleSystemComponent;


	};

}
