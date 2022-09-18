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

	enum class ScriptFieldType
	{
		None = 0,
		Bool, Char, Byte, UByte, Short, UShort, Int, UInt, Long, ULong,
		Float, Double,
		Enum, String,
		Vector2, Vector3, Vector4,
		Texture, Particle, Mesh, Material, Shader,
		Entity,
	};

	using ScriptFieldMap = std::unordered_map<std::string, struct ScriptFieldInstance>; // Map from field name to script field instance

	class ScriptEngine
	{
		friend class ScriptClass;

	public:
		static void Init();
		static void Shutdown();

		static void LoadAssembly(const std::string& path);
		static void LoadAppAssembly(const std::string& path);

		static void OnRuntimeStart(Ref<Scene> scene);
		static void OnRuntimeStop();

		static MonoDomain* GetAppDomain();
		static MonoImage* GetCoreAssemblyImage();

		static const Ref<Scene>& GetSceneContext();

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

	private:
		static void InitMono();
		static void ShutdownMono();

		static void LoadAssemblyClasses();
		static U32 InstantiateClass(MonoClass* monoClass);
		static void DestroyClass(U32 handle);
	};

	struct ScriptField
	{
		std::string Name;
		ScriptFieldType Type;
		MonoClassField* ClassField = nullptr;
	};

	struct ScriptFieldInstance
	{
		ScriptField* Field = nullptr;

		ScriptFieldInstance() = default;
		ScriptFieldInstance(ScriptField* field);
		ScriptFieldInstance(const ScriptFieldInstance& other);
		ScriptFieldInstance(ScriptFieldInstance&& other) noexcept;
		~ScriptFieldInstance();

		ScriptFieldInstance& operator=(const ScriptFieldInstance& other);
		ScriptFieldInstance& operator=(ScriptFieldInstance&& other) noexcept;

		template<typename T>
		T GetValue() const
		{
			T value;
			GetValue_Internal(&value);
			return value;
		}

		template<>
		const std::string& GetValue() const
		{
			return *reinterpret_cast<std::string*>(Buffer);
		}

		template<typename T>
		void SetValue(T value)
		{
			SetValue_Internal(&value);
		}

		template<>
		void SetValue(const std::string& value)
		{
			(*reinterpret_cast<std::string*>(Buffer)).assign(value);
		}

		template<typename T>
		T GetRuntimeValue(const Ref<ScriptInstance>& scriptInstance) const
		{
			T value;
			GetRuntimeValueInternal(scriptInstance, &value);
			return value;
		}

		template<>
		std::string GetRuntimeValue(const Ref<ScriptInstance>& scriptInstance) const
		{
			std::string value;
			GetRuntimeValueInternal(scriptInstance, value);
			return value;
		}

		template<typename T>
		void SetRuntimeValue(const Ref<ScriptInstance>& scriptInstance, T value)
		{
			SetRuntimeValueInternal(scriptInstance, &value);
		}

		template<>
		void SetRuntimeValue(const Ref<ScriptInstance>& scriptInstance, const std::string& value)
		{
			SetRuntimeValueInternal(scriptInstance, value);
		}

		void CopyValueFromRuntime(UUID entityID, const Ref<ScriptInstance>& scriptInstance);
		void CopyValueToRuntime(const Ref<ScriptInstance>& scriptInstance) const;

	private:
		void AllocateBuffer(ScriptFieldType type);

		void GetValue_Internal(void* outValue) const;
		void SetValue_Internal(const void* value) const;
		void GetRuntimeValueInternal(const Ref<ScriptInstance>& scriptInstance, void* outValue) const;
		void GetRuntimeValueInternal(const Ref<ScriptInstance>& scriptInstance, std::string& outValue) const;
		void SetRuntimeValueInternal(const Ref<ScriptInstance>& scriptInstance, const void* value) const;
		void SetRuntimeValueInternal(const Ref<ScriptInstance>& scriptInstance, const std::string& value) const;

	private:
		U8* Buffer = nullptr;
	};

	class ScriptClass
	{
		friend class ScriptEngine;

	public:
		ScriptClass() = default;
		ScriptClass(std::string nameSpace, std::string className, bool bIsCore = false);

		const auto& GetFields() const { return m_Fields; }

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
		friend struct ScriptFieldInstance;

	public:
		ScriptInstance(const Ref<ScriptClass>& scriptClass, Entity entity);
		~ScriptInstance();

		Ref<ScriptClass> GetScriptClass() const { return m_ScriptClass; }
		MonoObject* GetInstance() const;

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
