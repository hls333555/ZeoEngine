#pragma once

#include <any>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>

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
		/** Called every time this data is changed in the editor. (e.g. DURING dragging a slider to tweak the value) */
		virtual void OnDataValueEditChange(uint32_t dataId, std::any oldValue) {}
		/** Called only when this data is changed and deactivated in the editor. (e.g. AFTER dragging a slider to tweak the value) */
		virtual void PostDataValueEditChange(uint32_t dataId, std::any oldValue) {}
	};

	// TODO: Move to Test.h
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

		struct TestStruct1
		{
			bool operator==(const TestStruct1& other) const
			{
				return EnumVar == other.EnumVar && I32Var == other.I32Var;
			}

			TestEnum EnumVar;
			int32_t I32Var;
		};

		struct TestStruct2
		{
			bool operator==(const TestStruct2& other) const
			{
				return TestStruct1Var == other.TestStruct1Var && FloatVar == other.FloatVar;
			}

			TestStruct1 TestStruct1Var;
			float FloatVar;
		};

		TestStruct2& GetTestStruct2GetterVar() { return TestStruct2GetterVar; }
		float GetFloatSetterGetterVar() const { return FloatSetterGetterVar; } void SetFloatSetterGetterVar(float value) { FloatSetterGetterVar = value; }
		auto& GetTestStruct2VecGetterVar() { return TestStruct2VecGetterVar; }

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
		TestStruct1 TestStruct1Var;
		TestStruct2 TestStruct2GetterVar;
		float FloatSetterGetterVar;

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
		std::vector<Ref<Texture2D>> Texture2DVecVar;
		std::vector<Ref<ParticleTemplate>> ParticleTemplateVecVar;
		std::vector<TestStruct1> TestStruct1VecVar;
		std::vector<TestStruct2> TestStruct2VecGetterVar;

		TestComponent() = default;
		TestComponent(const TestComponent&) = default;
	};
#endif

	struct CoreComponent : public Component
	{
		std::string Name;
		size_t CreationId;
		bool bIsInternal = false;
		std::vector<uint32_t> OrderedComponents;

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
			glm::mat4 rotation = glm::toMat4(glm::quat(Rotation));

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
			RemoveCurrentParticleSystemInstance();
		}

		virtual void OnDataValueEditChange(uint32_t dataId, std::any oldValue) override
		{
			CreateParticleSystem(Template);
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

		void RemoveCurrentParticleSystemInstance()
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

		bool IsLocalSpace() const { return Template->bIsLocalSpace; } void SetLocalSpace(bool bValue) { Template->bIsLocalSpace = bValue; }
		int32_t GetLoopCount() const { return Template->LoopCount; } void SetLoopCount(int32_t count) { Template->LoopCount = count; }
		float GetLoopDuration() const { return Template->LoopDuration; } void SetLoopDuration(float duration) { Template->LoopDuration = duration; }
		ParticleFloat& GetSpawnRate() const { return Template->SpawnRate; }
		auto& GetBurstList() const { return Template->BurstList; }
		ParticleVec3& GetInitialPosition() const { return Template->InitialPosition; }
		ParticleVec3& GetInitialRotation() const { return Template->InitialRotation; }
		ParticleVec3& GetRotationRate() const { return Template->RotationRate; }
		ParticleVec3& GetSizeBegin() const { return Template->SizeBegin; }
		ParticleVec3& GetSizeEnd() const { return Template->SizeEnd; }
		ParticleVec3& GetInitialVelocity() const { return Template->InitialVelocity; }
		const glm::vec3& GetInheritVelocityRatio() const { return Template->InheritVelocityRatio; } void SetInheritVelocityRatio(const glm::vec3& ratio) { Template->InheritVelocityRatio = ratio; }
		ParticleColor& GetColorBegin() const { return Template->ColorBegin; }
		ParticleColor& GetColorEnd() const { return Template->ColorEnd; }
		ParticleFloat& GetLifetime() const { return Template->Lifetime; }
		const Ref<Texture2D>& GetTexture() const { return Template->Texture; } void SetTexture(const Ref<Texture2D>& texture) { Template->Texture = texture; }
		const glm::vec2& GetSubImageSize() const { return Template->SubImageSize; } void SetSubImageSize(const glm::vec2& size) { Template->SubImageSize = size; }
		uint32_t GetMaxParticles() const { return Template->MaxParticles; } void SetMaxParticles(uint32_t count) { Template->MaxParticles = count; }
		const Ref<Texture2D>& GetPreviewThumbnail() const { return Template->PreviewThumbnail; } void SetPreviewThumbnail(const Ref<Texture2D>& texture) { Template->PreviewThumbnail = texture; }

	};

}
