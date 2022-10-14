#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>
#include <IconsFontAwesome5.h>

#include "Engine/Asset/AssetLibrary.h"
#include "Engine/GameFramework/Entity.h"
#include "Engine/Core/UUID.h"
#include "Engine/GameFramework/SceneCamera.h"
#include "Engine/GameFramework/ParticleSystem.h"
#include "Engine/Renderer/Texture.h"
#include "Engine/Renderer/Light.h"
#include "Engine/Renderer/Mesh.h"
#include "Engine/Renderer/Material.h"
#include "Engine/Math/BoxSphereBounds.h"

namespace ZeoEngine {

	class Texture2D;
	class ScriptableEntity;

	extern std::vector<AssetHandle> g_AssetVectorPlaceholder;

	struct IComponent
	{
		Entity OwnerEntity;

		IComponent() = default;
		IComponent(const IComponent&) = default;
		virtual ~IComponent() = default;

		static const char* GetIcon() { return ICON_FA_CIRCLE_NOTCH; }
	};

#ifndef DOCTEST_CONFIG_DISABLE
	struct TestComponent : public IComponent
	{
		enum TestEnum
		{
			TestEnum1, TestEnum2, TestEnum3
		};

		enum class TestEnumClass
		{
			TestEnumClass1, TestEnumClass2, TestEnumClass3
		};

		bool& ShowSequenceContainers() { return bShowSequenceContainers; }

		bool BoolVar;
		U8 U8Var;
		U16 U16Var;
		U32 U32Var;
		U64 U64Var;
		I8 I8Var;
		I16 I16Var;
		I32 I32Var;
		I64 I64Var;
		float FloatVar;
		double DoubleVar;
		TestEnum EnumVar;
		TestEnumClass EnumClassVar;
		std::string StringVar;
		Vec2 Vec2Var;
		Vec3 Vec3Var;
		Vec4 ColorVar;
		AssetHandle TextureAssetVar;
		AssetHandle MeshAssetVar;

		bool bShowSequenceContainers;

		std::vector<bool> BoolVecVar;
		std::vector<U8> U8VecVar;
		std::vector<U16> U16VecVar;
		std::vector<U32> U32VecVar;
		std::vector<U64> U64VecVar;
		std::vector<I8> I8VecVar;
		std::vector<I16> I16VecVar;
		std::vector<I32> I32VecVar;
		std::vector<I64> I64VecVar;
		std::vector<float> FloatVecVar;
		std::vector<double> DoubleVecVar;
		std::vector<TestEnum> EnumVecVar;
		std::vector<TestEnumClass> EnumClassVecVar;
		std::vector<std::string> StringVecVar;
		std::vector<Vec2> Vec2VecVar;
		std::vector<Vec3> Vec3VecVar;
		std::vector<Vec4> ColorVecVar;
		std::vector<AssetHandle> TextureAssetVecVar;
		std::vector<AssetHandle> MeshAssetVecVar;

		TestComponent() = default;
		TestComponent(const TestComponent&) = default;
	};
#endif

	struct CoreComponent : public IComponent
	{
		std::string Name;

		U32 EntityIndex;
		std::vector<U32> OrderedComponents;

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
		Vec3 Translation = { 0.0f, 0.0f, 0.0f };
		Vec3 Rotation = { 0.0f, 0.0f, 0.0f }; // Stored in degrees
		Vec3 Scale = { 1.0f, 1.0f, 1.0f };

		TransformComponent() = default;
		TransformComponent(const TransformComponent&) = default;
		TransformComponent(const Vec3& translation)
			: Translation(translation) {}

		static const char* GetIcon() { return ICON_FA_MAP_MARKER_ALT; }

		Vec3 GetRotationInRadians() const { return glm::radians(Rotation); }

		Mat4 GetTransform() const
		{
			const Mat4 rotation = glm::toMat4(glm::quat(GetRotationInRadians()));
			return glm::translate(Mat4(1.0f), Translation) *
				rotation *
				glm::scale(Mat4(1.0f), Scale);
		}

	};

#pragma region 2D
	struct SpriteRendererComponent : public IComponent
	{
		AssetHandle TextureAsset;
		Vec4 TintColor{ 1.0f, 1.0f, 1.0f, 1.0f };
		Vec2 TextureTiling{ 1.0f };
		I32 SortingOrder = 0;

		SpriteRendererComponent() = default;
		SpriteRendererComponent(const SpriteRendererComponent&) = default;
		SpriteRendererComponent(const Vec4& color)
			: TintColor(color) {}
		SpriteRendererComponent(AssetHandle textureAsset, const Vec4& tintColor = Vec4(1.0f), const Vec2& textureTiling = { 1.0f, 1.0f })
			: TextureAsset(textureAsset), TintColor(tintColor), TextureTiling(textureTiling) {}

		Ref<Texture2D> GetTexture() const { return AssetLibrary::LoadAsset<Texture2D>(TextureAsset); }

		static const char* GetIcon() { return ICON_FA_GHOST; }

	};

	struct CircleRendererComponent : public IComponent
	{
		Vec4 Color{ 1.0f, 1.0f, 1.0f, 1.0f };
		float Thickness = 1.0f;
		float Fade = 0.005f;
		I32 SortingOrder = 0;

		CircleRendererComponent() = default;
		CircleRendererComponent(const CircleRendererComponent&) = default;

		static const char* GetIcon() { return ICON_FA_CIRCLE; }

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
		Vec2 Offset = { 0.0f, 0.0f };
		Vec2 Size = { 0.5f, 0.5f }; // Half width and half height

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
		Vec2 Offset = { 0.0f, 0.0f };
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
#pragma endregion

	struct CameraComponent : public IComponent
	{
		SceneCamera Camera;
		bool bIsPrimary = true;
		bool bFixedAspectRatio = false;

		CameraComponent() = default;
		CameraComponent(const CameraComponent&) = default;

		static const char* GetIcon() { return ICON_FA_CAMERA; }

		SceneCamera::ProjectionType& GetProjectionType() { return Camera.m_ProjectionType; }

		float& GetPerspectiveVerticalFOV() { return Camera.m_PerspectiveFOV; }
		float& GetPerspectiveNearClip() { return Camera.m_PerspectiveNear; }
		float& GetPerspectiveFarClip() { return Camera.m_PerspectiveFar; }

		float& GetOrthographicSize() { return Camera.m_OrthographicSize; }
		float& GetOrthographicNearClip() { return Camera.m_OrthographicNear; }
		float& GetOrthographicFarClip() { return Camera.m_OrthographicFar; }

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

	struct ScriptComponent : public IComponent
	{
		std::string ClassName;

		ScriptComponent() = default;
		ScriptComponent(const ScriptComponent&) = default;

		static const char* GetIcon() { return ICON_FA_FILE_CODE; }
	};

	struct ParticleSystemComponent : public IComponent
	{
		Ref<ParticleTemplate> ParticleTemplateAsset; // TODO: Change to handle
		Vec3 PositionOffset{ 0.0f };

		Ref<ParticleSystemInstance> Instance;

		ParticleSystemComponent() = default;
		ParticleSystemComponent(const ParticleSystemComponent&) = default;

		static const char* GetIcon() { return ICON_FA_FIRE_ALT; }
	};

	struct ParticleSystemDetailComponent : public ParticleSystemComponent
	{
		ParticleSystemDetailComponent() = default;
		ParticleSystemDetailComponent(const Ref<ParticleTemplate>& particleTemplate)
		{
			ParticleTemplateAsset = particleTemplate;
		}

		bool& IsLocalSpace() const { return ParticleTemplateAsset->bIsLocalSpace; }
		I32& GetLoopCount() const { return ParticleTemplateAsset->LoopCount; }
		float& GetLoopDuration() const { return ParticleTemplateAsset->LoopDuration; }
		ParticleFloat& GetSpawnRate() { return ParticleTemplateAsset->SpawnRate; }
		auto& GetBurstList() { return ParticleTemplateAsset->BurstList; }
		ParticleVec3& GetInitialPosition() { return ParticleTemplateAsset->InitialPosition; }
		ParticleVec3& GetInitialRotation() { return ParticleTemplateAsset->InitialRotation; }
		ParticleVec3& GetRotationRate() { return ParticleTemplateAsset->RotationRate; }
		ParticleVec3& GetSizeBegin() { return ParticleTemplateAsset->SizeBegin; }
		ParticleVec3& GetSizeEnd() { return ParticleTemplateAsset->SizeEnd; }
		ParticleVec3& GetInitialVelocity() { return ParticleTemplateAsset->InitialVelocity; }
		Vec3& GetInheritVelocityRatio() const { return ParticleTemplateAsset->InheritVelocityRatio; }
		ParticleColor& GetColorBegin() { return ParticleTemplateAsset->ColorBegin; }
		ParticleColor& GetColorEnd() { return ParticleTemplateAsset->ColorEnd; }
		ParticleFloat& GetLifetime() { return ParticleTemplateAsset->Lifetime; }
		Ref<Texture2D>& GetTexture() const { return ParticleTemplateAsset->Texture; }
		Vec2& GetSubImageSize() const { return ParticleTemplateAsset->SubImageSize; }
		U32& GetMaxParticles() const { return ParticleTemplateAsset->MaxParticles; }

	};
	
	struct MeshRendererComponent : public IComponent
	{
		AssetHandle MeshAsset;

		Ref<MeshInstance> Instance;

		MeshRendererComponent() = default;
		MeshRendererComponent(AssetHandle meshAsset)
			: MeshAsset(meshAsset) {}
		MeshRendererComponent(const MeshRendererComponent&) = default;

		Ref<Mesh> GetMesh() const { return AssetLibrary::LoadAsset<Mesh>(MeshAsset); }

		std::vector<AssetHandle>& GetMaterialAssets() const { return Instance ? Instance->GetMaterialAssets() : g_AssetVectorPlaceholder; }

		static const char* GetIcon() { return ICON_FA_CUBE; }
	};

	struct MeshDetailComponent : public IComponent
	{
		Ref<Mesh> LoadedMesh;
		Ref<MeshInstance> Instance;

		MeshDetailComponent() = default;
		MeshDetailComponent(const Ref<Mesh>& mesh)
			: LoadedMesh(mesh) {}
		MeshDetailComponent(const MeshDetailComponent&) = default;

		std::vector<AssetHandle>& GetMaterialAssets() const { return LoadedMesh ? LoadedMesh->GetDefaultMaterialAssets() : g_AssetVectorPlaceholder; }
	};

	struct LightComponentBase : public IComponent
	{
		Ref<Light> LightSource;

		Vec4& GetColor() const { return LightSource->m_Color; }
		float& GetIntensity() const { return LightSource->m_Intensity; }
		bool& IsCastShadow() const { return LightSource->m_bCastShadow; }
		Light::ShadowType& GetShadowType() const { return LightSource->m_ShadowType; }
		float& GetDepthBias() const { return LightSource->m_DepthBias; }
		float& GetNormalBias() const { return LightSource->m_NormalBias; }
		float& GetLightSize() const { return LightSource->m_LightSize; }
		float& GetFilterSize() const { return LightSource->m_FilterSize; }
	};

	struct DirectionalLightComponent : public LightComponentBase
	{
		DirectionalLightComponent() = default;
		DirectionalLightComponent(const DirectionalLightComponent&) = default;

		static const char* GetIcon() { return ICON_FA_SUN; }

		Ref<DirectionalLight> GetDirectionalLight() const { return std::static_pointer_cast<DirectionalLight>(LightSource); }

		U32& GetCascadeCount() const { return GetDirectionalLight()->m_CascadeCount; }
		float& GetCascadeBlendThreshold() const { return GetDirectionalLight()->m_CascadeBlendThreshold; }
		float& GetMaxShadowDistance() const { return GetDirectionalLight()->m_MaxShadowDistance; }
		float& GetCascadeSplitLambda() const { return GetDirectionalLight()->m_CascadeSplitLambda; }
	};

	struct PointLightComponent : public LightComponentBase
	{
		PointLightComponent() = default;
		PointLightComponent(const PointLightComponent&) = default;

		static const char* GetIcon() { return ICON_FA_LIGHTBULB; }

		Ref<PointLight> GetPointLight() const { return std::static_pointer_cast<PointLight>(LightSource); }

		float& GetRange() const { return GetPointLight()->m_Range; }
	};

	struct SpotLightComponent : public PointLightComponent
	{
		SpotLightComponent() = default;
		SpotLightComponent(const SpotLightComponent&) = default;

		static const char* GetIcon() { return ICON_FA_LIGHTBULB; }

		Ref<SpotLight> GetSpotLight() const { return std::static_pointer_cast<SpotLight>(LightSource); }
		
		float& GetCutoff() const { return GetSpotLight()->m_CutoffAngle; }
	};

	struct MaterialDetailComponent : public IComponent
	{
		Ref<Material> LoadedMaterial;

		MaterialDetailComponent() = default;
		MaterialDetailComponent(const Ref<Material>& material)
			: LoadedMaterial(material) {}
		MaterialDetailComponent(const MaterialDetailComponent&) = default;

		AssetHandle& GetShaderAsset() const { return LoadedMaterial->m_ShaderInstance->m_ShaderAsset; }
		U32& GetShaderVariant() const { return LoadedMaterial->m_ShaderInstance->m_ShaderVariantID; }
	};

	struct BillboardComponent : public IComponent
	{
		Ref<Texture2D> TextureAsset;
		Vec2 Size{ 0.5f, 0.5f };

		BillboardComponent() = default;
		BillboardComponent(const BillboardComponent&) = default;
	};

	struct BoundsComponent : public IComponent
	{
		BoxSphereBounds Bounds;

		BoundsComponent() = default;
		BoundsComponent(const BoundsComponent&) = default;
	};

	struct TextureDetailComponent : public IComponent
	{
		Ref<Texture2D> LoadedTexture;
		SamplerType SamplerType = SamplerType::BilinearRepeat;

		TextureDetailComponent() = default;
		TextureDetailComponent(const Ref<Texture2D>& texture)
			: LoadedTexture(texture) {}
		TextureDetailComponent(const TextureDetailComponent&) = default;

		bool& IsSRGB() const { return LoadedTexture->m_bIsSRGB; }
		bool& ShouldGenerateMipmaps() const { return LoadedTexture->m_bShouldGenerateMipmaps; }
	};

}
