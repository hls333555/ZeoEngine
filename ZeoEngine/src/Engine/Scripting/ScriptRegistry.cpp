#include "ZEpch.h"
#include "Engine/Scripting/ScriptRegistry.h"

#include <mono/metadata/object.h>
#include <mono/metadata/reflection.h>

#include "Engine/Scripting/ScriptEngine.h"
#include "Engine/Core/Input.h"
#include "Engine/Core/KeyCodes.h"
#include "Engine/Asset/AssetLibrary.h"
#include "Engine/GameFramework/Components.h"
#include "Engine/Physics/PhysXCharacterController.h"
#include "Engine/Physics/PhysXScene.h"

namespace ZeoEngine {

	namespace Utils {

		static MonoString* StringToMonoString(const std::string& str)
		{
			return mono_string_new(ScriptEngine::GetAppDomain(), str.c_str());
		}
		
	}

	/** @param Name - Both C# method name and C++ function name */
#define ZE_ADD_INTERNAL_CALL(Name) mono_add_internal_call("ZeoEngine.InternalCalls::" #Name, Name)
#define EXPAND_PARAM_TYPE(...) "(" #__VA_ARGS__ ")"
/**
 * @param Name - C# method name
 * @param OverloadName - C++ function name
 * @param ... - Overload params from C# with no space between
 *				Correct: ulong,bool,float
 *				Wrong: ulong, bool, float
 */
#define ZE_ADD_INTERNAL_CALL_OVERLOAD(Name, OverloadName, ...) mono_add_internal_call("ZeoEngine.InternalCalls::" #Name EXPAND_PARAM_TYPE(__VA_ARGS__), OverloadName)

	static MonoString* Entity_GetName(UUID entityID)
	{
		return Utils::StringToMonoString(ScriptEngine::GetEntityByID(entityID).GetName());
	}

	static bool Entity_HasComponent(UUID entityID, MonoReflectionType* compType)
	{
		return ScriptEngine::GetEntityByID(entityID).HasComponentByID(ScriptRegistry::GetComponentIDFromType(compType));
	}

	static U64 Entity_GetEntityByName(MonoString* name)
	{
		char* str = mono_string_to_utf8(name);
		const Entity entity = ScriptEngine::GetEntityByName(str);
		mono_free(str);
		return entity ? entity.GetUUID() : 0;
	}

	static MonoObject* GetScriptInstance(UUID entityID)
	{
		const auto scriptInstance = ScriptEngine::GetEntityScriptInstance(entityID);
		ZE_CORE_ASSERT(scriptInstance);
		return scriptInstance->GetMonoInstance();
	}

	static void Entity_GetForwardVector(UUID entityID, Vec3* outForwardVector)
	{
		*outForwardVector = ScriptEngine::GetEntityByID(entityID).GetForwardVector();
	}

	static void Entity_GetRightVector(UUID entityID, Vec3* outForwardVector)
	{
		*outForwardVector = ScriptEngine::GetEntityByID(entityID).GetRightVector();
	}

	static void Entity_GetUpVector(UUID entityID, Vec3* outUpVector)
	{
		*outUpVector = ScriptEngine::GetEntityByID(entityID).GetUpVector();
	}

	static void Entity_LookAt(UUID entityID, Vec3* worldPosition, Vec3* upVector)
	{
		Entity entity = ScriptEngine::GetEntityByID(entityID);
		const Vec3 rotation = Math::FindLookAtRotation(entity.GetWorldTranslation(), *worldPosition, *upVector);
		entity.SetWorldRotation(rotation);
	}

	static void TransformComponent_GetTranslation(UUID entityID, Vec3* outTranslation)
	{
		*outTranslation = ScriptEngine::GetEntityByID(entityID).GetTranslation();
	}

	static void TransformComponent_SetTranslation(UUID entityID, Vec3* translation)
	{
		ScriptEngine::GetEntityByID(entityID).SetTranslation(*translation);
	}

	static void TransformComponent_GetRotation(UUID entityID, Vec3* outRotation)
	{
		*outRotation = ScriptEngine::GetEntityByID(entityID).GetRotation();
	}

	static void TransformComponent_SetRotation(UUID entityID, Vec3* rotation)
	{
		ScriptEngine::GetEntityByID(entityID).SetRotation(*rotation);
	}

	static void TransformComponent_GetScale(UUID entityID, Vec3* outScale)
	{
		*outScale = ScriptEngine::GetEntityByID(entityID).GetScale();
	}

	static void TransformComponent_SetScale(UUID entityID, Vec3* scale)
	{
		ScriptEngine::GetEntityByID(entityID).SetScale(*scale);
	}

	static void WorldTransformComponent_GetTranslation(UUID entityID, Vec3* outTranslation)
	{
		*outTranslation = ScriptEngine::GetEntityByID(entityID).GetWorldTranslation();
	}

	static void WorldTransformComponent_SetTranslation(UUID entityID, Vec3* translation)
	{
		ScriptEngine::GetEntityByID(entityID).SetWorldTranslation(*translation);
	}

	static void WorldTransformComponent_GetRotation(UUID entityID, Vec3* outRotation)
	{
		*outRotation = ScriptEngine::GetEntityByID(entityID).GetWorldRotation();
	}

	static void WorldTransformComponent_SetRotation(UUID entityID, Vec3* rotation)
	{
		ScriptEngine::GetEntityByID(entityID).SetWorldRotation(*rotation);
	}

	static void WorldTransformComponent_GetScale(UUID entityID, Vec3* outScale)
	{
		*outScale = ScriptEngine::GetEntityByID(entityID).GetWorldScale();
	}

	static void WorldTransformComponent_SetScale(UUID entityID, Vec3* scale)
	{
		ScriptEngine::GetEntityByID(entityID).SetWorldScale(*scale);
	}

	static void MeshRendererComponent_GetMeshAsset(UUID entityID, AssetHandle* outMeshAsset)
	{
		*outMeshAsset = ScriptEngine::GetEntityByID(entityID).GetComponent<MeshRendererComponent>().MeshAsset;
	}

	static void MeshRendererComponent_SetMeshAsset(UUID entityID, AssetHandle meshAsset)
	{
		Entity entity = ScriptEngine::GetEntityByID(entityID);
		entity.PatchComponentSingleField<MeshRendererComponent>("MeshAsset"_hs, [meshAsset](MeshRendererComponent& meshComp)
		{
			meshComp.MeshAsset = meshAsset;
		});
	}

	static void* MeshRendererComponent_GetInstance(UUID entityID)
	{
		return ScriptEngine::GetEntityByID(entityID).GetComponent<MeshRendererComponent>().Instance.get();
	}

	static void MeshRendererComponent_GetMaterialAsset(UUID entityID, U32 index, AssetHandle* outMaterialAsset)
	{
		*outMaterialAsset =  ScriptEngine::GetEntityByID(entityID).GetComponent<MeshRendererComponent>().MaterialAssets[index];
	}

	static void MeshRendererComponent_SetMaterialAsset(UUID entityID, U32 index, AssetHandle materialAsset)
	{
		Entity entity = ScriptEngine::GetEntityByID(entityID);
		auto& meshComp = entity.GetComponent<MeshRendererComponent>();
		meshComp.MaterialAssets[index] = materialAsset;
	}

	static void RigidBodyComponent_SetKinematicTarget(UUID entityID, Vec3* targetPosition, Vec3* targetRotation)
	{
		Entity entity = ScriptEngine::GetEntityByID(entityID);
		const auto* physicsScene = ScriptEngine::GetSceneContext().GetPhysicsScene();
		ZE_CORE_ASSERT(physicsScene);
		const auto* actor = physicsScene->GetActor(entity);
		ZE_CORE_ASSERT(actor);
		actor->SetKinematicTarget(*targetPosition, *targetRotation);
	}

	static bool CharacterControllerComponent_IsGrounded(UUID entityID)
	{
		Entity entity = ScriptEngine::GetEntityByID(entityID);
		const auto* physicsScene = ScriptEngine::GetSceneContext().GetPhysicsScene();
		ZE_CORE_ASSERT(physicsScene);
		const auto* controller = physicsScene->GetCharacterController(entity);
		ZE_CORE_ASSERT(controller);
		return controller->IsGrounded();
	}

	static void CharacterControllerComponent_Move(UUID entityID, Vec3* displacement)
	{
		Entity entity = ScriptEngine::GetEntityByID(entityID);
		const auto* physicsScene = ScriptEngine::GetSceneContext().GetPhysicsScene();
		ZE_CORE_ASSERT(physicsScene);
		auto* controller = physicsScene->GetCharacterController(entity);
		ZE_CORE_ASSERT(controller);
		controller->Move(*displacement);
	}

	static float Math_FloatInterpTo(float current, float target, float dt, float interpSpeed)
	{
		return Math::InterpTo(current, target, dt, interpSpeed);
	}

	static void Math_Vector2InterpTo(Vec2* current, Vec2* target, float dt, float interpSpeed, Vec2* outResult)
	{
		*outResult = Math::InterpTo(*current, *target, dt, interpSpeed);
	}

	static void Math_Vector3InterpTo(Vec3* current, Vec3* target, float dt, float interpSpeed, Vec3* outResult)
	{
		*outResult = Math::InterpTo(*current, *target, dt, interpSpeed);
	}

	static float Math_FloatInterpConstantTo(float current, float target, float dt, float interpSpeed)
	{
		return Math::InterpConstantTo(current, target, dt, interpSpeed);
	}

	static void Math_Vector2InterpConstantTo(Vec2* current, Vec2* target, float dt, float interpSpeed, Vec2* outResult)
	{
		*outResult = Math::InterpConstantTo(*current, *target, dt, interpSpeed);
	}

	static void Math_Vector3InterpConstantTo(Vec3* current, Vec3* target, float dt, float interpSpeed, Vec3* outResult)
	{
		*outResult = Math::InterpConstantTo(*current, *target, dt, interpSpeed);
	}

	static bool Input_IsKeyPressed(KeyCode keycode)
	{
		return Input::IsKeyPressed(keycode);
	}

	static bool Input_IsKeyReleased(KeyCode keycode)
	{
		return Input::IsKeyReleased(keycode);
	}

	static bool Input_IsMouseButtonPressed(MouseCode mousecode)
	{
		return Input::IsMouseButtonPressed(mousecode);
	}

	static bool Input_IsMouseButtonReleased(MouseCode mousecode)
	{
		return Input::IsMouseButtonReleased(mousecode);
	}

	static void* AssetLibrary_LoadAssetByPath(MonoString* path, bool bForceLoad)
	{
		char* str = mono_string_to_utf8(path);
		const auto asset = AssetLibrary::LoadAsset<IAsset>(str, bForceLoad).get();
		mono_free(str);
		return asset;
	}

	static void* AssetLibrary_LoadAssetByHandle(AssetHandle handle, bool bForceLoad)
	{
		return AssetLibrary::LoadAsset<IAsset>(handle, bForceLoad).get();
	}

	static MonoString* Asset_GetName(AssetHandle handle)
	{
		const auto* metadata = AssetRegistry::Get().GetAssetMetadata(handle);
		return Utils::StringToMonoString(metadata ? metadata->PathName : "Invalid name");
	}

	static void Asset_GetHandle(void* assetPtr, AssetHandle* handle)
	{
		const auto* asset = static_cast<IAsset*>(assetPtr);
		*handle = asset ? asset->GetHandle() : 0;
	}

	static void Physics_GetGravity(Vec3* outGravity)
	{
		const auto* physicsScene = ScriptEngine::GetSceneContext().GetPhysicsScene();
		ZE_CORE_ASSERT(physicsScene);
		*outGravity = physicsScene->GetGravity();
	}

	static void Physics_SetGravity(Vec3* gravity)
	{
		const auto* physicsScene = ScriptEngine::GetSceneContext().GetPhysicsScene();
		ZE_CORE_ASSERT(physicsScene);
		physicsScene->SetGravity(*gravity);
	}

	enum class LogLevel
	{
		Trace,
		Info,
		Warn,
		Error,
		Critical
	};

	static void Log_LogMessage(LogLevel level, MonoString* message)
	{
		char* msg = mono_string_to_utf8(message);
		switch (level)
		{
			case LogLevel::Trace:		ZE_TRACE(msg); break;
			case LogLevel::Info:		ZE_INFO(msg); break;
			case LogLevel::Warn:		ZE_WARN(msg); break;
			case LogLevel::Error:		ZE_ERROR(msg); break;
			case LogLevel::Critical:	ZE_CRITICAL(msg); break;
		}
		mono_free(msg);
	}

	std::unordered_map<MonoType*, U32> ScriptRegistry::s_RegisteredMonoComponents;
	std::unordered_map<std::string, U32> ScriptRegistry::s_RegisteredMonoComponentNames;

	void ScriptRegistry::RegisterFunctions()
	{
#pragma region Entity
		ZE_ADD_INTERNAL_CALL(Entity_GetName);
		ZE_ADD_INTERNAL_CALL(Entity_HasComponent);
		ZE_ADD_INTERNAL_CALL(Entity_GetEntityByName);
		ZE_ADD_INTERNAL_CALL(GetScriptInstance);
		ZE_ADD_INTERNAL_CALL(Entity_GetForwardVector);
		ZE_ADD_INTERNAL_CALL(Entity_GetRightVector);
		ZE_ADD_INTERNAL_CALL(Entity_GetUpVector);
		ZE_ADD_INTERNAL_CALL(Entity_LookAt);
#pragma endregion

#pragma region TransformComponent
		ZE_ADD_INTERNAL_CALL(TransformComponent_GetTranslation);
		ZE_ADD_INTERNAL_CALL(TransformComponent_SetTranslation);
		ZE_ADD_INTERNAL_CALL(TransformComponent_GetRotation);
		ZE_ADD_INTERNAL_CALL(TransformComponent_SetRotation);
		ZE_ADD_INTERNAL_CALL(TransformComponent_GetScale);
		ZE_ADD_INTERNAL_CALL(TransformComponent_SetScale);
#pragma endregion

#pragma region WorldTransformComponent
		ZE_ADD_INTERNAL_CALL(WorldTransformComponent_GetTranslation);
		ZE_ADD_INTERNAL_CALL(WorldTransformComponent_SetTranslation);
		ZE_ADD_INTERNAL_CALL(WorldTransformComponent_GetRotation);
		ZE_ADD_INTERNAL_CALL(WorldTransformComponent_SetRotation);
		ZE_ADD_INTERNAL_CALL(WorldTransformComponent_GetScale);
		ZE_ADD_INTERNAL_CALL(WorldTransformComponent_SetScale);
#pragma endregion

#pragma region MeshComponent
		ZE_ADD_INTERNAL_CALL(MeshRendererComponent_GetMeshAsset);
		ZE_ADD_INTERNAL_CALL(MeshRendererComponent_SetMeshAsset);
		ZE_ADD_INTERNAL_CALL(MeshRendererComponent_GetInstance);
		ZE_ADD_INTERNAL_CALL(MeshRendererComponent_GetMaterialAsset);
		ZE_ADD_INTERNAL_CALL(MeshRendererComponent_SetMaterialAsset);
#pragma endregion

#pragma region RigidBodyComponent
		ZE_ADD_INTERNAL_CALL(RigidBodyComponent_SetKinematicTarget);
#pragma endregion

#pragma region CharacterControllerComponent
		ZE_ADD_INTERNAL_CALL(CharacterControllerComponent_IsGrounded);
		ZE_ADD_INTERNAL_CALL(CharacterControllerComponent_Move);
#pragma endregion

#pragma region Math
		ZE_ADD_INTERNAL_CALL(Math_FloatInterpTo);
		ZE_ADD_INTERNAL_CALL(Math_Vector2InterpTo);
		ZE_ADD_INTERNAL_CALL(Math_Vector3InterpTo);
		ZE_ADD_INTERNAL_CALL(Math_FloatInterpConstantTo);
		ZE_ADD_INTERNAL_CALL(Math_Vector2InterpConstantTo);
		ZE_ADD_INTERNAL_CALL(Math_Vector3InterpConstantTo);
#pragma endregion

#pragma region Input
		ZE_ADD_INTERNAL_CALL(Input_IsKeyPressed);
		ZE_ADD_INTERNAL_CALL(Input_IsKeyReleased);
		ZE_ADD_INTERNAL_CALL(Input_IsMouseButtonPressed);
		ZE_ADD_INTERNAL_CALL(Input_IsMouseButtonReleased);
#pragma endregion

#pragma region Asset
		ZE_ADD_INTERNAL_CALL(AssetLibrary_LoadAssetByPath);
		ZE_ADD_INTERNAL_CALL(AssetLibrary_LoadAssetByHandle);
		ZE_ADD_INTERNAL_CALL(Asset_GetName);
		ZE_ADD_INTERNAL_CALL(Asset_GetHandle);
#pragma endregion

#pragma region Mesh

#pragma endregion

#pragma region Physics
		ZE_ADD_INTERNAL_CALL(Physics_GetGravity);
		ZE_ADD_INTERNAL_CALL(Physics_SetGravity);
#pragma endregion
		
#pragma region Log
		ZE_ADD_INTERNAL_CALL(Log_LogMessage);
#pragma endregion

	}

	void ScriptRegistry::RegisterMonoComponent(char* monoCompName, U32 compID)
	{
		if (auto* monoType = mono_reflection_type_from_name(monoCompName, ScriptEngine::GetCoreAssemblyImage()))
		{
			s_RegisteredMonoComponents[monoType] = compID;
			s_RegisteredMonoComponentNames[monoCompName] = compID;
		}
	}

	void ScriptRegistry::ReloadMonoComponents()
	{
		s_RegisteredMonoComponents.clear();
		for (const auto& [name, compID] : s_RegisteredMonoComponentNames)
		{
			RegisterMonoComponent(const_cast<char*>(name.c_str()), compID);
		}
	}

	U32 ScriptRegistry::GetComponentIDFromType(MonoReflectionType* compType)
	{
		MonoType* monoCompType = mono_reflection_type_get_type(compType);
		ZE_CORE_ASSERT(ScriptRegistry::s_RegisteredMonoComponents.find(monoCompType) != ScriptRegistry::s_RegisteredMonoComponents.end());
		return s_RegisteredMonoComponents[monoCompType];
	}

}
