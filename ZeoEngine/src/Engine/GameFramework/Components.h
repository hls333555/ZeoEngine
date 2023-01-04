#pragma once

#include <IconsFontAwesome5.h>

#include "Engine/Asset/AssetLibrary.h"
#include "Engine/GameFramework/Entity.h"
#include "Engine/Core/UUID.h"
#include "Engine/GameFramework/SceneCamera.h"
#include "Engine/GameFramework/ParticleSystem.h"
#include "Engine/Renderer/Texture.h"
#include "Engine/Renderer/Mesh.h"
#include "Engine/Renderer/Material.h"
#include "Engine/Math/BoxSphereBounds.h"
#include "Engine/Physics/PhysicsMaterial.h"

namespace ZeoEngine {

	class Texture2D;
	class ScriptableEntity;
	class PhysicsMaterial;

	struct IComponent
	{
		Entity OwnerEntity;

		IComponent() = default;
		IComponent(const IComponent&) = default;
		IComponent(IComponent&&) = default;
		virtual ~IComponent() = default;

		IComponent& operator=(const IComponent&) = default;
		IComponent& operator=(IComponent&&) = default;

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
	};
#endif

	struct CoreComponent : public IComponent
	{
		std::string Name;

		U32 EntityIndex;
		/** Stores list of registered component IDs in order */
		std::vector<U32> OrderedComponents;
	};

	struct IDComponent : public IComponent
	{
		UUID ID;

		IDComponent() = default;
		IDComponent(UUID uuid)
			: ID(uuid) {}
	};

	struct RelationshipComponent : public IComponent
	{
		UUID ParentEntity = 0;
		std::vector<UUID> ChildEntities;
	};

	// Stores local transform
	struct TransformComponent : public IComponent
	{
		Vec3 Translation = { 0.0f, 0.0f, 0.0f };
		Vec3 Rotation = { 0.0f, 0.0f, 0.0f }; // Stored in degrees
		Vec3 Scale = { 1.0f, 1.0f, 1.0f };

		TransformComponent() = default;
		TransformComponent(const Vec3& translation)
			: Translation(translation) {}

		static const char* GetIcon() { return ICON_FA_MAP_MARKER_ALT; }

		Vec3 GetRotationInRadians() const { return glm::radians(Rotation); }

		Mat4 GetTransform() const
		{
			return Math::ComposeTransform(Translation, GetRotationInRadians(), Scale);
		}
		void SetTransform(const Mat4& transform)
		{
			Math::DecomposeTransform(transform, Translation, Rotation, Scale);
			Rotation = glm::degrees(Rotation);
		}
	};

	// Stores world transform
	// Only present on child entities
	struct WorldTransformComponent : public IComponent
	{
		Vec3 Translation = { 0.0f, 0.0f, 0.0f };
		Vec3 Rotation = { 0.0f, 0.0f, 0.0f }; // Stored in radians
		Vec3 Scale = { 1.0f, 1.0f, 1.0f };

		WorldTransformComponent() = default;
		WorldTransformComponent(const Vec3& translation, const Vec3& rotation, const Vec3& scale)
			: Translation(translation), Rotation(rotation), Scale(scale) {}

		Mat4 GetTransform() const
		{
			return Math::ComposeTransform(Translation, Rotation, Scale);
		}
		void SetTransform(const Mat4& transform)
		{
			Math::DecomposeTransform(transform, Translation, Rotation, Scale);
		}
	};

	struct BoundsComponent : public IComponent
	{
		BoxSphereBounds Bounds;
		std::unordered_map<U32, std::function<BoxSphereBounds(Entity)>> BoundsCalculationFuncs;
	};

	struct BillboardComponent : public IComponent
	{
		Ref<Texture2D> TextureAsset;
		Vec2 Size{ 0.5f, 0.5f };
	};

	struct FieldChangeComponent : public IComponent
	{
		U32 FieldID = 0;
	};

#pragma region 2D
	struct SpriteRendererComponent : public IComponent
	{
		AssetHandle TextureAsset;
		Vec4 TintColor{ 1.0f, 1.0f, 1.0f, 1.0f };
		Vec2 TextureTiling{ 1.0f };
		I32 SortingOrder = 0;

		SpriteRendererComponent() = default;
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

		static const char* GetIcon() { return ICON_FA_CIRCLE; }
	};

	struct RigidBody2DComponent : public IComponent
	{
		enum class BodyType
		{
			Static = 0, Dynamic, Kinematic
		};

		BodyType Type = BodyType::Static;
		bool bFixedRotation = false;

		void* RuntimeBody = nullptr;
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
	};
#pragma endregion

	struct CameraComponent : public IComponent
	{
		SceneCamera Camera;
		bool bIsPrimary = true;
		bool bFixedAspectRatio = false;

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

	enum class ScriptUpdateStage
	{
		Default, PrePhysics, PostPhysics
	};

	struct ScriptComponent : public IComponent
	{
		std::string ClassName;
		ScriptUpdateStage UpdateStage = ScriptUpdateStage::Default;

		static const char* GetIcon() { return ICON_FA_FILE_CODE; }
	};

	struct ParticleSystemComponent : public IComponent
	{
		Ref<ParticleTemplate> ParticleTemplateAsset; // TODO: Change to handle
		Vec3 PositionOffset{ 0.0f };

		Ref<ParticleSystemInstance> Instance;

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
		AssetHandle MeshAsset = 0;
		std::vector<AssetHandle> MaterialAssets;

		Ref<MeshInstance> Instance;

		MeshRendererComponent() = default;
		MeshRendererComponent(AssetHandle meshAsset)
			: MeshAsset(meshAsset) {}
		MeshRendererComponent(const MeshRendererComponent& other)
			: IComponent(other)
			, MeshAsset(other.MeshAsset), MaterialAssets(other.MaterialAssets), Instance(CreateRef<MeshInstance>(*other.Instance))
		{
		}
		MeshRendererComponent(MeshRendererComponent&&) = default;

		MeshRendererComponent& operator=(const MeshRendererComponent& other)
		{
			if (&other != this)
			{
				MeshAsset = other.MeshAsset;
				MaterialAssets = other.MaterialAssets;
				Instance = CreateRef<MeshInstance>(*other.Instance);
			}
			return *this;
		}
		MeshRendererComponent& operator=(MeshRendererComponent&&) = default;

		static const char* GetIcon() { return ICON_FA_CUBE; }
	};

	struct MeshDetailComponent : public IComponent
	{
		Ref<Mesh> LoadedMesh;
		Ref<MeshInstance> Instance;

		MeshDetailComponent() = default;
		MeshDetailComponent(const Ref<Mesh>& mesh)
			: LoadedMesh(mesh) {}

		std::vector<AssetHandle>& GetMaterialAssets() const { return LoadedMesh->GetDefaultMaterialAssets(); }
	};

	struct MaterialDetailComponent : public IComponent
	{
		Ref<Material> LoadedMaterial;

		MaterialDetailComponent() = default;
		MaterialDetailComponent(const Ref<Material>& material)
			: LoadedMaterial(material) {}

		AssetHandle& GetShaderAsset() const { return LoadedMaterial->m_ShaderInstance->m_ShaderAsset; }
		U32& GetShaderVariant() const { return LoadedMaterial->m_ShaderInstance->m_ShaderVariantID; }
	};

	struct TextureDetailComponent : public IComponent
	{
		Ref<Texture2D> LoadedTexture;
		SamplerType SamplerType = SamplerType::BilinearRepeat;

		TextureDetailComponent() = default;
		TextureDetailComponent(const Ref<Texture2D>& texture)
			: LoadedTexture(texture) {}

		bool& IsSRGB() const { return LoadedTexture->m_bIsSRGB; }
		bool& ShouldGenerateMipmaps() const { return LoadedTexture->m_bShouldGenerateMipmaps; }
	};

	enum class ShadowType
	{
		HardShadow = 0,
		PCF,
		PCSS,
	};

	struct LightComponentBase : public IComponent
	{
		Vec4 Color{ 1.0f };
		float Intensity = 1.0f;
		bool bCastShadow = false;
		ShadowType ShadowType = ShadowType::HardShadow;
		float DepthBias = 0.002f;
		float NormalBias = 30.0f;
		float FilterSize = 20.0f;
		float LightSize = 150.0f;
	};

	struct DirectionalLightComponent : public LightComponentBase
	{
		U32 CascadeCount = 4;
		float CascadeBlendThreshold = 0.5f;
		float MaxShadowDistance = 100.0f;
		float CascadeSplitLambda = 0.85f;

		static const char* GetIcon() { return ICON_FA_SUN; }
	};

	struct PointLightComponent : public LightComponentBase
	{
		float Range = 1.0f;

		static const char* GetIcon() { return ICON_FA_LIGHTBULB; }
	};

	struct SpotLightComponent : public PointLightComponent
	{
		/** Half the angle of the spot light cone stored in degrees */
		float CutoffAngle = 30.0f;

		float GetCutoffInRadians() const { return glm::radians(CutoffAngle); }

		static const char* GetIcon() { return ICON_FA_LIGHTBULB; }
	};

	struct RigidBodyComponent : public IComponent
	{
		enum class BodyType
		{
			Static = 0, Dynamic
		};

		enum class CollisionDetectionType
		{
			Discrete, Continuous, ContinuousSpeculative
		};

		BodyType Type = BodyType::Static;
		bool bIsKinematic = false;
		CollisionDetectionType CollisionDetection = CollisionDetectionType::Discrete;
		float Mass = 1.0f;
		float LinearDamping = 0.01f;
		float AngularDamping = 0.05f;
		bool bEnableGravity = true;

		U8 LockFlags = 0;

		bool bReceiveSleepEvents = false;
		bool bDisableSimulation = false;
	};

	struct PhysicsMaterialDetailComponent : public IComponent
	{
		Ref<PhysicsMaterial> LoadedPhysicsMaterial;

		PhysicsMaterialDetailComponent() = default;
		PhysicsMaterialDetailComponent(const Ref<PhysicsMaterial>& physicsMaterial)
			: LoadedPhysicsMaterial(physicsMaterial) {}

		float& GetStaticFriction() const { return LoadedPhysicsMaterial->m_StaticFriction; }
		float& GetDynamicFriction() const { return LoadedPhysicsMaterial->m_DynamicFriction; }
		float& GetBounciness() const { return LoadedPhysicsMaterial->m_Bounciness; }
	};

	struct ColliderComponentBase : public IComponent
	{
		AssetHandle PhysicsMaterialAsset;
		bool bEnableSimulation = true;
		bool bEnableQuery = true;
		bool bIsTrigger = false;
		U32 CollisionLayer = 0;
		U32 CollidesWithGroup = 0;
	};

	struct BoxColliderComponent : public ColliderComponentBase
	{
		Vec3 Size{ 1.0f };
		Vec3 Offset{ 0.0f };
	};

	struct SphereColliderComponent : public ColliderComponentBase
	{
		float Radius = 0.5f;
		Vec3 Offset{ 0.0f };
	};

	struct CapsuleColliderComponent : public ColliderComponentBase
	{
		float Radius = 0.5f;
		float Height = 1.0f;
		Vec3 Offset{ 0.0f };
	};

	struct CharacterControllerComponent : public IComponent
	{
		AssetHandle PhysicsMaterialAsset;
		U32 CollisionLayer = 0;
		U32 CollidesWithGroup = 0;
		float Radius = 0.5f;
		float Height = 1.0f;
		Vec3 Offset{ 0.0f };

		float SlopeLimitAngle = 45.0f; // In degrees
		float SkinThickness = 0.05f;
		float StepOffset = 0.3f;
	};

}
