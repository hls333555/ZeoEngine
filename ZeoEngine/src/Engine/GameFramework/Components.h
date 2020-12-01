#pragma once

#include <any>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "Engine/Renderer/Texture.h"
#include "Engine/GameFramework/SceneCamera.h"
#include "Engine/GameFramework/ScriptableEntity.h"
#include "Engine/GameFramework/ParticleSystem.h"
#include "Engine/Core/ReflectionCore.h"

#define ENABLE_TEST 0

namespace ZeoEngine {

	struct Component
	{
		Entity OwnerEntity;

		Component() = default;
		Component(const Component&) = default;

		// Callbacks
		/** Called before this component has been removed from the owner entity. */
		virtual void OnDestroy() {}
		/** Called every time this data is changed in the editor. (e.g. during dragging a slider to tweak the value) */
		virtual void OnDataValueEditChange(uint32_t dataId, std::any oldValue) {}
		/** Called only when this data is changed and deactivated in the editor. (e.g. after dragging a slider to tweak the value) */
		virtual void PostDataValueEditChange(uint32_t dataId, std::any oldValue) {}
	};

#if ENABLE_TEST
	struct TestComponent : public Component
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
		int8_t I8Var;
		int32_t I32Var;
		int64_t I64Var;
		float FloatVar;
		double DoubleVar;
		TestEnum EnumVar;
		TestEnumClass EnumClassVar;
		std::string StringVar;
		glm::vec2 Vec2Var;
		glm::vec3 Vec3Var;
		glm::vec4 ColorVar;
		Ref<Texture2D> Texture2DVar;
		Ref<ParticleTemplate> ParticleTemplateVar;

		std::deque<bool> BoolDeqVar;
		std::vector<uint8_t> Ui8VecVar;
		std::vector<uint32_t> Ui32VecVar;
		std::vector<uint64_t> Ui64VecVar;
		std::vector<int8_t> I8VecVar;
		std::vector<int32_t> I32VecVar;
		std::vector<int64_t> I64VecVar;
		std::vector<float> FloatVecVar;
		std::vector<double> DoubleVecVar;
		std::vector<TestEnum> EnumVecVar;
		std::vector<std::string> StringVecVar;
		std::vector<glm::vec2> Vec2VecVar;
		std::vector<glm::vec3> Vec3VecVar;
		std::vector<glm::vec4> ColorVecVar;
		std::vector<Ref<ParticleTemplate>> ParticleTemplateVecVar;

		TestComponent() = default;
		TestComponent(const TestComponent&) = default;
	};
#endif

	struct CoreComponent : public Component
	{
		std::string Name;
		size_t CreationId;
		bool bIsInternal{ false };

		CoreComponent() = default;
		CoreComponent(const CoreComponent&) = default;

	};

	struct TransformComponent : public Component
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

	struct SpriteRendererComponent : public Component
	{
		glm::vec4 TintColor{ 1.0f, 1.0f, 1.0f, 1.0f };
		Ref<Texture2D> Texture;
		glm::vec2 TextureTiling{ 1.0f };

		SpriteRendererComponent() = default;
		SpriteRendererComponent(const SpriteRendererComponent&) = default;
		SpriteRendererComponent(const glm::vec4& color)
			: TintColor(color) {}
		SpriteRendererComponent(const Ref<Texture2D>& texture, const glm::vec4& tintColor = glm::vec4(1.0f), const glm::vec2& textureTiling = { 1.0f, 1.0f })
			: Texture(texture), TintColor(tintColor), TextureTiling(textureTiling) {}

	};

	struct CameraComponent : public Component
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

	struct NativeScriptComponent : public Component
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

	struct ParticleSystemComponent : public Component
	{
		Ref<ParticleTemplate> Template;
		Ref<ParticleSystem> ParticleSystemRuntime;
		glm::vec3 PositionOffset{ 0.0f };

		ParticleSystemComponent() = default;
		ParticleSystemComponent(const ParticleSystemComponent&) = default;

		virtual void OnDestroy() override
		{
			// Clear particle system reference before this component has been removed
			RemoveParticleSystemInstance();
		}

		virtual void PostDataValueEditChange(uint32_t dataId, std::any oldValue) override
		{
			Ref<ParticleTemplate> oldTemplate;
			if (dataId == ZE_DATA_ID(Template))
			{
				oldTemplate = (*oldValue._Cast<Ref<ParticleTemplate>>());
			}
			CreateParticleSystem(oldTemplate);
		}

		void CreateParticleSystem(const Ref<ParticleTemplate>& oldTemplate)
		{
			// Clear last reference before creating new instance
			if (oldTemplate)
			{
				oldTemplate->RemoveParticleSystemInstance(ParticleSystemRuntime);
			}
			if (Template)
			{
				ParticleSystemRuntime = CreateRef<ParticleSystem>(Template, PositionOffset, OwnerEntity);
				// Add newly created instance to cache
				Template->AddParticleSystemInstance(ParticleSystemRuntime);
			}
		}

		void UpdateParticleSystem(DeltaTime dt)
		{
			if (ParticleSystemRuntime)
			{
				ParticleSystemRuntime->OnUpdate(dt);
			}
		}

		void RenderParticleSystem()
		{
			if (ParticleSystemRuntime)
			{
				ParticleSystemRuntime->OnRender();
			}
		}

		void RemoveParticleSystemInstance()
		{
			if (Template)
			{
				Template->RemoveParticleSystemInstance(ParticleSystemRuntime);
			}
		}

	};

	struct ParticleSystemPreviewComponent : public ParticleSystemComponent
	{
		ParticleSystemPreviewComponent()
		{
			Template = CreateRef<ParticleTemplate>();
		}
		ParticleSystemPreviewComponent(const ParticleSystemPreviewComponent&) = default;
		ParticleSystemPreviewComponent(const Ref<ParticleTemplate>& pTemplate)
		{
			Template = pTemplate;
		}

		virtual void PostDataValueEditChange(uint32_t dataId, std::any oldValue) override
		{
			// Notify all alive instances to reflect the change
			for (const auto& ps : Template->ParticleSystemInstances)
			{
				ps->Reevaluate();
			}
		}

		void SetTemplate(const Ref<ParticleTemplate>& pTemplate)
		{
			auto oldTemplate = Template;
			Template = pTemplate;
			CreateParticleSystem(oldTemplate);
		}

		bool IsLocalSpace() const { return Template->bIsLocalSpace; }
		void SetLocalSpace(bool bValue) { Template->bIsLocalSpace = bValue; }
		int32_t GetLoopCount() const { return Template->LoopCount; }
		void SetLoopCount(int32_t count) { Template->LoopCount = count; }
		float GetLoopDuration() const { return Template->LoopDuration; }
		void SetLoopDuration(float duration) { Template->LoopDuration = duration; }
		const ParticleFloat& GetSpawnRate() const { return Template->SpawnRate; }
		void SetSpawnRate(const ParticleFloat& value) { Template->SpawnRate = value; }
		auto& GetBurstList() const { return Template->BurstList; }
		const ParticleVec3& GetInitialPosition() const { return Template->InitialPosition; }
		void SetInitialPosition(const ParticleVec3& position) { Template->InitialPosition = position; }
		const ParticleVec3& GetInitialRotation() const { return Template->InitialRotation; }
		void SetInitialRotation(const ParticleVec3& rotation) { Template->InitialRotation = rotation; }
		const ParticleVec3& GetRotationRate() const { return Template->RotationRate; }
		void SetRotationRate(const ParticleVec3& rotationRate) { Template->RotationRate = rotationRate; }
		const ParticleVec3& GetSizeBegin() const { return Template->SizeBegin; }
		void SetSizeBegin(const ParticleVec3& size) { Template->SizeBegin = size; }
		const ParticleVec3& GetSizeEnd() const { return Template->SizeEnd; }
		void SetSizeEnd(const ParticleVec3& size) { Template->SizeEnd = size; }
		const ParticleVec3& GetInitialVelocity() const { return Template->InitialVelocity; }
		void SetInitialVelocity(const ParticleVec3& velocity) { Template->InitialVelocity = velocity; }
		const glm::vec3& GetInheritVelocityRatio() const { return Template->InheritVelocityRatio; }
		void SetInheritVelocityRatio(const glm::vec3& ratio) { Template->InheritVelocityRatio = ratio; }
		const ParticleColor& GetColorBegin() const { return Template->ColorBegin; }
		void SetColorBegin(const ParticleColor& color) { Template->ColorBegin = color; }
		const ParticleColor& GetColorEnd() const { return Template->ColorEnd; }
		void SetColorEnd(const ParticleColor& color) { Template->ColorEnd = color; }
		const ParticleFloat& GetLifetime() const { return Template->Lifetime; }
		void SetLifetime(const ParticleFloat& lifetime) { Template->Lifetime = lifetime; }
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
