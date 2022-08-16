#include "ZEpch.h"
#include "Engine/Scripting/ScriptEngine.h"

#include <fstream>
#include <mono/jit/jit.h>
#include <mono/metadata/assembly.h>

#include "Engine/GameFramework/Components.h"
#include "Engine/Scripting/ScriptRegistry.h"

namespace ZeoEngine {

	namespace Utils {

		// TODO: Move to utils library
		static char* ReadBytes(const std::string& filepath, U32* outSize)
		{
		    std::ifstream stream(filepath, std::ios::binary | std::ios::ate);
		    
		    if (!stream)
		    {
		        // Failed to open the file
		        return nullptr;
		    }

		    std::streampos end = stream.tellg();
		    stream.seekg(0, std::ios::beg);
		    U32 size = static_cast<U32>(end - stream.tellg());
		    
		    if (size == 0)
		    {
		        // File is empty
		        return nullptr;
		    }

		    char* buffer = new char[size];
		    stream.read(buffer, size);
		    stream.close();

		    *outSize = size;
		    return buffer;
		}

		static MonoAssembly* LoadMonoAssembly(const std::string& assemblyPath)
		{
		    U32 fileSize = 0;
		    char* fileData = ReadBytes(assemblyPath, &fileSize);

		    // NOTE: We can't use this image for anything other than loading the assembly because this image doesn't have a reference to the assembly
		    MonoImageOpenStatus status;
		    MonoImage* image = mono_image_open_from_data_full(fileData, fileSize, 1, &status, 0);

		    if (status != MONO_IMAGE_OK)
		    {
		        const char* errorMessage = mono_image_strerror(status);
		        // Log some error message using the errorMessage data
		        return nullptr;
		    }

		    MonoAssembly* assembly = mono_assembly_load_from_full(image, assemblyPath.c_str(), &status, 0);
		    mono_image_close(image);
		    
		    // Don't forget to free the file data
		    delete[] fileData;

		    return assembly;
		}

		static void PrintAssemblyTypes(MonoAssembly* assembly)
		{
		    MonoImage* image = mono_assembly_get_image(assembly);
		    const MonoTableInfo* typeDefinitionsTable = mono_image_get_table_info(image, MONO_TABLE_TYPEDEF);
		    I32 numTypes = mono_table_info_get_rows(typeDefinitionsTable);

		    for (I32 i = 0; i < numTypes; i++)
		    {
		        U32 cols[MONO_TYPEDEF_SIZE];
		        mono_metadata_decode_row(typeDefinitionsTable, i, cols, MONO_TYPEDEF_SIZE);

		        const char* nameSpace = mono_metadata_string_heap(image, cols[MONO_TYPEDEF_NAMESPACE]);
		        const char* name = mono_metadata_string_heap(image, cols[MONO_TYPEDEF_NAME]);

		        ZE_CORE_INFO("{}.{}", nameSpace, name);
		    }
		}

	}

	struct ScriptEngineData
	{
		MonoDomain* RootDomain = nullptr;
		MonoDomain* AppDomain = nullptr;

		MonoAssembly* CoreAssembly = nullptr;
		MonoImage* CoreAssemblyImage = nullptr;

		MonoAssembly* AppAssembly = nullptr;
		MonoImage* AppAssemblyImage = nullptr;

		ScriptClass EntityClass;

		Ref<Scene> SceneContext;

		std::unordered_map<std::string, Ref<ScriptClass>> EntityClasses;
		std::unordered_map<UUID, Ref<ScriptInstance>> EntityInstances; // Map from Entity UUID to script instance
	};

	static ScriptEngineData* s_Data = nullptr;

	void ScriptEngine::Init()
	{
		s_Data = new ScriptEngineData();

		InitMono();
		LoadAssembly("resources/scripts/ZeoEngine-ScriptCore.dll");
		LoadAppAssembly("SandboxProject/Assets/Scripts/Binaries/Sandbox.dll");
		LoadAssemblyClasses();
		ScriptRegistry::RegisterFunctions();
		s_Data->EntityClass = ScriptClass("ZeoEngine", "Entity", true);

#if Test
		Utils::PrintAssemblyTypes(s_Data->CoreAssembly);

		// Create an object and call ctor
		MonoObject* instance = s_Data->EntityClass.Instantiate();

		// Call method
		MonoMethod* printMsgFunc = s_Data->EntityClass.GetMethod("PrintMsg", 0);
		s_Data->EntityClass.InvokeMethod(instance, printMsgFunc);

		// Call method with parameters
		MonoMethod* printIntFunc = s_Data->EntityClass.GetMethod("PrintInt", 1);
		int value = 3;
		void* param = &value;
		s_Data->EntityClass.InvokeMethod(instance, printIntFunc, &param);

		MonoMethod* printIntsFunc = s_Data->EntityClass.GetMethod("PrintInts", 2);
		int value2 = 5;
		void* params[2] =
		{
			&value,
			&value2
		};
		s_Data->EntityClass.InvokeMethod(instance, printIntsFunc, params);

		MonoString* monoString = mono_string_new(s_Data->AppDomain, "Hello World from C++!");
		MonoMethod* printStringFunc =s_Data->EntityClass.GetMethod("PrintString", 1);
		void* stringParam = monoString;
		s_Data->EntityClass.InvokeMethod(instance, printStringFunc, &stringParam);
#endif
	}

	void ScriptEngine::Shutdown()
	{
		ShutdownMono();
		delete s_Data;
	}

	// https://peter1745.github.io/
	void ScriptEngine::InitMono()
	{
		mono_set_assemblies_path("mono/lib");

		MonoDomain* rootDomain = mono_jit_init("ZeoEngineJitRuntime");
	    ZE_CORE_ASSERT(rootDomain);

	    // Store the root domain pointer
	    s_Data->RootDomain = rootDomain;
	}

	void ScriptEngine::ShutdownMono()
	{
		//mono_domain_unload(s_Data->AppDomain);
		s_Data->AppDomain = nullptr;
		//mono_jit_cleanup(s_Data->RootDomain);
		s_Data->RootDomain = nullptr;
	}

	void ScriptEngine::LoadAssembly(const std::string& path)
	{
		// Create an App Domain
	    s_Data->AppDomain = mono_domain_create_appdomain("ZeoEngineScriptRuntime", nullptr);
	    mono_domain_set(s_Data->AppDomain, true);

		s_Data->CoreAssembly = Utils::LoadMonoAssembly(path);
		s_Data->CoreAssemblyImage = mono_assembly_get_image(s_Data->CoreAssembly);
	}

	void ScriptEngine::LoadAppAssembly(const std::string& path)
	{
		s_Data->AppAssembly = Utils::LoadMonoAssembly(path);
		s_Data->AppAssemblyImage = mono_assembly_get_image(s_Data->AppAssembly);
	}

	void ScriptEngine::OnRuntimeStart(Ref<Scene> scene)
	{
		s_Data->SceneContext = std::move(scene);
	}

	void ScriptEngine::OnRuntimeStop()
	{
		s_Data->SceneContext = nullptr;
		s_Data->EntityInstances.clear();
	}

	MonoDomain* ScriptEngine::GetAppDomain()
	{
		return s_Data->AppDomain;
	}

	MonoImage* ScriptEngine::GetCoreAssemblyImage()
	{
		return s_Data->CoreAssemblyImage;
	}

	void ScriptEngine::OnCreateEntity(Entity entity)
	{
		const auto& scriptComp = entity.GetComponent<ScriptComponent>();
		const auto& className = scriptComp.ClassName;
		if (EntityClassExists(className))
		{
			auto instance = CreateRef<ScriptInstance>(s_Data->EntityClasses[className], entity);
			instance->InvokeOnCreate();
			s_Data->EntityInstances[entity.GetUUID()] = std::move(instance);
		}
	}

	void ScriptEngine::OnUpdateEntity(Entity entity, DeltaTime dt)
	{
		const auto uuid = entity.GetUUID();
		ZE_CORE_ASSERT(s_Data->EntityInstances.find(uuid) != s_Data->EntityInstances.end());

		s_Data->EntityInstances[uuid]->InvokeOnUpdate(dt);
	}

	bool ScriptEngine::EntityClassExists(const std::string& fullClassName)
	{
		return s_Data->EntityClasses.find(fullClassName) != s_Data->EntityClasses.end();
	}

	const Ref<Scene>& ScriptEngine::GetSceneContext()
	{
		return s_Data->SceneContext;
	}

	const std::unordered_map<std::string, Ref<ScriptClass>>& ScriptEngine::GetEntityClasses()
	{
		return s_Data->EntityClasses;
	}

	void ScriptEngine::LoadAssemblyClasses()
	{
		s_Data->EntityClasses.clear();

		const MonoTableInfo* typeDefinitionsTable = mono_image_get_table_info(s_Data->AppAssemblyImage, MONO_TABLE_TYPEDEF);
		I32 numTypes = mono_table_info_get_rows(typeDefinitionsTable);
		MonoClass* entityClass = mono_class_from_name(s_Data->CoreAssemblyImage, "ZeoEngine", "Entity");

		for (I32 i = 0; i < numTypes; i++)
		{
			U32 cols[MONO_TYPEDEF_SIZE];
			mono_metadata_decode_row(typeDefinitionsTable, i, cols, MONO_TYPEDEF_SIZE);

			const char* nameSpace = mono_metadata_string_heap(s_Data->AppAssemblyImage, cols[MONO_TYPEDEF_NAMESPACE]);
			const char* name = mono_metadata_string_heap(s_Data->AppAssemblyImage, cols[MONO_TYPEDEF_NAME]);
			std::string fullName = strlen(nameSpace) ? fmt::format("{}.{}", nameSpace, name) : name;

			MonoClass* monoClass = mono_class_from_name(s_Data->AppAssemblyImage, nameSpace, name);
			if (monoClass == entityClass) continue;

			bool bIsEntity = mono_class_is_subclass_of(monoClass, entityClass, false);
			if (bIsEntity)
			{
				s_Data->EntityClasses[fullName] = CreateRef<ScriptClass>(nameSpace, name);
			}
		}
	}

	MonoObject* ScriptEngine::InstantiateClass(MonoClass* monoClass)
	{
		MonoObject* instance = mono_object_new(s_Data->AppDomain, monoClass);
		mono_runtime_object_init(instance);
		return instance;
	}

	ScriptClass::ScriptClass(std::string nameSpace, std::string className, bool bIsCore)
		: m_Namespace(std::move(nameSpace)), m_ClassName(std::move(className))
	{
		m_MonoClass = mono_class_from_name(bIsCore ? s_Data->CoreAssemblyImage : s_Data->AppAssemblyImage, m_Namespace.c_str(), m_ClassName.c_str());
	}

	MonoObject* ScriptClass::Instantiate() const
	{
		return ScriptEngine::InstantiateClass(m_MonoClass);
	}

	MonoMethod* ScriptClass::GetMethod(const std::string& name, int paramCount) const
	{
		return mono_class_get_method_from_name(m_MonoClass, name.c_str(), paramCount);
	}

	MonoObject* ScriptClass::InvokeMethod(MonoObject* instance, MonoMethod* method, void** params) const
	{
		return mono_runtime_invoke(method, instance, params, nullptr);
	}

	ScriptInstance::ScriptInstance(const Ref<ScriptClass>& scriptClass, Entity entity)
		: m_ScriptClass(scriptClass)
	{
		m_Instance = m_ScriptClass->Instantiate();
		// The script class may not have that ctor, so we have to retrieve from Entity class
		m_Constructor = s_Data->EntityClass.GetMethod(".ctor", 1);
		m_OnCreateMethod = m_ScriptClass->GetMethod("OnCreate", 0);
		m_OnUpdateMethod = m_ScriptClass->GetMethod("OnUpdate", 1);

		// Call entity ctor
		auto entityID = entity.GetUUID();
		void* param = &entityID;
		m_ScriptClass->InvokeMethod(m_Instance, m_Constructor, &param);
	}

	void ScriptInstance::InvokeOnCreate() const
	{
		if (m_OnCreateMethod)
		{
			m_ScriptClass->InvokeMethod(m_Instance, m_OnCreateMethod);
		}
	}

	void ScriptInstance::InvokeOnUpdate(float dt) const
	{
		if (m_OnUpdateMethod)
		{
			void* param = &dt;
			m_ScriptClass->InvokeMethod(m_Instance, m_OnUpdateMethod, &param);
		}
	}

}
