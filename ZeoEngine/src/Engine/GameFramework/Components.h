#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>

#include "Engine/GameFramework/ComponentHelpers.h"
#include "Engine/GameFramework/SceneCamera.h"
#include "Engine/GameFramework/ScriptableEntity.h"
#include "Engine/GameFramework/ParticleSystem.h"

namespace ZeoEngine {

	class IComponentHelper;
	class Texture2D;

	struct IComponent
	{
		Entity OwnerEntity;
		Ref<IComponentHelper> ComponentHelper;

		IComponent() = default;
		IComponent(const IComponent&) = default;
	};

	struct CoreComponent : public IComponent
	{
		std::string Name;
		size_t CreationId;
		bool bIsInternal = false;
		std::vector<uint32_t> OrderedComponents;

		CoreComponent() = default;
		CoreComponent(const CoreComponent&) = default;

	};

	struct TransformComponent : public IComponent
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
			glm::mat4 rotation = glm::toMat4(glm::quat(Rotation));

			return glm::translate(glm::mat4(1.0f), Translation) *
				rotation *
				glm::scale(glm::mat4(1.0f), Scale);
		}

	};

	struct SpriteRendererComponent : public IComponent
	{
		glm::vec4 TintColor{ 1.0f, 1.0f, 1.0f, 1.0f };
		Ref<Texture2D> Texture;
		glm::vec2 TextureTiling{ 1.0f };

		int32_t SortingOrder = 0;

		SpriteRendererComponent() = default;
		SpriteRendererComponent(const SpriteRendererComponent&) = default;
		SpriteRendererComponent(const glm::vec4& color)
			: TintColor(color) {}
		SpriteRendererComponent(const Ref<Texture2D>& texture, const glm::vec4& tintColor = glm::vec4(1.0f), const glm::vec2& textureTiling = { 1.0f, 1.0f })
			: Texture(texture), TintColor(tintColor), TextureTiling(textureTiling) {}

	};

	struct CameraComponent : public IComponent
	{
		SceneCamera Camera;
		bool bIsPrimary = true;
		bool bFixedAspectRatio = false;

		CameraComponent() = default;
		CameraComponent(const CameraComponent&) = default;

		SceneCamera::ProjectionType GetProjectionType() const { return Camera.GetProjectionType(); }
		void SetProjectionType(SceneCamera::ProjectionType type) { Camera.SetProjectionType(type); }

		float GetPerspectiveVerticalFOV() const { return glm::degrees(Camera.GetPerspectiveVerticalFOV()); }
		void SetPerspectiveVerticalFOV(float verticalFOV) { Camera.SetPerspectiveVerticalFOV(glm::radians(verticalFOV)); }
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

	struct NativeScriptComponent : public IComponent
	{
		using InstantiateScriptDef = ScriptableEntity*(*)();
		using DestroyScriptDef = void(*)(NativeScriptComponent*);

		ScriptableEntity* Instance = nullptr;

		InstantiateScriptDef InstantiateScript;
		DestroyScriptDef DestroyScript;

		template<typename T, typename ... Args>
		void Bind(Args&& ... args)
		{
			InstantiateScript = []() { return static_cast<ScriptableEntity*>(new T()); };
			DestroyScript = [](NativeScriptComponent* nsc) { delete nsc->Instance; nsc->Instance = nullptr; };
		}
	};

	struct ParticleSystemComponent : public IComponent
	{
		Ref<ParticleTemplate> Template;
		Ref<ParticleSystemInstance> Instance;
		glm::vec3 PositionOffset{ 0.0f };

		ParticleSystemComponent()
		{
			ComponentHelper = CreateRef<ParticleSystemComponentHelper>();
		}
		ParticleSystemComponent(const ParticleSystemComponent&) = default;

	};

	struct ParticleSystemPreviewComponent : public ParticleSystemComponent
	{
		ParticleSystemPreviewComponent()
		{
			ComponentHelper = CreateRef<ParticleSystemPreviewComponentHelper>();
			Template = CreateRef<ParticleTemplate>();
		}
		ParticleSystemPreviewComponent(const ParticleSystemPreviewComponent&) = default;
		explicit ParticleSystemPreviewComponent(const Ref<ParticleTemplate>& pTemplate)
		{
			Template = pTemplate;
		}

		bool IsLocalSpace() const { return Template->bIsLocalSpace; }
		void SetLocalSpace(bool bValue) { Template->bIsLocalSpace = bValue; }
		int32_t GetLoopCount() const { return Template->LoopCount; }
		void SetLoopCount(int32_t count) { Template->LoopCount = count; }
		float GetLoopDuration() const { return Template->LoopDuration; }
		void SetLoopDuration(float duration) { Template->LoopDuration = duration; }
		ParticleFloat& GetSpawnRate() const { return Template->SpawnRate; }
		auto& GetBurstList() const { return Template->BurstList; }
		ParticleVec3& GetInitialPosition() const { return Template->InitialPosition; }
		ParticleVec3& GetInitialRotation() const { return Template->InitialRotation; }
		ParticleVec3& GetRotationRate() const { return Template->RotationRate; }
		ParticleVec3& GetSizeBegin() const { return Template->SizeBegin; }
		ParticleVec3& GetSizeEnd() const { return Template->SizeEnd; }
		ParticleVec3& GetInitialVelocity() const { return Template->InitialVelocity; }
		const glm::vec3& GetInheritVelocityRatio() const { return Template->InheritVelocityRatio; }
		void SetInheritVelocityRatio(const glm::vec3& ratio) { Template->InheritVelocityRatio = ratio; }
		ParticleColor& GetColorBegin() const { return Template->ColorBegin; }
		ParticleColor& GetColorEnd() const { return Template->ColorEnd; }
		ParticleFloat& GetLifetime() const { return Template->Lifetime; }
		const Ref<Texture2D>& GetTexture() const { return Template->Texture; }
		void SetTexture(const Ref<Texture2D>& texture) { Template->Texture = texture; }
		const glm::vec2& GetSubImageSize() const { return Template->SubImageSize; }
		void SetSubImageSize(const glm::vec2& size) { Template->SubImageSize = size; }
		uint32_t GetMaxParticles() const { return Template->MaxParticles; }
		void SetMaxParticles(uint32_t count) { Template->MaxParticles = count; }
		const Ref<Texture2D>& GetPreviewThumbnail() const { return Template->PreviewThumbnail; }
		void SetPreviewThumbnail(const Ref<Texture2D>& texture) { Template->PreviewThumbnail = texture; }

	};

}
