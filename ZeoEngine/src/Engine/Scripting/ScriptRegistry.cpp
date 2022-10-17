#include "ZEpch.h"
#include "Engine/Scripting/ScriptRegistry.h"

#include <mono/metadata/object.h>
#include <mono/metadata/reflection.h>

#include "Engine/Scripting/ScriptEngine.h"
#include "Engine/Core/Input.h"
#include "Engine/Core/KeyCodes.h"
#include "Engine/Asset/AssetLibrary.h"
#include "Engine/GameFramework/Components.h"

namespace ZeoEngine {

	namespace Utils {

		static MonoString* StringToMonoString(const std::string& str)
		{
			return mono_string_new(ScriptEngine::GetAppDomain(), str.c_str());
		}
		
	}

#define ZE_ADD_INTERNAL_CALL(Name) mono_add_internal_call("ZeoEngine.InternalCalls::" #Name, Name)
#define EXPAND_PARAM_TYPE(...) "(" #__VA_ARGS__ ")"
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

	static AssetHandle MeshRendererComponent_GetMeshAsset(UUID entityID)
	{
		return ScriptEngine::GetEntityByID(entityID).GetComponent<MeshRendererComponent>().MeshAsset;
	}

	static void MeshRendererComponent_SetMeshAsset(UUID entityID, AssetHandle meshAsset)
	{
		// TODO:
		Entity entity = ScriptEngine::GetEntityByID(entityID);
		auto& meshComp = entity.GetComponent<MeshRendererComponent>();
		meshComp.MeshAsset = meshAsset;
		entity.UpdateBounds();
		const auto mesh = meshComp.GetMesh();
		meshComp.Instance = mesh ? mesh->CreateInstance(entity.GetScene()) : nullptr;
	}

	static void* MeshRendererComponent_GetInstance(UUID entityID)
	{
		return ScriptEngine::GetEntityByID(entityID).GetComponent<MeshRendererComponent>().Instance.get();
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

	static void* AssetLibrary_LoadAssetByPath(MonoString* path)
	{
		char* str = mono_string_to_utf8(path);
		const auto asset = AssetLibrary::LoadAsset<IAsset>(str).get();
		mono_free(str);
		return asset;
	}

	static void* AssetLibrary_LoadAssetByHandle(AssetHandle handle)
	{
		return AssetLibrary::LoadAsset<IAsset>(handle).get();
	}

	static MonoString* Asset_GetName(AssetHandle handle)
	{
		const auto metadata = AssetRegistry::Get().GetAssetMetadata(handle);
		return Utils::StringToMonoString(metadata ? metadata->PathName : "Invalid name");
	}

	static void Asset_GetHandle(void* assetPtr, AssetHandle* handle)
	{
		const auto* asset = static_cast<IAsset*>(assetPtr);
		*handle = asset ? asset->GetHandle() : 0;
	}

	static U64 MeshInstance_GetMaterial(void* meshInstance, U32 index)
	{
		const auto* instance = static_cast<MeshInstance*>(meshInstance);
		return instance ? instance->GetMaterial(index) : 0;
	}

	static void MeshInstance_SetMaterial(void* meshInstance, U32 index, AssetHandle materialAsset)
	{
		auto* instance = static_cast<MeshInstance*>(meshInstance);
		if (!instance) return;

		instance->SetMaterial(index, materialAsset);
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
#pragma endregion

#pragma region TransformComponent
		ZE_ADD_INTERNAL_CALL(TransformComponent_GetTranslation);
		ZE_ADD_INTERNAL_CALL(TransformComponent_SetTranslation);
		ZE_ADD_INTERNAL_CALL(TransformComponent_GetRotation);
		ZE_ADD_INTERNAL_CALL(TransformComponent_SetRotation);
		ZE_ADD_INTERNAL_CALL(TransformComponent_GetScale);
		ZE_ADD_INTERNAL_CALL(TransformComponent_SetScale);
#pragma endregion

#pragma region MeshComponent
		ZE_ADD_INTERNAL_CALL(MeshRendererComponent_GetMeshAsset);
		ZE_ADD_INTERNAL_CALL(MeshRendererComponent_SetMeshAsset);
		ZE_ADD_INTERNAL_CALL(MeshRendererComponent_GetInstance);
#pragma endregion

#pragma region Input
		ZE_ADD_INTERNAL_CALL(Input_IsKeyPressed);
		ZE_ADD_INTERNAL_CALL(Input_IsKeyReleased);
		ZE_ADD_INTERNAL_CALL(Input_IsMouseButtonPressed);
		ZE_ADD_INTERNAL_CALL(Input_IsMouseButtonReleased);
#pragma endregion

#pragma region Asset
		ZE_ADD_INTERNAL_CALL_OVERLOAD(AssetLibrary_LoadAsset, AssetLibrary_LoadAssetByPath, string);
		ZE_ADD_INTERNAL_CALL_OVERLOAD(AssetLibrary_LoadAsset, AssetLibrary_LoadAssetByHandle, ulong);
		ZE_ADD_INTERNAL_CALL(Asset_GetName);
		ZE_ADD_INTERNAL_CALL(Asset_GetHandle);
#pragma endregion

#pragma region Mesh
		ZE_ADD_INTERNAL_CALL(MeshInstance_GetMaterial);
		ZE_ADD_INTERNAL_CALL(MeshInstance_SetMaterial);
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
