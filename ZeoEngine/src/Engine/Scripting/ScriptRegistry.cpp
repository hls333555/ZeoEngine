#include "ZEpch.h"
#include "Engine/Scripting/ScriptRegistry.h"

#include <mono/metadata/object.h>
#include <mono/metadata/reflection.h>

#include "Engine/Scripting/ScriptEngine.h"
#include "Engine/Core/Input.h"
#include "Engine/Core/KeyCodes.h"

namespace ZeoEngine {

	namespace Utils {

		static U32 ComponentTypeToID(MonoReflectionType* compType)
		{
			MonoType* monoCompType = mono_reflection_type_get_type(compType);
			ZE_CORE_ASSERT(ScriptRegistry::s_RegisteredMonoComponents.find(monoCompType) != ScriptRegistry::s_RegisteredMonoComponents.end());
			return ScriptRegistry::s_RegisteredMonoComponents[monoCompType];
		}

		static MonoString* StringToMonoString(const std::string& str)
		{
			return mono_string_new(ScriptEngine::GetAppDomain(), str.c_str());
		}
		
	}

#define ZE_ADD_INTERNAL_CALL(Name) mono_add_internal_call("ZeoEngine.InternalCalls::" #Name, Name)

	std::unordered_map<MonoType*, U32> ScriptRegistry::s_RegisteredMonoComponents;

	static bool Entity_HasComponent(UUID entityID, MonoReflectionType* compType)
	{
		return ScriptEngine::GetEntityByID(entityID).HasComponentByID(Utils::ComponentTypeToID(compType));
	}

	static MonoString* Entity_GetName(UUID entityID)
	{
		return Utils::StringToMonoString(ScriptEngine::GetEntityByID(entityID).GetName());
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

	void ScriptRegistry::RegisterFunctions()
	{
#pragma region Entity
		ZE_ADD_INTERNAL_CALL(Entity_GetName);
		ZE_ADD_INTERNAL_CALL(Entity_GetForwardVector);
		ZE_ADD_INTERNAL_CALL(Entity_GetRightVector);
		ZE_ADD_INTERNAL_CALL(Entity_GetUpVector);
		ZE_ADD_INTERNAL_CALL(Entity_HasComponent);
#pragma endregion

#pragma region TransformComponent
		ZE_ADD_INTERNAL_CALL(TransformComponent_GetTranslation);
		ZE_ADD_INTERNAL_CALL(TransformComponent_SetTranslation);
		ZE_ADD_INTERNAL_CALL(TransformComponent_GetRotation);
		ZE_ADD_INTERNAL_CALL(TransformComponent_SetRotation);
		ZE_ADD_INTERNAL_CALL(TransformComponent_GetScale);
		ZE_ADD_INTERNAL_CALL(TransformComponent_SetScale);
#pragma endregion

#pragma region Input
		ZE_ADD_INTERNAL_CALL(Input_IsKeyPressed);
		ZE_ADD_INTERNAL_CALL(Input_IsKeyReleased);
		ZE_ADD_INTERNAL_CALL(Input_IsMouseButtonPressed);
		ZE_ADD_INTERNAL_CALL(Input_IsMouseButtonReleased);
#pragma endregion

	}

}
