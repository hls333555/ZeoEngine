#include "ZEpch.h"
#include "Engine/Scripting/ScriptRegistry.h"

#include <mono/metadata/object.h>

#include "Engine/Scripting/ScriptEngine.h"
#include "Engine/Core/Input.h"
#include "Engine/Core/KeyCodes.h"

namespace ZeoEngine {

#define ZE_ADD_INTERNAL_CALL(Name) mono_add_internal_call("ZeoEngine.InternalCalls::" #Name, Name)

	static void NativeLog(MonoString* str, int param)
	{
		auto* cStr = mono_string_to_utf8(str);
		ZE_CORE_INFO("{0} {1}", cStr, param);
		mono_free(cStr);
	}

	static void NativeLog_Vector(Vec3* param, Vec3* outResult)
	{
		ZE_CORE_INFO("Pos: {0}, {1}, {2}", param->x, param->y, param->z);
		*outResult = glm::cross(*param, Vec3(param->x, param->y, -param->z));
	}

	static float NativeLog_VectorDot(Vec3* param)
	{
		return glm::dot(*param, Vec3(param->x, param->y, -param->z));
	}

	static void Entity_GetTranslation(UUID entityID, Vec3* outTranslation)
	{
		const Ref<Scene>& scene = ScriptEngine::GetSceneContext();
		const Entity entity = scene->GetEntityByUUID(entityID);
		*outTranslation = entity.GetTranslation();
	}

	static void Entity_SetTranslation(UUID entityID, Vec3* translation)
	{
		const Ref<Scene>& scene = ScriptEngine::GetSceneContext();
		Entity entity = scene->GetEntityByUUID(entityID);
		entity.SetTranslation(*translation);
	}

	static void Entity_GetRotation(UUID entityID, Vec3* outRotation)
	{
		const Ref<Scene>& scene = ScriptEngine::GetSceneContext();
		const Entity entity = scene->GetEntityByUUID(entityID);
		*outRotation = entity.GetRotation();
	}

	static void Entity_SetRotation(UUID entityID, Vec3* rotation)
	{
		const Ref<Scene>& scene = ScriptEngine::GetSceneContext();
		Entity entity = scene->GetEntityByUUID(entityID);
		entity.SetRotation(*rotation);
	}

	static void Entity_GetScale(UUID entityID, Vec3* outScale)
	{
		const Ref<Scene>& scene = ScriptEngine::GetSceneContext();
		const Entity entity = scene->GetEntityByUUID(entityID);
		*outScale = entity.GetScale();
	}

	static void Entity_SetScale(UUID entityID, Vec3* scale)
	{
		const Ref<Scene>& scene = ScriptEngine::GetSceneContext();
		Entity entity = scene->GetEntityByUUID(entityID);
		entity.SetScale(*scale);
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
		ZE_ADD_INTERNAL_CALL(NativeLog);
		ZE_ADD_INTERNAL_CALL(NativeLog_Vector);
		ZE_ADD_INTERNAL_CALL(NativeLog_VectorDot);

#pragma region Entity
		ZE_ADD_INTERNAL_CALL(Entity_GetTranslation);
		ZE_ADD_INTERNAL_CALL(Entity_SetTranslation);
		ZE_ADD_INTERNAL_CALL(Entity_GetRotation);
		ZE_ADD_INTERNAL_CALL(Entity_SetRotation);
		ZE_ADD_INTERNAL_CALL(Entity_GetScale);
		ZE_ADD_INTERNAL_CALL(Entity_SetScale);
#pragma endregion

#pragma region Input
		ZE_ADD_INTERNAL_CALL(Input_IsKeyPressed);
		ZE_ADD_INTERNAL_CALL(Input_IsKeyReleased);
		ZE_ADD_INTERNAL_CALL(Input_IsMouseButtonPressed);
		ZE_ADD_INTERNAL_CALL(Input_IsMouseButtonReleased);
#pragma endregion

	}

}
