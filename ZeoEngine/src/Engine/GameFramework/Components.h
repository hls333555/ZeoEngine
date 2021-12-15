#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>
#include <IconsFontAwesome5.h>

#include "Engine/GameFramework/ComponentHelpers.h"
#include "Engine/Core/UUID.h"
#include "Engine/GameFramework/SceneCamera.h"
#include "Engine/GameFramework/ParticleSystem.h"
#include "Engine/Renderer/Light.h"
#include "Engine/Renderer/Mesh.h"
#include "Engine/Renderer/Material.h"
#include "Engine/Math/BoxSphereBounds.h"

namespace ZeoEngine {

	class Texture2DAsset;
	class ScriptableEntity;
	class ShaderAsset;

	struct IComponent
	{
		Ref<IComponentHelper> ComponentHelper;

		IComponent() = default;
		IComponent(const IComponent&) = default;

		virtual void CreateHelper(Entity* entity) {}
		template<typename T>
		Ref<T> GetHelper()
		{
			return std::dynamic_pointer_cast<T>(ComponentHelper);
		}

		static const char* GetIcon() { return ICON_FA_CIRCLE_NOTCH; }
	};

	struct CoreComponent : public IComponent
	{
		std::string Name;

		uint32_t EntityIndex;
		std::vector<uint32_t> OrderedComponents;

		CoreComponent() = default;
		CoreComponent(const CoreComponent&) = default;
	};

	struct IDComponent : public IComponent
	{
		UUID ID;

		IDComponent() = default;
		IDComponent(const IDComponent&) = default;
		IDComponent(UUID uuid)
			: ID(uuid) {}
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

		virtual void CreateHelper(Entity* entity) override
		{
			ComponentHelper = CreateRef<TransformComponentHelper>(entity);
		}

		static const char* GetIcon() { return ICON_FA_MAP_MARKER_ALT; }

		glm::vec3 GetRotationAsDegrees() const { return glm::degrees(Rotation); }
		void SetRotationToRadians(const glm::vec3& rotationInDegrees) { Rotation = glm::radians(rotationInDegrees); }

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
		AssetHandle<Texture2DAsset> Texture;
		glm::vec2 TextureTiling{ 1.0f };
		int32_t SortingOrder = 0;

		SpriteRendererComponent() = default;
		SpriteRendererComponent(const SpriteRendererComponent&) = default;
		SpriteRendererComponent(const glm::vec4& color)
			: TintColor(color) {}
		SpriteRendererComponent(const AssetHandle<Texture2DAsset>& texture, const glm::vec4& tintColor = glm::vec4(1.0f), const glm::vec2& textureTiling = { 1.0f, 1.0f })
			: Texture(texture), TintColor(tintColor), TextureTiling(textureTiling) {}

		static const char* GetIcon() { return ICON_FA_GHOST; }

	};

	struct CircleRendererComponent : public IComponent
	{
		glm::vec4 Color{ 1.0f, 1.0f, 1.0f, 1.0f };
		float Thickness = 1.0f;
		float Fade = 0.005f;
		int32_t SortingOrder = 0;

		CircleRendererComponent() = default;
		CircleRendererComponent(const CircleRendererComponent&) = default;

		static const char* GetIcon() { return ICON_FA_CIRCLE; }

	};

	struct CameraComponent : public IComponent
	{
		SceneCamera Camera;
		bool bIsPrimary = true;
		bool bFixedAspectRatio = false;

		CameraComponent() = default;
		CameraComponent(const CameraComponent&) = default;

		virtual void CreateHelper(Entity* entity) override
		{
			ComponentHelper = CreateRef<CameraComponentHelper>(entity);
		}

		static const char* GetIcon() { return ICON_FA_CAMERA; }

		SceneCamera::ProjectionType GetProjectionType() const { return Camera.GetProjectionType(); }
		void SetProjectionType(SceneCamera::ProjectionType type) { Camera.SetProjectionType(type); }

		float GetPerspectiveVerticalFOV() const { return glm::degrees(Camera.GetPerspectiveVerticalFOV()); }
		void SetPerspectiveVerticalFOV(float horizontalFOV) { Camera.SetPerspectiveVerticalFOV(glm::radians(horizontalFOV)); }
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
		AssetHandle<ParticleTemplateAsset> Template;
		glm::vec3 PositionOffset{ 0.0f };

		Ref<ParticleSystemInstance> Instance;

		ParticleSystemComponent() = default;
		ParticleSystemComponent(const ParticleSystemComponent&) = default;

		virtual void CreateHelper(Entity* entity) override
		{
			ComponentHelper = CreateRef<ParticleSystemComponentHelper>(entity);
		}

		static const char* GetIcon() { return ICON_FA_FIRE_ALT; }
	};

	struct ParticleSystemPreviewComponent : public ParticleSystemComponent
	{
		ParticleSystemPreviewComponent()
		{
			Template = ParticleTemplateAsset::Create(); // Create default particle asset
		}
		ParticleSystemPreviewComponent(const ParticleSystemPreviewComponent&) = default;
		explicit ParticleSystemPreviewComponent(const AssetHandle<ParticleTemplateAsset>& pTemplate)
		{
			Template = pTemplate;
		}

		virtual void CreateHelper(Entity* entity) override
		{
			ComponentHelper = CreateRef<ParticleSystemPreviewComponentHelper>(entity);
		}

		bool IsLocalSpace() const { return Template->bIsLocalSpace; }
		void SetLocalSpace(bool bValue) { Template->bIsLocalSpace = bValue; }
		int32_t GetLoopCount() const { return Template->LoopCount; }
		void SetLoopCount(int32_t count) { Template->LoopCount = count; }
		float GetLoopDuration() const { return Template->LoopDuration; }
		void SetLoopDuration(float duration) { Template->LoopDuration = duration; }
		ParticleFloat& GetSpawnRate() { return Template->SpawnRate; }
		auto& GetBurstList() { return Template->BurstList; }
		ParticleVec3& GetInitialPosition() { return Template->InitialPosition; }
		ParticleVec3& GetInitialRotation() { return Template->InitialRotation; }
		ParticleVec3& GetRotationRate() { return Template->RotationRate; }
		ParticleVec3& GetSizeBegin() { return Template->SizeBegin; }
		ParticleVec3& GetSizeEnd() { return Template->SizeEnd; }
		ParticleVec3& GetInitialVelocity() { return Template->InitialVelocity; }
		const glm::vec3& GetInheritVelocityRatio() const { return Template->InheritVelocityRatio; }
		void SetInheritVelocityRatio(const glm::vec3& ratio) { Template->InheritVelocityRatio = ratio; }
		ParticleColor& GetColorBegin() { return Template->ColorBegin; }
		ParticleColor& GetColorEnd() { return Template->ColorEnd; }
		ParticleFloat& GetLifetime() { return Template->Lifetime; }
		const AssetHandle<Texture2DAsset>& GetTexture() const { return Template->Texture; }
		void SetTexture(const AssetHandle<Texture2DAsset>& texture) { Template->Texture = texture; }
		const glm::vec2& GetSubImageSize() const { return Template->SubImageSize; }
		void SetSubImageSize(const glm::vec2& size) { Template->SubImageSize = size; }
		uint32_t GetMaxParticles() const { return Template->MaxParticles; }
		void SetMaxParticles(uint32_t count) { Template->MaxParticles = count; }

	};

	struct Rigidbody2DComponent : public IComponent
	{
		enum class BodyType
		{
			Static = 0, Dynamic, Kinematic
		};

		BodyType Type = BodyType::Static;
		bool bFixedRotation = false;

		void* RuntimeBody = nullptr;

		Rigidbody2DComponent() = default;
		Rigidbody2DComponent(const Rigidbody2DComponent&) = default;
	};

	struct BoxCollider2DComponent : public IComponent
	{
		glm::vec2 Offset = { 0.0f, 0.0f };
		glm::vec2 Size = { 0.5f, 0.5f }; // Half width and half height

		// TODO: Move into physics material
		float Density = 1.0f;
		float Friction = 0.5f;
		float Restitution = 0.0f;
		float RestitutionThreshold = 0.5f;

		void* RuntimeFixture = nullptr;

		BoxCollider2DComponent() = default;
		BoxCollider2DComponent(const BoxCollider2DComponent&) = default;
	};

	struct CircleCollider2DComponent : public IComponent
	{
		glm::vec2 Offset = { 0.0f, 0.0f };
		float Radius = 0.5f;

		// TODO: Move into physics material
		float Density = 1.0f;
		float Friction = 0.5f;
		float Restitution = 0.0f;
		float RestitutionThreshold = 0.5f;

		void* RuntimeFixture = nullptr;

		CircleCollider2DComponent() = default;
		CircleCollider2DComponent(const CircleCollider2DComponent&) = default;
	};
	
	struct MeshRendererComponent : public IComponent
	{
		AssetHandle<MeshAsset> Mesh;
		std::vector<AssetHandle<MaterialAsset>> MaterialsPlaceholder;

		MeshRendererComponent() = default;
		MeshRendererComponent(const AssetHandle<MeshAsset>& meshAsset)
			: Mesh(meshAsset) {}
		MeshRendererComponent(const MeshRendererComponent&) = default;

		virtual void CreateHelper(Entity* entity) override
		{
			ComponentHelper = CreateRef<MeshRendererComponentHelper>(entity);
		}

		auto& GetMaterials() { return Mesh ? Mesh->GetMesh()->GetMaterials() : MaterialsPlaceholder; }

		static const char* GetIcon() { return ICON_FA_CUBE; }
	};

	struct LightComponent : public IComponent
	{
		Ref<Light> LightSource;

		enum class LightType
		{
			DirectionalLight = 0, PointLight, SpotLight,
		};

		LightType Type = LightType::PointLight;

		LightComponent() = default;
		LightComponent(LightType type)
			: Type(type) {}
		LightComponent(const LightComponent& rhs) = default;

		virtual void CreateHelper(Entity* entity) override
		{
			ComponentHelper = CreateRef<LightComponentHelper>(entity);
		}

		static const char* GetIcon() { return ICON_FA_LIGHTBULB; }

		template<typename T>
		Ref<T> GetLight()
		{
			return std::dynamic_pointer_cast<T>(LightSource);
		}

		const glm::vec4& GetColor() const { return LightSource->GetColor(); }
		void SetColor(const glm::vec4& color) { LightSource->SetColor(color); }
		float GetIntensity() const { return LightSource->GetIntensity(); }
		void SetIntensity(float intensity) { return LightSource->SetIntensity(intensity); }
		float GetRange() const { return LightSource->GetRange(); }
		void SetRange(float range) { LightSource->SetRange(range); }
		float GetCutoffAsDegrees() const { return glm::degrees(LightSource->GetCutoff()); }
		void SetCutoffToRadians(float cutoffInDegrees) { LightSource->SetCutoff(glm::radians(cutoffInDegrees)); }
	};

	struct MaterialPreviewComponent : public IComponent
	{
		AssetHandle<MaterialAsset> Template;

		MaterialPreviewComponent() = default;
		explicit MaterialPreviewComponent(const AssetHandle<MaterialAsset>& material)
			: Template(material) {}
		MaterialPreviewComponent(const MaterialPreviewComponent&) = default;

		const AssetHandle<ShaderAsset>& GetShader() const { return Template->GetMaterial()->GetShaderAsset(); }
		void SetShader(const AssetHandle<ShaderAsset>& shader) { Template->GetMaterial()->SetShaderAsset(shader); } 
	};

	struct BillboardComponent : public IComponent
	{
		AssetHandle<Texture2DAsset> Texture;
		glm::vec2 Size{ 0.5f, 0.5f };

		BillboardComponent() = default;
		BillboardComponent(const BillboardComponent&) = default;
	};

	struct BoundsComponent : public IComponent
	{
		BoxSphereBounds Bounds;

		BoundsComponent() = default;
		BoundsComponent(const BoundsComponent&) = default;
	};

}
