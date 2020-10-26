#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

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
		Ref<Texture2D> Texture2DVar;

		TestComponent() = default;
		TestComponent(const TestComponent&) = default;
	};
#endif

	struct CoreComponent
	{
		std::string Name;
		uint32_t CreationId;
		bool bIsInternal{ false };

		CoreComponent() = default;
		CoreComponent(const CoreComponent&) = default;

	};

	struct TransformComponent
	{
		glm::vec3 Translation = { 0.0f, 0.0f, 0.0f };
		glm::vec3 Rotation = { 0.0f, 0.0f, 0.0f }; // Stored in radians
		glm::vec3 Scale = { 1.0f, 1.0f, 1.0f };

		TransformComponent() = default;
		TransformComponent(const TransformComponent&) = default;
		TransformComponent(const glm::vec3& translation)
			: Translation(translation) {}

		glm::vec3 GetRotationAsDegrees() const
		{
			return glm::degrees(Rotation);
		}

		void SetRotationToRadians(const glm::vec3& rotationInDegrees)
		{
			Rotation = glm::radians(rotationInDegrees);
		}

		glm::mat4 GetTransform() const
		{
			glm::mat4 rotation = glm::rotate(glm::mat4(1.0f), Rotation.x, { 1, 0, 0 }) *
				glm::rotate(glm::mat4(1.0f), Rotation.y, { 0, 1, 0 }) *
				glm::rotate(glm::mat4(1.0f), Rotation.z, { 0, 0, 1 });

			return glm::translate(glm::mat4(1.0f), Translation) *
				rotation *
				glm::scale(glm::mat4(1.0f), Scale);
		}

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

	struct ParticleSystemDetailComponent : public ParticleSystemComponent
	{
		using ParticleSystemComponent::ParticleSystemComponent;


	};

}
