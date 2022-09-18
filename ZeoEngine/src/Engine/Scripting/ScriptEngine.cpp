#include "ZEpch.h"
#include "Engine/Scripting/ScriptEngine.h"

#include <fstream>
#include <mono/jit/jit.h>
#include <mono/metadata/assembly.h>
#include <mono/metadata/tabledefs.h>

#include "Engine/GameFramework/Components.h"
#include "Engine/Scripting/ScriptRegistry.h"

namespace ZeoEngine {

	// TODO:
	static std::unordered_map<std::string, ScriptFieldType> s_ScriptFieldTypes =
	{
		{ "System.Boolean", ScriptFieldType::Bool },
		{ "System.Char", ScriptFieldType::Char },
		{ "System.Byte", ScriptFieldType::Byte },
		{ "System.UByte", ScriptFieldType::UByte },
		{ "System.Int16", ScriptFieldType::Short },
		{ "System.UInt16", ScriptFieldType::UShort },
		{ "System.Int32", ScriptFieldType::Int },
		{ "System.UInt32", ScriptFieldType::UInt },
		{ "System.Int64", ScriptFieldType::Long },
		{ "System.UInt64", ScriptFieldType::ULong },
		{ "System.Single", ScriptFieldType::Float },
		{ "System.Double", ScriptFieldType::Double },
		{ "System.Enum", ScriptFieldType::Enum },
		{ "System.String", ScriptFieldType::String },

		{ "ZeoEngine.Vector2", ScriptFieldType::Vector2 },
		{ "ZeoEngine.Vector3", ScriptFieldType::Vector3 },
		{ "ZeoEngine.Vector4", ScriptFieldType::Vector4 },
		{ "ZeoEngine.Texture", ScriptFieldType::Texture },
		{ "ZeoEngine.Particle", ScriptFieldType::Particle },
		{ "ZeoEngine.Mesh", ScriptFieldType::Mesh },
		{ "ZeoEngine.Material", ScriptFieldType::Material },
		{ "ZeoEngine.Shader", ScriptFieldType::Shader },
		{ "ZeoEngine.Entity", ScriptFieldType::Entity }
	};

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

		static ScriptFieldType MonoTypeToScriptFieldType(MonoType* type)
		{
			const std::string typeName = mono_type_get_name(type);
			const auto it = s_ScriptFieldTypes.find(typeName);
			if (it == s_ScriptFieldTypes.end())
			{
				ZE_CORE_ERROR("Unknown type: {}", typeName);
				return ScriptFieldType::None;
			}

			return it->second;
		}

		// TODO:
		static U32 GetScriptFieldSize(ScriptFieldType type)
		{
			switch (type)
			{
				case ScriptFieldType::Bool:			return 1;
				case ScriptFieldType::Float:		return 4;
				case ScriptFieldType::Int:			return 4;
				case ScriptFieldType::UInt:			return 4;
				//case ScriptFieldType::String:		return 8;
				case ScriptFieldType::Vector2:		return 4 * 2;
				case ScriptFieldType::Vector3:		return 4 * 3;
				case ScriptFieldType::Vector4:		return 4 * 4;
				case ScriptFieldType::Entity:		return 8;
			}
			ZE_CORE_ASSERT(false, "Unknown script field type!");
			return 0;
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

		Ref<Scene> SceneContext;

		ScriptClass EntityClass;
		std::unordered_map<std::string, Ref<ScriptClass>> EntityClasses;

		std::unordered_map<UUID, Ref<ScriptInstance>> EntityInstances; // Map from entity UUID to script instance
		std::unordered_map<UUID, ScriptFieldMap> EntityScriptFields; // Map from entity UUID to script field map
		using StringBufferMap = std::unordered_map<std::string, std::string>; // Map from script field name to string buffer
		std::unordered_map<UUID, StringBufferMap> EntityScriptFieldStringBuffer; // Map from entity UUID to string buffer map
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

	void ScriptEngine::InitScriptEntity(Entity entity)
	{
		const auto& scriptComp = entity.GetComponent<ScriptComponent>();
		const auto& className = scriptComp.ClassName;
		if (!EntityClassExists(className)) return;

		const UUID entityID = entity.GetUUID();
		const auto entityClass = GetEntityClass(className);
		// Default construct an instance of the script class
		// We then use this to set initial values for any public fields
		auto scriptInstance = CreateRef<ScriptInstance>(entityClass, entity);
		for (const auto& [name, field] : entityClass->GetFields())
		{
			auto* fieldPtr = const_cast<ScriptField*>(&field);
			ScriptFieldInstance fieldInstance = { fieldPtr };
			fieldInstance.CopyValueFromRuntime(entityID, scriptInstance);
			s_Data->EntityScriptFields[entityID][name] = std::move(fieldInstance);
		}
		s_Data->EntityInstances[entityID] = std::move(scriptInstance);
	}

	void ScriptEngine::InstantiateEntityClass(Entity entity)
	{
		const auto& scriptComp = entity.GetComponent<ScriptComponent>();
		const auto& className = scriptComp.ClassName;
		if (!EntityClassExists(className)) return;

		const UUID entityID = entity.GetUUID();
		const auto entityClass = GetEntityClass(className);
		auto scriptInstance = CreateRef<ScriptInstance>(entityClass, entity);
		for (const auto& [name, field] : entityClass->GetFields())
		{
			const auto& fieldInstance = s_Data->EntityScriptFields[entityID][name];
			fieldInstance.CopyValueToRuntime(scriptInstance);
		}
		s_Data->EntityInstances[entityID] = std::move(scriptInstance);
	}

	void ScriptEngine::OnCreateEntity(Entity entity)
	{
		if (const auto instance = GetEntityScriptInstance(entity.GetUUID()))
		{
			instance->InvokeOnCreate();
		}
	}

	void ScriptEngine::OnUpdateEntity(Entity entity, DeltaTime dt)
	{
		if (const auto instance = GetEntityScriptInstance(entity.GetUUID()))
		{
			instance->InvokeOnUpdate(dt);
		}
	}

	void ScriptEngine::OnDestroyEntity(Entity entity)
	{
		const UUID entityID = entity.GetUUID();
		const auto it = s_Data->EntityInstances.find(entityID);
		if (it != s_Data->EntityInstances.end())
		{
			s_Data->EntityInstances.erase(it);
		}
		const auto it2 = s_Data->EntityScriptFields.find(entityID);
		if (it2 != s_Data->EntityScriptFields.end())
		{
			s_Data->EntityScriptFields.erase(it2);
		}
		const auto it3 = s_Data->EntityScriptFieldStringBuffer.find(entityID);
		if (it3 != s_Data->EntityScriptFieldStringBuffer.end())
		{
			s_Data->EntityScriptFieldStringBuffer.erase(it3);
		}
	}

	bool ScriptEngine::EntityClassExists(const std::string& fullClassName)
	{
		return s_Data->EntityClasses.find(fullClassName) != s_Data->EntityClasses.end();
	}

	const Ref<Scene>& ScriptEngine::GetSceneContext()
	{
		return s_Data->SceneContext;
	}

	Ref<ScriptInstance> ScriptEngine::GetEntityScriptInstance(UUID entityID)
	{
		const auto it = s_Data->EntityInstances.find(entityID);
		if (it == s_Data->EntityInstances.end()) return nullptr;

		return it->second;
	}

	Ref<ScriptClass> ScriptEngine::GetEntityClass(const std::string& fullClassName)
	{
		const auto it = s_Data->EntityClasses.find(fullClassName);
		if (it == s_Data->EntityClasses.end()) return nullptr;

		return it->second;
	}

	const std::unordered_map<std::string, Ref<ScriptClass>>& ScriptEngine::GetEntityClasses()
	{
		return s_Data->EntityClasses;
	}

	ScriptFieldMap& ScriptEngine::GetScriptFieldMap(UUID entityID)
	{
		ZE_CORE_ASSERT(s_Data->EntityScriptFields.find(entityID) != s_Data->EntityScriptFields.end());

		return s_Data->EntityScriptFields[entityID];
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
			if (!bIsEntity) continue;

			Ref<ScriptClass> scriptClass = CreateRef<ScriptClass>(nameSpace, name);
			s_Data->EntityClasses[fullName] = scriptClass;

			void* it = nullptr;
			while (MonoClassField* field = mono_class_get_fields(monoClass, &it))
			{
				std::string fieldName = mono_field_get_name(field);
				const auto fieldFlags = mono_field_get_flags(field);
				if (fieldFlags & FIELD_ATTRIBUTE_PUBLIC)
				{
					const ScriptFieldType fieldType = Utils::MonoTypeToScriptFieldType(mono_field_get_type(field));
					scriptClass->m_Fields[fieldName] = { fieldName, fieldType, field };
				}
			}
		}
	}

	U32 ScriptEngine::InstantiateClass(MonoClass* monoClass)
	{
		MonoObject* instance = mono_object_new(s_Data->AppDomain, monoClass);
		mono_runtime_object_init(instance);
		return mono_gchandle_new(instance, false);
	}

	void ScriptEngine::DestroyClass(U32 handle)
	{
		mono_gchandle_free(handle);
	}

	ScriptFieldInstance::ScriptFieldInstance(ScriptField* field)
		: Field(field)
	{
		ZE_CORE_ASSERT(Field, "Failed to create a script field instance from an emtpy field!");

		// For string, we have a global buffer to store them, so this buffer just points to that global buffer
		if (Field->Type != ScriptFieldType::String)
		{
			AllocateBuffer(Field->Type);
		}
	}

	ScriptFieldInstance::ScriptFieldInstance(const ScriptFieldInstance& other)
		: Field(other.Field)
	{
		if (Field->Type != ScriptFieldType::String)
		{
			AllocateBuffer(Field->Type);
			memcpy(Buffer, other.Buffer, Utils::GetScriptFieldSize(Field->Type));
		}
		else
		{
			Buffer = other.Buffer;
		}
	}

	ScriptFieldInstance::ScriptFieldInstance(ScriptFieldInstance&& other) noexcept
		: Field(other.Field)
		, Buffer(other.Buffer)
	{
		other.Field = nullptr;
		if (Field->Type != ScriptFieldType::String)
		{
			other.Buffer = nullptr;
		}
	}

	ScriptFieldInstance::~ScriptFieldInstance()
	{
		if (Field && Field->Type != ScriptFieldType::String)
		{
			delete[] Buffer;
		}
	}

	ScriptFieldInstance& ScriptFieldInstance::operator=(const ScriptFieldInstance& other)
	{
		if (&other != this)
		{
			if (Field && Field->Type != ScriptFieldType::String)
			{
				delete[] Buffer;
			}

			Field = other.Field;
			if (Field->Type != ScriptFieldType::String)
			{
				AllocateBuffer(Field->Type);
				memcpy(Buffer, other.Buffer, Utils::GetScriptFieldSize(Field->Type));
			}
			else
			{
				Buffer = other.Buffer;
			}
		}

		return *this;
	}

	ScriptFieldInstance& ScriptFieldInstance::operator=(ScriptFieldInstance&& other) noexcept
	{
		if (&other != this)
		{
			if (Field && Field->Type != ScriptFieldType::String)
			{
				delete[] Buffer;
			}

			Field = other.Field;
			Buffer = other.Buffer;

			other.Field = nullptr;
			if (Field->Type != ScriptFieldType::String)
			{
				other.Buffer = nullptr;
			}
		}

		return *this;
	}

	void ScriptFieldInstance::CopyValueFromRuntime(UUID entityID, const Ref<ScriptInstance>& scriptInstance)
	{
		if (Field->Type == ScriptFieldType::String)
		{
			std::string outStr;
			GetRuntimeValueInternal(scriptInstance, outStr);
			auto& stringBuffer = s_Data->EntityScriptFieldStringBuffer[entityID][Field->Name];
			stringBuffer = std::move(outStr);
			Buffer = reinterpret_cast<U8*>(&stringBuffer);
		}
		else
		{
			GetRuntimeValueInternal(scriptInstance, Buffer);
		}
	}

	void ScriptFieldInstance::CopyValueToRuntime(const Ref<ScriptInstance>& scriptInstance) const
	{
		if (Field->Type == ScriptFieldType::String)
		{
			SetRuntimeValueInternal(scriptInstance, *reinterpret_cast<std::string*>(Buffer));
		}
		else
		{
			SetRuntimeValueInternal(scriptInstance, Buffer);
		}
	}

	void ScriptFieldInstance::AllocateBuffer(ScriptFieldType type)
	{
		const U32 size = Utils::GetScriptFieldSize(type);
		Buffer = new U8[size];
		memset(Buffer, 0, size);
	}

	void ScriptFieldInstance::GetValue_Internal(void* outValue) const
	{
		const U32 size = Utils::GetScriptFieldSize(Field->Type);
		memcpy(outValue, Buffer, size);
	}

	void ScriptFieldInstance::SetValue_Internal(const void* value) const
	{
		const U32 size = Utils::GetScriptFieldSize(Field->Type);
		memcpy(Buffer, value, size);
	}

	void ScriptFieldInstance::GetRuntimeValueInternal(const Ref<ScriptInstance>& scriptInstance, void* outValue) const
	{
		mono_field_get_value(scriptInstance->GetInstance(), Field->ClassField, outValue);
	}

	void ScriptFieldInstance::GetRuntimeValueInternal(const Ref<ScriptInstance>& scriptInstance, std::string& outValue) const
	{
		MonoString* monoStr;
		mono_field_get_value(scriptInstance->GetInstance(), Field->ClassField, &monoStr);
		outValue = monoStr != nullptr ? mono_string_to_utf8(monoStr) : "";
	}

	void ScriptFieldInstance::SetRuntimeValueInternal(const Ref<ScriptInstance>& scriptInstance, const void* value) const
	{
		mono_field_set_value(scriptInstance->GetInstance(), Field->ClassField, const_cast<void*>(value));
	}

	void ScriptFieldInstance::SetRuntimeValueInternal(const Ref<ScriptInstance>& scriptInstance, const std::string& value) const
	{
		MonoString* monoStr = mono_string_new(ScriptEngine::GetAppDomain(), value.c_str());
		mono_field_set_value(scriptInstance->GetInstance(), Field->ClassField, monoStr);
	}

	ScriptClass::ScriptClass(std::string nameSpace, std::string className, bool bIsCore)
		: m_Namespace(std::move(nameSpace)), m_ClassName(std::move(className))
	{
		m_MonoClass = mono_class_from_name(bIsCore ? s_Data->CoreAssemblyImage : s_Data->AppAssemblyImage, m_Namespace.c_str(), m_ClassName.c_str());
	}

	U32 ScriptClass::Instantiate() const
	{
		return ScriptEngine::InstantiateClass(m_MonoClass);
	}

	void ScriptClass::Destroy(U32 handle) const
	{
		ScriptEngine::DestroyClass(handle);
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
		m_InstanceHandle = m_ScriptClass->Instantiate();
		// The script class may not have that ctor, so we have to retrieve from Entity class
		m_Constructor = s_Data->EntityClass.GetMethod(".ctor", 1);
		m_OnCreateMethod = m_ScriptClass->GetMethod("OnCreate", 0);
		m_OnUpdateMethod = m_ScriptClass->GetMethod("OnUpdate", 1);

		// Call entity ctor
		auto entityID = entity.GetUUID();
		void* param = &entityID;
		m_ScriptClass->InvokeMethod(GetInstance(), m_Constructor, &param);
	}

	ScriptInstance::~ScriptInstance()
	{
		m_ScriptClass->Destroy(m_InstanceHandle);
		m_InstanceHandle = 0;
	}

	MonoObject* ScriptInstance::GetInstance() const
	{
		ZE_CORE_ASSERT(m_InstanceHandle, "Entity has not been instantiated!");
		return mono_gchandle_get_target(m_InstanceHandle);
	}

	void ScriptInstance::InvokeOnCreate() const
	{
		if (m_OnCreateMethod)
		{
			m_ScriptClass->InvokeMethod(GetInstance(), m_OnCreateMethod);
		}
	}

	void ScriptInstance::InvokeOnUpdate(float dt) const
	{
		if (m_OnUpdateMethod)
		{
			void* param = &dt;
			m_ScriptClass->InvokeMethod(GetInstance(), m_OnUpdateMethod, &param);
		}
	}

}
