#pragma once

#include "Engine/GameFramework/Entity.h"

extern "C" {
	typedef struct _MonoClass MonoClass;
	typedef struct _MonoObject MonoObject;
	typedef struct _MonoMethod MonoMethod;
	typedef struct _MonoAssembly MonoAssembly;
	typedef struct _MonoImage MonoImage;
	typedef struct _MonoDomain MonoDomain;
	typedef struct _MonoClassField MonoClassField;
}

namespace ZeoEngine {

	class ScriptClass;
	class ScriptInstance;
	class Scene;
	struct IComponent;

	using ScriptFieldMap = std::unordered_map<std::string, Ref<class ScriptFieldInstance>>; // Map from field name to script field instance

	class ScriptEngine
	{
		friend class ScriptClass;

	public:
		static void Init();
		static void Shutdown();

		static void LoadCoreAssembly(const std::string& path);
		static void LoadAppAssembly(const std::string& path);
		static void OnFileModified(const std::string& path);
		static void ReloadAssembly();
		static entt::sink<entt::sigh<void()>>* GetScriptReloadedDelegate();

		static MonoDomain* GetAppDomain();
		static MonoImage* GetCoreAssemblyImage();

		static Scene& GetSceneContext();
		static void SetSceneContext(Ref<Scene> scene);

		static MonoObject* GetMonoInstanceFromHandle(U32 handle);

		static void OnScriptClassChanged(Entity entity);
		static void InitScriptEntity(Entity entity);
		static void InstantiateEntityClass(Entity entity);
		static void OnCreateEntity(Entity entity);
		static void OnUpdateEntity(Entity entity, DeltaTime dt);
		static void OnDestroyEntity(Entity entity);

		static bool EntityClassExists(const std::string& fullClassName);
		static Ref<ScriptInstance> GetEntityScriptInstance(UUID entityID);
		static Ref<ScriptClass> GetEntityClass(const std::string& fullClassName);
		static const std::unordered_map<std::string, Ref<ScriptClass>>& GetEntityClasses();
		static ScriptFieldMap& GetScriptFieldMap(UUID entityID);

		static Entity GetEntityByID(UUID entityID);
		static Entity GetEntityByName(std::string_view name);

	private:
		static void InitMono();
		static void ShutdownMono();

		static void LoadAssemblyClasses();
		static U32 InstantiateClass(MonoClass* monoClass);
		static void DestroyClass(U32 handle);
		static void ClearEntityCacheData();
	};

	struct ScriptField
	{
		std::string Name;
		FieldType Type;
		MonoClassField* ClassField = nullptr;
	};

	class ScriptClass
	{
		friend class ScriptEngine;

	public:
		ScriptClass() = default;
		ScriptClass(std::string nameSpace, std::string className, bool bIsCore = false);

		const auto& GetFields() const { return m_Fields; }
		const ScriptField* GetField(const std::string& name) const;

		U32 Instantiate() const;
		void Destroy(U32 handle) const;
		MonoMethod* GetMethod(const std::string& name, int paramCount) const;
		MonoObject* InvokeMethod(MonoObject* instance, MonoMethod* method, void** params = nullptr) const;

	private:
		std::string m_Namespace;
		std::string m_ClassName;

		std::unordered_map<std::string, ScriptField> m_Fields;

		MonoClass* m_MonoClass = nullptr;
	};

	class ScriptInstance
	{
		friend class ScriptFieldInstance;

	public:
		ScriptInstance(const Ref<ScriptClass>& scriptClass, Entity entity);
		~ScriptInstance();

		Ref<ScriptClass> GetScriptClass() const { return m_ScriptClass; }
		MonoObject* GetMonoInstance() const;

		void InvokeOnCreate() const;
		void InvokeOnUpdate(float dt) const;

	private:
		Ref<ScriptClass> m_ScriptClass;

		U32 m_InstanceHandle = 0; // Mono object handle to prevent from being gc
		MonoMethod* m_Constructor = nullptr;
		MonoMethod* m_OnCreateMethod = nullptr;
		MonoMethod* m_OnUpdateMethod = nullptr;
	};
	
}
