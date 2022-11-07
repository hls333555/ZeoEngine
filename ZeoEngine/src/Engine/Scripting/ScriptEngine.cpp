#include "ZEpch.h"
#include "Engine/Scripting/ScriptEngine.h"

#include <fstream>
#include <mono/jit/jit.h>
#include <mono/metadata/assembly.h>
#include <mono/metadata/tabledefs.h>
#include <mono/metadata/mono-debug.h>
#include <mono/metadata/threads.h>

#include "Engine/GameFramework/Components.h"
#include "Engine/Scripting/ScriptRegistry.h"
#include "Engine/Utils/EngineUtils.h"
#include "Engine/Scripting/ScriptFieldInstance.h"

namespace ZeoEngine {

	static std::unordered_map<std::string, FieldType> s_ScriptFieldTypes =
	{
		{ "System.Boolean", FieldType::Bool },
		{ "System.Char", FieldType::I8 },
		{ "System.Byte", FieldType::U8 },
		{ "System.Int16", FieldType::I16 },
		{ "System.UInt16", FieldType::U16 },
		{ "System.Int32", FieldType::I32 },
		{ "System.UInt32", FieldType::U32 },
		{ "System.Int64", FieldType::I64 },
		{ "System.UInt64", FieldType::U64 },
		{ "System.Single", FieldType::Float },
		{ "System.Double", FieldType::Double },
		{ "System.Enum", FieldType::Enum },
		{ "System.String", FieldType::String },

		{ "ZeoEngine.Vector2", FieldType::Vec2 },
		{ "ZeoEngine.Vector3", FieldType::Vec3 },
		{ "ZeoEngine.Vector4", FieldType::Vec4 },
		{ "ZeoEngine.AssetHandle", FieldType::Asset },
		{ "ZeoEngine.Entity", FieldType::Entity }
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

		static MonoAssembly* LoadMonoAssembly(const std::string& assemblyPath, bool bLoadPDB = false)
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

#ifdef ZE_DEBUG
			if (bLoadPDB)
			{
				std::filesystem::path pdbPath = assemblyPath;
				pdbPath.replace_extension(".pdb");
				if (std::filesystem::exists(pdbPath))
				{
					U32 pdbFileSize = 0;
					const char* pdbFileData = ReadBytes(pdbPath.string(), &pdbFileSize);
					mono_debug_open_image_from_memory(image, reinterpret_cast<const mono_byte*>(pdbFileData), pdbFileSize);
					ZE_CORE_INFO("Loaded PDB {}", pdbPath);
					delete[] pdbFileData;
				}
			}
#endif

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

		static FieldType MonoTypeToFieldType(MonoType* type)
		{
			const std::string typeName = mono_type_get_name(type);
			const auto it = s_ScriptFieldTypes.find(typeName);
			if (it == s_ScriptFieldTypes.end())
			{
				ZE_CORE_ERROR("Unknown type: {}", typeName);
				return FieldType::None;
			}

			return it->second;
		}

	}

	struct ScriptEngineData
	{
		MonoDomain* RootDomain = nullptr;
		MonoDomain* AppDomain = nullptr;

		std::string CoreAssemblyPath;
		std::string AppAssemblyPath;
		entt::sink<entt::sigh<void()>> OnScriptReloaded{ OnScriptReloadedDel };
		entt::sigh<void()> OnScriptReloadedDel;

		MonoAssembly* CoreAssembly = nullptr;
		MonoImage* CoreAssemblyImage = nullptr;

		MonoAssembly* AppAssembly = nullptr;
		MonoImage* AppAssemblyImage = nullptr;

		// TODO: Configuable
		bool bEnableDebugging = true;

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
		LoadCoreAssembly("resources/scripts/ZeoEngine-ScriptCore.dll");
		const std::string path = fmt::format("{}/Scripts/Binaries/Sandbox.dll", AssetRegistry::GetProjectAssetDirectory());
		LoadAppAssembly(path);
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
		s_Data = nullptr;
	}

	// https://peter1745.github.io/
	void ScriptEngine::InitMono()
	{
		mono_set_assemblies_path("mono/lib");

#ifdef ZE_DEBUG // TODO: Enable debugging will crash in release build, don't know why...
		// Init debugger before jit init
		if (s_Data->bEnableDebugging)
		{
			constexpr int argc = 2;
			const char* argv[argc] = {
				"--debugger-agent=transport=dt_socket,address=127.0.0.1:2550,server=y,suspend=n,loglevel=3,logfile=MonoDebugger.log",
				"--soft-breakpoints"
			};
			mono_jit_parse_options(argc, const_cast<char**>(argv));
			mono_debug_init(MONO_DEBUG_FORMAT_MONO);
		}
#endif

		MonoDomain* rootDomain = mono_jit_init("ZeoEngineJitRuntime");
	    ZE_CORE_ASSERT(rootDomain);

	    // Store the root domain pointer
	    s_Data->RootDomain = rootDomain;

#ifdef ZE_DEBUG
		if (s_Data->bEnableDebugging)
		{
			mono_debug_domain_create(s_Data->RootDomain);
		}
#endif

		mono_thread_set_main(mono_thread_current());
	}

	void ScriptEngine::ShutdownMono()
	{
		mono_domain_set(mono_get_root_domain(), false);
		mono_domain_unload(s_Data->AppDomain);
		s_Data->AppDomain = nullptr;
		mono_jit_cleanup(s_Data->RootDomain);
		s_Data->RootDomain = nullptr;

		s_Data->SceneContext = nullptr;
		ClearEntityCacheData();
	}

	void ScriptEngine::LoadCoreAssembly(const std::string& path)
	{
		// Create an App Domain
	    s_Data->AppDomain = mono_domain_create_appdomain("ZeoEngineScriptRuntime", nullptr);
	    mono_domain_set(s_Data->AppDomain, true);

		s_Data->CoreAssemblyPath = path;
		s_Data->CoreAssembly = Utils::LoadMonoAssembly(path, s_Data->bEnableDebugging);
		s_Data->CoreAssemblyImage = mono_assembly_get_image(s_Data->CoreAssembly);
	}

	void ScriptEngine::LoadAppAssembly(const std::string& path)
	{
		s_Data->AppAssemblyPath = path;
		s_Data->AppAssembly = Utils::LoadMonoAssembly(path, s_Data->bEnableDebugging);
		s_Data->AppAssemblyImage = mono_assembly_get_image(s_Data->AppAssembly);
	}

	void ScriptEngine::OnFileModified(const std::string& path)
	{
		if (PathUtils::GetCanonicalPath(path) == PathUtils::GetCanonicalPath(s_Data->CoreAssemblyPath) ||
			PathUtils::GetCanonicalPath(path) == PathUtils::GetCanonicalPath(s_Data->AppAssemblyPath))
		{
			Application::Get().SubmitToMainThread([]()
			{
				ReloadAssembly();
			});
		}
	}

	void ScriptEngine::ReloadAssembly()
	{
		Timer timer;
		{
			mono_domain_set(mono_get_root_domain(), false);
			mono_domain_unload(s_Data->AppDomain);

			LoadCoreAssembly(s_Data->CoreAssemblyPath);
			LoadAppAssembly(s_Data->AppAssemblyPath);

			ScriptRegistry::ReloadMonoComponents();

			LoadAssemblyClasses();
			s_Data->EntityClass = ScriptClass("ZeoEngine", "Entity", true);

			std::vector<UUID> entities;
			for (const auto& pair : s_Data->EntityInstances)
			{
				entities.emplace_back(pair.first);
			}
			for (const UUID entityID : entities)
			{
				InitScriptEntity(GetEntityByID(entityID));
			}

			s_Data->OnScriptReloadedDel.publish();
		}
		ZE_CORE_WARN("Reloading scripts took {} ms", timer.ElapsedMillis());
	}

	entt::sink<entt::sigh<void()>>* ScriptEngine::GetScriptReloadedDelegate()
	{
		return s_Data ? &s_Data->OnScriptReloaded : nullptr;
	}

	MonoDomain* ScriptEngine::GetAppDomain()
	{
		return s_Data->AppDomain;
	}

	MonoImage* ScriptEngine::GetCoreAssemblyImage()
	{
		return s_Data->CoreAssemblyImage;
	}

	Scene& ScriptEngine::GetSceneContext()
	{
		return *s_Data->SceneContext;
	}

	void ScriptEngine::SetSceneContext(Ref<Scene> scene)
	{
		s_Data->SceneContext = std::move(scene);
	}

	MonoObject* ScriptEngine::GetMonoInstanceFromHandle(U32 handle)
	{
		return mono_gchandle_get_target(handle);
	}

	void ScriptEngine::OnScriptClassChanged(Entity entity)
	{
		s_Data->EntityInstances.clear();
		s_Data->EntityScriptFields.clear();
		s_Data->EntityScriptFieldStringBuffer.clear();

		InitScriptEntity(entity);
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
		s_Data->EntityInstances[entityID] = CreateRef<ScriptInstance>(entityClass, entity);

		auto& scriptFields = s_Data->EntityScriptFields[entityID];
		// Save old fields so that reloading does not break modified values
		std::unordered_map<std::string, Ref<ScriptFieldInstance>> oldFieldInstances;
		for (auto& [name, fieldInstance] : scriptFields)
		{
			oldFieldInstances[name] = std::move(fieldInstance);
		}

		scriptFields.clear();
		for (const auto& [name, field] : entityClass->GetFields())
		{
			auto& fieldInstance = scriptFields[name];

			// Apply saved fields
			auto* fieldPtr = const_cast<ScriptField*>(&field);
			const auto it = oldFieldInstances.find(name);
			if (it != oldFieldInstances.end() && field.Type == it->second->GetFieldType())
			{
				fieldInstance = std::move(it->second);
				fieldInstance->Field = fieldPtr;
				continue;
			}

			// Create a new field instance
			fieldInstance = CreateRef<ScriptFieldInstance>(fieldPtr, entityID);
			fieldInstance->CopyValueFromRuntime();
		}
	}

	void ScriptEngine::InstantiateEntityClass(Entity entity)
	{
		const auto& scriptComp = entity.GetComponent<ScriptComponent>();
		const auto& className = scriptComp.ClassName;
		if (!EntityClassExists(className)) return;

		const UUID entityID = entity.GetUUID();
		const auto entityClass = GetEntityClass(className);
		s_Data->EntityInstances[entityID] = CreateRef<ScriptInstance>(entityClass, entity);
		for (const auto& [name, field] : entityClass->GetFields())
		{
			const auto& fieldInstance = s_Data->EntityScriptFields[entityID][name];
			fieldInstance->CopyValueToRuntime();
		}
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

	Entity ScriptEngine::GetEntityByID(UUID entityID)
	{
		const Entity entity = GetSceneContext().GetEntityByUUID(entityID);
		ZE_CORE_ASSERT(entity);
		return entity;
	}

	Entity ScriptEngine::GetEntityByName(std::string_view name)
	{
		return GetSceneContext().GetEntityByName(name);
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

			s_Data->EntityClasses[fullName] = CreateRef<ScriptClass>(nameSpace, name);
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

	void ScriptEngine::ClearEntityCacheData()
	{
		s_Data->EntityInstances.clear();
		s_Data->EntityScriptFields.clear();
		s_Data->EntityScriptFieldStringBuffer.clear();
	}

	ScriptFieldInstance::ScriptFieldInstance(ScriptField* field, UUID entityID)
		: FieldInstanceBase(field->Type)
		, Field(field)
		, EntityID(entityID)
	{
		ZE_CORE_ASSERT(Field, "Failed to create a script field instance from an emtpy field!");

		// For string, we have a global buffer to store them, so our buffer just points to that global buffer
		if (GetFieldType() != FieldType::String)
		{
			Buffer = EngineUtils::AllocateFieldBuffer(GetFieldType());
			RuntimeBuffer = EngineUtils::AllocateFieldBuffer(GetFieldType());
		}
	}

	ScriptFieldInstance::ScriptFieldInstance(const ScriptFieldInstance& other)
		: FieldInstanceBase(other)
		, Field(other.Field)
		, EntityID(other.EntityID)
	{
		if (GetFieldType() != FieldType::String)
		{
			Buffer = EngineUtils::AllocateFieldBuffer(GetFieldType());
			memcpy(Buffer, other.Buffer, EngineUtils::GetFieldSize(GetFieldType()));
			RuntimeBuffer = EngineUtils::AllocateFieldBuffer(GetFieldType());
			memcpy(RuntimeBuffer, other.RuntimeBuffer, EngineUtils::GetFieldSize(GetFieldType()));
		}
		else
		{
			Buffer = other.Buffer;
			RuntimeBuffer = other.RuntimeBuffer;
		}
	}

	ScriptFieldInstance::ScriptFieldInstance(ScriptFieldInstance&& other) noexcept
		: FieldInstanceBase(std::move(other))
		, Field(other.Field)
		, EntityID(other.EntityID)
		, Buffer(other.Buffer)
		, RuntimeBuffer(other.RuntimeBuffer)
	{
		other.Field = nullptr;
		if (GetFieldType() != FieldType::String)
		{
			other.Buffer = nullptr;
			other.RuntimeBuffer = nullptr;
		}
	}

	ScriptFieldInstance::~ScriptFieldInstance()
	{
		if (GetFieldType() != FieldType::String)
		{
			EngineUtils::FreeFieldBuffer(Buffer);
			EngineUtils::FreeFieldBuffer(RuntimeBuffer);
		}
	}

	ScriptFieldInstance& ScriptFieldInstance::operator=(const ScriptFieldInstance& other)
	{
		FieldInstanceBase::operator=(other);

		if (&other != this)
		{
			if (GetFieldType() != FieldType::String)
			{
				EngineUtils::FreeFieldBuffer(Buffer);
				EngineUtils::FreeFieldBuffer(RuntimeBuffer);
			}

			Field = other.Field;
			EntityID = other.EntityID;
			if (GetFieldType() != FieldType::String)
			{
				Buffer = EngineUtils::AllocateFieldBuffer(GetFieldType());
				memcpy(Buffer, other.Buffer, EngineUtils::GetFieldSize(GetFieldType()));
				RuntimeBuffer = EngineUtils::AllocateFieldBuffer(GetFieldType());
				memcpy(RuntimeBuffer, other.RuntimeBuffer, EngineUtils::GetFieldSize(GetFieldType()));
			}
			else
			{
				Buffer = other.Buffer;
				RuntimeBuffer = other.RuntimeBuffer;
			}
		}

		return *this;
	}

	ScriptFieldInstance& ScriptFieldInstance::operator=(ScriptFieldInstance&& other) noexcept
	{
		FieldInstanceBase::operator=(std::move(other));

		if (&other != this)
		{
			if (GetFieldType() != FieldType::String)
			{
				EngineUtils::FreeFieldBuffer(Buffer);
				EngineUtils::FreeFieldBuffer(RuntimeBuffer);
			}

			Field = other.Field;
			EntityID = other.EntityID;
			Buffer = other.Buffer;
			RuntimeBuffer = other.RuntimeBuffer;

			other.Field = nullptr;
			if (GetFieldType() != FieldType::String)
			{
				other.Buffer = nullptr;
				other.RuntimeBuffer = nullptr;
			}
		}

		return *this;
	}

	const char* ScriptFieldInstance::GetFieldName() const
	{
		return Field->Name.c_str();
	}

	void* ScriptFieldInstance::GetValueRaw() const
	{
		if (SceneUtils::IsLevelRuntime())
		{
			GetRuntimeValueInternal(RuntimeBuffer);
			return RuntimeBuffer;
		}
		return Buffer;
	}

	void ScriptFieldInstance::SetValueRaw(const void* value) const
	{
		if (SceneUtils::IsLevelRuntime())
		{
			SetRuntimeValueInternal(value);
		}
		else
		{
			const U32 size = EngineUtils::GetFieldSize(GetFieldType());
			memcpy(Buffer, value, size);
		}
	}

	void ScriptFieldInstance::CopyValueFromRuntime()
	{
		if (GetFieldType() == FieldType::String)
		{
			std::string outStr;
			GetRuntimeValueInternal(outStr);
			auto& stringBuffer = s_Data->EntityScriptFieldStringBuffer[EntityID][Field->Name];
			stringBuffer = std::move(outStr);
			Buffer = reinterpret_cast<U8*>(&stringBuffer);
		}
		//else if (GetFieldType() == FieldType::Asset)
		//{
		//	GetRuntimeValueInternal(reinterpret_cast<AssetHandle*>(Buffer));
		//}
		else
		{
			GetRuntimeValueInternal(Buffer);
		}
	}

	void ScriptFieldInstance::CopyValueToRuntime() const
	{
		if (GetFieldType() == FieldType::String)
		{
			SetRuntimeValueInternal(*reinterpret_cast<std::string*>(Buffer));
		}
		//else if (GetFieldType() == FieldType::Asset)
		//{
		//	SetRuntimeValueInternal(reinterpret_cast<AssetHandle*>(Buffer));
		//}
		else
		{
			SetRuntimeValueInternal(Buffer);
		}
	}

	void ScriptFieldInstance::GetValue_Internal(void* outValue) const
	{
		const U32 size = EngineUtils::GetFieldSize(GetFieldType());
		memcpy(outValue, Buffer, size);
	}

	void ScriptFieldInstance::SetValue_Internal(const void* value) const
	{
		const U32 size = EngineUtils::GetFieldSize(GetFieldType());
		memcpy(Buffer, value, size);
	}

	void ScriptFieldInstance::GetRuntimeValueInternal(void* outValue) const
	{
		const auto scriptInstance = ScriptEngine::GetEntityScriptInstance(EntityID);
		mono_field_get_value(scriptInstance->GetMonoInstance(), Field->ClassField, outValue);
	}

	void ScriptFieldInstance::GetRuntimeValueInternal(std::string& outValue) const
	{
		const auto scriptInstance = ScriptEngine::GetEntityScriptInstance(EntityID);
		MonoString* monoStr;
		mono_field_get_value(scriptInstance->GetMonoInstance(), Field->ClassField, &monoStr);
		char* str = mono_string_to_utf8(monoStr);
		outValue = monoStr != nullptr ? str : "";
		mono_free(str);
	}

	//void ScriptFieldInstance::GetRuntimeValueInternal(AssetHandle* outValue) const
	//{
	//	const auto scriptInstance = ScriptEngine::GetEntityScriptInstance(EntityID);
	//	// Get managed object (C# class)
	//	MonoObject* object = nullptr;
	//	mono_field_get_value(scriptInstance->GetMonoInstance(), Field->ClassField, &object);
	//	const auto clazz = ScriptClass("ZeoEngine", "AssetHandle", true);
	//	// Get ID field
	//	const auto* idField = clazz.GetField("ID");
	//	mono_field_get_value(object, idField->ClassField, outValue);
	//}

	void ScriptFieldInstance::SetRuntimeValueInternal(const void* value) const
	{
		const auto scriptInstance = ScriptEngine::GetEntityScriptInstance(EntityID);
		mono_field_set_value(scriptInstance->GetMonoInstance(), Field->ClassField, const_cast<void*>(value));
	}

	void ScriptFieldInstance::SetRuntimeValueInternal(const std::string& value) const
	{
		const auto scriptInstance = ScriptEngine::GetEntityScriptInstance(EntityID);
		MonoString* monoStr = mono_string_new(ScriptEngine::GetAppDomain(), value.c_str());
		mono_field_set_value(scriptInstance->GetMonoInstance(), Field->ClassField, monoStr);
	}

	//void ScriptFieldInstance::SetRuntimeValueInternal(const AssetHandle* value) const
	//{
	//	// Create Managed Object
	//	const auto clazz = ScriptClass("ZeoEngine", "AssetHandle", true);
	//	const U32 objectHandle = clazz.Instantiate();
	//	auto* object = ScriptEngine::GetMonoInstanceFromHandle(objectHandle);
	//	auto* ctor = clazz.GetMethod(".ctor", 1);
	//	void* params[] = { const_cast<AssetHandle*>(value) };
	//	clazz.InvokeMethod(object, ctor, params);

	//	const auto scriptInstance = ScriptEngine::GetEntityScriptInstance(EntityID);
	//	mono_field_set_value(scriptInstance->GetMonoInstance(), Field->ClassField, object);
	//}

	ScriptClass::ScriptClass(std::string nameSpace, std::string className, bool bIsCore)
		: m_Namespace(std::move(nameSpace)), m_ClassName(std::move(className))
	{
		m_MonoClass = mono_class_from_name(bIsCore ? s_Data->CoreAssemblyImage : s_Data->AppAssemblyImage, m_Namespace.c_str(), m_ClassName.c_str());

		// TODO: Gather fields
		void* it = nullptr;
		while (MonoClassField* field = mono_class_get_fields(m_MonoClass, &it))
		{
			std::string fieldName = mono_field_get_name(field);
			const auto fieldFlags = mono_field_get_flags(field);
			if (fieldFlags & FIELD_ATTRIBUTE_PUBLIC)
			{
				const FieldType fieldType = Utils::MonoTypeToFieldType(mono_field_get_type(field));
				m_Fields[fieldName] = { fieldName, fieldType, field };
			}
		}
	}

	const ScriptField* ScriptClass::GetField(const std::string& name) const
	{
		const auto it = m_Fields.find(name);
		if (it == m_Fields.end()) return nullptr;

		return &it->second;
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
		MonoObject* exception = nullptr;
		return mono_runtime_invoke(method, instance, params, &exception);
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
		m_ScriptClass->InvokeMethod(GetMonoInstance(), m_Constructor, &param);
	}

	ScriptInstance::~ScriptInstance()
	{
		m_ScriptClass->Destroy(m_InstanceHandle);
		m_InstanceHandle = 0;
	}

	MonoObject* ScriptInstance::GetMonoInstance() const
	{
		ZE_CORE_ASSERT(m_InstanceHandle, "Entity has not been instantiated!");
		return ScriptEngine::GetMonoInstanceFromHandle(m_InstanceHandle);
	}

	void ScriptInstance::InvokeOnCreate() const
	{
		if (m_OnCreateMethod)
		{
			m_ScriptClass->InvokeMethod(GetMonoInstance(), m_OnCreateMethod);
		}
	}

	void ScriptInstance::InvokeOnUpdate(float dt) const
	{
		if (m_OnUpdateMethod)
		{
			void* param = &dt;
			m_ScriptClass->InvokeMethod(GetMonoInstance(), m_OnUpdateMethod, &param);
		}
	}

}
