#pragma once

#include "Engine/GameFramework/Entity.h"

extern "C" {
	typedef struct _MonoClass MonoClass;
	typedef struct _MonoObject MonoObject;
	typedef struct _MonoMethod MonoMethod;
	typedef struct _MonoAssembly MonoAssembly;
}

namespace ZeoEngine {

	class ScriptClass;
	class Scene;

	class ScriptEngine
	{
		friend class ScriptClass;

	public:
		static void Init();
		static void Shutdown();

		static void LoadAssembly(const std::string& path);

		static void OnRuntimeStart(Ref<Scene> scene);
		static void OnRuntimeStop();

		static void OnCreateEntity(Entity entity);
		static void OnUpdateEntity(Entity entity, DeltaTime dt);
		static bool EntityClassExists(const std::string& fullClassName);

		static const Ref<Scene>& GetSceneContext();
		static const std::unordered_map<std::string, Ref<ScriptClass>>& GetEntityClasses();

	private:
		static void InitMono();
		static void ShutdownMono();

		static void LoadAssemblyClasses(MonoAssembly* assembly);
		static MonoObject* InstantiateClass(MonoClass* monoClass);
	};

	class ScriptClass
	{
	public:
		ScriptClass() = default;
		ScriptClass(std::string nameSpace, std::string className);

		MonoObject* Instantiate() const;
		MonoMethod* GetMethod(const std::string& name, int paramCount) const;
		MonoObject* InvokeMethod(MonoObject* instance, MonoMethod* method, void** params = nullptr) const;

	private:
		std::string m_Namespace;
		std::string m_ClassName;
		MonoClass* m_MonoClass = nullptr;
	};

	class ScriptInstance
	{
	public:
		ScriptInstance(const Ref<ScriptClass>& scriptClass, Entity entity);

		void InvokeOnCreate() const;
		void InvokeOnUpdate(float dt) const;

	private:
		Ref<ScriptClass> m_ScriptClass;

		MonoObject* m_Instance = nullptr;
		MonoMethod* m_Constructor = nullptr;
		MonoMethod* m_OnCreateMethod = nullptr;
		MonoMethod* m_OnUpdateMethod = nullptr;
	};
	
}
