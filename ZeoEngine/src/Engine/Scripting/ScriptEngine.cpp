#include "ZEpch.h"
#include "Engine/Scripting/ScriptEngine.h"

#include <fstream>
#include <mono/jit/jit.h>
#include <mono/metadata/assembly.h>
#include <mono/metadata/tabledefs.h>
#include <mono/metadata/mono-debug.h>
#include <mono/metadata/threads.h>

#include "Engine/Core/Buffer.h"
#include "Engine/Core/CommonPaths.h"
#include "Engine/Core/Project.h"
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

		static MonoAssembly* LoadMonoAssembly(const std::string& assemblyPath, bool bLoadPDB = false)
		{
		    ScopedBuffer fileData = FileSystemUtils::ReadFileBinary(assemblyPath);

		    // NOTE: We can't use this image for anything other than loading the assembly because this image doesn't have a reference to the assembly
		    MonoImageOpenStatus status;
		    MonoImage* image = mono_image_open_from_data_full(fileData.As<char>(), static_cast<U32>(fileData.Size()), 1, &status, 0);

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
					const std::string pdbPathStr = pdbPath.generic_string();
					ScopedBuffer pdbFileData = FileSystemUtils::ReadFileBinary(pdbPathStr);
					mono_debug_open_image_from_memory(image, pdbFileData.As<const mono_byte>(), static_cast<int>(pdbFileData.Size()));
					//ZE_CORE_INFO("Loaded debug symbol file '{}'", pdbPathStr);
				}
			}
#endif

		    MonoAssembly* assembly = mono_assembly_load_from_full(image, assemblyPath.c_str(), &status, 0);
		    mono_image_close(image);

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
		bool bReloadSubmitted = false;

		MonoAssembly* CoreAssembly = nullptr;
		MonoImage* CoreAssemblyImage = nullptr;

		MonoAssembly* AppAssembly = nullptr;
		MonoImage* AppAssemblyImage = nullptr;

#ifdef ZE_DEBUG
		bool bEnableDebugging = true;
#else
		bool bEnableDebugging = false;
#endif

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

		Project::GetProjectLoadedDelegate().connect<&ScriptEngine::LoadProjectAssembly>();
		InitMono();
		ScriptRegistry::RegisterFunctions();
		if (!LoadCoreAssembly(CommonPaths::GetCoreAssemblyPath())) return;

		ZE_CORE_TRACE("Script engine intialized");
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

		MonoDomain* rootDomain = mono_jit_init("ZeoEngineJitRuntime");
	    ZE_CORE_ASSERT(rootDomain);

	    // Store the root domain pointer
	    s_Data->RootDomain = rootDomain;

		if (s_Data->bEnableDebugging)
		{
			mono_debug_domain_create(s_Data->RootDomain);
		}

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

	void ScriptEngine::LoadProjectAssembly()
	{
		if (LoadAppAssembly(CommonPaths::GetAppAssemblyPath()))
		{
			LoadAssemblyClasses();
		}
	}

	bool ScriptEngine::LoadCoreAssembly(const std::string& path)
	{
		// Create an App Domain
	    s_Data->AppDomain = mono_domain_create_appdomain("ZeoEngineScriptRuntime", nullptr);
	    mono_domain_set(s_Data->AppDomain, true);

		s_Data->CoreAssemblyPath = path;
		s_Data->CoreAssembly = Utils::LoadMonoAssembly(path, s_Data->bEnableDebugging);
		if (!s_Data->CoreAssembly)
		{
			ZE_CORE_ERROR("Could not load ZeoEngine-ScriptCore assembly! Please make sure C# core dll is built correctly.");
			return false;
		}

		s_Data->CoreAssemblyImage = mono_assembly_get_image(s_Data->CoreAssembly);
		return true;
	}

	bool ScriptEngine::LoadAppAssembly(const std::string& path)
	{
		s_Data->AppAssemblyPath = path;
		s_Data->AppAssembly = Utils::LoadMonoAssembly(path, s_Data->bEnableDebugging);
		if (!s_Data->AppAssembly)
		{
			ZE_CORE_ERROR("Could not load app assembly! Please make sure C# app dll is built correctly.");
			return false;
		}

		s_Data->AppAssemblyImage = mono_assembly_get_image(s_Data->AppAssembly);
		return true;
	}

	void ScriptEngine::LoadAssemblyClasses()
	{
		if (!s_Data->CoreAssemblyImage || !s_Data->AppAssemblyImage)
		{
			ZE_CORE_ERROR("Failed to load assembly classes!");
			return;
		}

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

		s_Data->EntityClass = ScriptClass("ZeoEngine", "Entity", true);
	}

	void ScriptEngine::OnAssemblyChanged(const std::string& path)
	{
		if (FileSystemUtils::GetCanonicalPath(path) == FileSystemUtils::GetCanonicalPath(s_Data->CoreAssemblyPath) ||
			FileSystemUtils::GetCanonicalPath(path) == FileSystemUtils::GetCanonicalPath(s_Data->AppAssemblyPath))
		{
			if (!s_Data->bReloadSubmitted)
			{
				Application::Get().SubmitToMainThread([]()
				{
					ReloadAssembly();
				});
				s_Data->bReloadSubmitted = true;
			}
		}
	}

	void ScriptEngine::ReloadAssembly()
	{
		s_Data->bReloadSubmitted = false;

		Timer timer;
		{
			mono_domain_set(mono_get_root_domain(), false);
			mono_domain_unload(s_Data->AppDomain);

			const bool bCoreLoaded = LoadCoreAssembly(s_Data->CoreAssemblyPath);
			const bool bAppLoaded = LoadAppAssembly(s_Data->AppAssemblyPath);
			if (!bCoreLoaded || !bAppLoaded) return;

			LoadAssemblyClasses();

			ScriptRegistry::ReloadMonoComponents();

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
		const UUID entityID = entity.GetUUID();
		s_Data->EntityInstances[entityID].reset();
		s_Data->EntityScriptFields[entityID].clear();
		s_Data->EntityScriptFieldStringBuffer[entityID].clear();

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
		// TODO: Should overwrite non-modified values
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
				fieldInstance->m_Field = fieldPtr;
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
		if (!EntityClassExists(className))
		{
			ZE_CORE_ERROR("Could not find script class: '{}' to instantiate!", className);
			return;
		}

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

	void ScriptEngine::OnCollisionBegin(Entity entity, const CollisionInfo& collisionInfo)
	{
		if (const auto instance = GetEntityScriptInstance(entity.GetUUID()))
		{
			instance->InvokeOnCollisionBegin(collisionInfo);
		}
	}

	void ScriptEngine::OnCollisionEnd(Entity entity, const CollisionInfo& collisionInfo)
	{
		if (const auto instance = GetEntityScriptInstance(entity.GetUUID()))
		{
			instance->InvokeOnCollisionEnd(collisionInfo);
		}
	}

	void ScriptEngine::OnTriggerBegin(Entity entity, Entity otherEntity)
	{
		if (const auto instance = GetEntityScriptInstance(entity.GetUUID()))
		{
			instance->InvokeOnTriggerBegin(otherEntity);
		}
	}

	void ScriptEngine::OnTriggerEnd(Entity entity, Entity otherEntity)
	{
		if (const auto instance = GetEntityScriptInstance(entity.GetUUID()))
		{
			instance->InvokeOnTriggerEnd(otherEntity);
		}
	}

	void ScriptEngine::OnCharacterControllerCollide(Entity entity, const CharacterControllerHit& collisionInfo)
	{
		if (const auto instance = GetEntityScriptInstance(entity.GetUUID()))
		{
			instance->InvokeOnCharacterControllerCollide(collisionInfo);
		}
	}

	bool ScriptEngine::IsEntityScriptClassValid(Entity entity)
	{
		return entity.HasComponent<ScriptComponent>() && EntityClassExists(entity.GetComponent<ScriptComponent>().ClassName);
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
		, m_Field(field)
		, m_EntityID(entityID)
	{
		ZE_CORE_ASSERT(m_Field, "Failed to create a script field instance from an emtpy field!");

		// For string, we have a global buffer to store them, so our buffer just points to that global buffer
		if (GetFieldType() != FieldType::String)
		{
			const U32 fieldSize = GetFieldSize();
			m_Buffer = Buffer(fieldSize);
			m_RuntimeBuffer = Buffer(fieldSize);
		}
	}

	ScriptFieldInstance::ScriptFieldInstance(const ScriptFieldInstance& other)
		: FieldInstanceBase(other)
		, m_Field(other.m_Field)
		, m_EntityID(other.m_EntityID)
	{
		if (GetFieldType() != FieldType::String)
		{
			m_Buffer = Buffer::Copy(other.m_Buffer);
			m_RuntimeBuffer = Buffer::Copy(other.m_RuntimeBuffer);
		}
		else
		{
			m_Buffer = other.m_Buffer;
			m_RuntimeBuffer = other.m_RuntimeBuffer;
		}
	}

	ScriptFieldInstance::ScriptFieldInstance(ScriptFieldInstance&& other) noexcept
		: FieldInstanceBase(std::move(other))
		, m_Field(other.m_Field)
		, m_EntityID(other.m_EntityID)
	{
		other.m_Field = nullptr;
		other.m_EntityID = 0;

		if (GetFieldType() == FieldType::String)
		{
			m_Buffer = other.m_Buffer;
			m_RuntimeBuffer = other.m_RuntimeBuffer;
			other.m_Buffer = {};
			other.m_RuntimeBuffer = {};
		}
		else
		{
			m_Buffer = std::move(other.m_Buffer);
			m_RuntimeBuffer = std::move(other.m_RuntimeBuffer);
		}
	}

	ScriptFieldInstance::~ScriptFieldInstance()
	{
		if (GetFieldType() != FieldType::String)
		{
			m_Buffer.Release();
			m_RuntimeBuffer.Release();
		}
	}

	ScriptFieldInstance& ScriptFieldInstance::operator=(const ScriptFieldInstance& other)
	{
		FieldInstanceBase::operator=(other);

		if (&other != this)
		{
			if (GetFieldType() != FieldType::String)
			{
				m_Buffer.Release();
				m_RuntimeBuffer.Release();
			}

			m_Field = other.m_Field;
			m_EntityID = other.m_EntityID;
			if (GetFieldType() != FieldType::String)
			{
				m_Buffer = Buffer::Copy(other.m_Buffer);
				m_RuntimeBuffer = Buffer::Copy(other.m_RuntimeBuffer);
			}
			else
			{
				m_Buffer = other.m_Buffer;
				m_RuntimeBuffer = other.m_RuntimeBuffer;
			}
		}

		return *this;
	}

	ScriptFieldInstance& ScriptFieldInstance::operator=(ScriptFieldInstance&& other) noexcept
	{
		FieldInstanceBase::operator=(std::move(other));

		if (&other != this)
		{
			m_Field = other.m_Field;
			m_EntityID = other.m_EntityID;

			other.m_Field = nullptr;
			other.m_EntityID = 0;

			if (GetFieldType() == FieldType::String)
			{
				m_Buffer = other.m_Buffer;
				m_RuntimeBuffer = other.m_RuntimeBuffer;
				other.m_Buffer = {};
				other.m_RuntimeBuffer = {};
			}
			else
			{
				m_Buffer = std::move(other.m_Buffer);
				m_RuntimeBuffer = std::move(other.m_RuntimeBuffer);
			}
		}

		return *this;
	}

	const char* ScriptFieldInstance::GetFieldName() const
	{
		return m_Field->Name.c_str();
	}

	void* ScriptFieldInstance::GetValueRaw() const
	{
		if (SceneUtils::IsLevelRuntime())
		{
			GetRuntimeValueInternal(m_RuntimeBuffer.Data);
			return m_RuntimeBuffer.Data;
		}
		return m_Buffer.Data;
	}

	void ScriptFieldInstance::SetValueRaw(const void* value) const
	{
		if (SceneUtils::IsLevelRuntime())
		{
			SetRuntimeValueInternal(value);
		}
		else
		{
			memcpy(m_Buffer.Data, value, GetFieldSize());
		}
	}

	void ScriptFieldInstance::CopyValueFromRuntime()
	{
		if (GetFieldType() == FieldType::String)
		{
			std::string outStr;
			GetRuntimeValueInternal(outStr);
			auto& stringBuffer = s_Data->EntityScriptFieldStringBuffer[m_EntityID][m_Field->Name];
			stringBuffer = std::move(outStr);
			m_Buffer.Data = reinterpret_cast<U8*>(&stringBuffer);
		}
		else
		{
			GetRuntimeValueInternal(m_Buffer.Data);
		}
	}

	void ScriptFieldInstance::CopyValueToRuntime() const
	{
		if (GetFieldType() == FieldType::String)
		{
			SetRuntimeValueInternal(*m_Buffer.As<std::string>());
		}
		else
		{
			SetRuntimeValueInternal(m_Buffer.Data);
		}
	}

	void ScriptFieldInstance::GetValue_Internal(void* outValue) const
	{
		memcpy(outValue, m_Buffer.Data, GetFieldSize());
	}

	void ScriptFieldInstance::SetValue_Internal(const void* value) const
	{
		memcpy(m_Buffer.Data, value, GetFieldSize());
	}

	void ScriptFieldInstance::GetRuntimeValueInternal(void* outValue) const
	{
		const auto scriptInstance = ScriptEngine::GetEntityScriptInstance(m_EntityID);
		if (GetFieldType() == FieldType::Entity)
		{
			// Get managed object (C# class)
			MonoObject* object = nullptr;
			mono_field_get_value(scriptInstance->GetMonoInstance(), m_Field->ClassField, &object);
			if (object) // Entity may have not been assigned (not newed yet)
			{
				// Get ID field
				const auto* idField = s_Data->EntityClass.GetField("ID");
				mono_field_get_value(object, idField->ClassField, outValue);
			}
		}
		else
		{
			mono_field_get_value(scriptInstance->GetMonoInstance(), m_Field->ClassField, outValue);
		}
	}

	void ScriptFieldInstance::GetRuntimeValueInternal(std::string& outValue) const
	{
		const auto scriptInstance = ScriptEngine::GetEntityScriptInstance(m_EntityID);
		MonoString* monoStr;
		mono_field_get_value(scriptInstance->GetMonoInstance(), m_Field->ClassField, &monoStr);
		char* str = mono_string_to_utf8(monoStr);
		outValue = monoStr != nullptr ? str : "";
		mono_free(str);
	}

	void ScriptFieldInstance::SetRuntimeValueInternal(const void* value) const
	{
		const auto scriptInstance = ScriptEngine::GetEntityScriptInstance(m_EntityID);
		if (GetFieldType() == FieldType::Entity)
		{
			// Create Managed Object
			const U32 objectHandle = s_Data->EntityClass.Instantiate();
			auto* object = ScriptEngine::GetMonoInstanceFromHandle(objectHandle);
			auto* ctor = s_Data->EntityClass.GetMethod(".ctor", 1);
			void* params[] = { const_cast<void*>(value) };
			s_Data->EntityClass.InvokeMethod(object, ctor, params);

			mono_field_set_value(scriptInstance->GetMonoInstance(), m_Field->ClassField, object);
		}
		else
		{
			mono_field_set_value(scriptInstance->GetMonoInstance(), m_Field->ClassField, const_cast<void*>(value));
		}
	}

	void ScriptFieldInstance::SetRuntimeValueInternal(const std::string& value) const
	{
		const auto scriptInstance = ScriptEngine::GetEntityScriptInstance(m_EntityID);
		MonoString* monoStr = mono_string_new(ScriptEngine::GetAppDomain(), value.c_str());
		mono_field_set_value(scriptInstance->GetMonoInstance(), m_Field->ClassField, monoStr);
	}

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
		m_OnCollisionBeginMethod = m_ScriptClass->GetMethod("OnCollisionBegin", 1);
		m_OnCollisionEndMethod = m_ScriptClass->GetMethod("OnCollisionEnd", 1);
		m_OnTriggerBeginMethod = m_ScriptClass->GetMethod("OnTriggerBegin", 1);
		m_OnTriggerEndMethod = m_ScriptClass->GetMethod("OnTriggerEnd", 1);
		m_OnCharacterControllerCollideMethod = m_ScriptClass->GetMethod("OnCharacterControllerCollide", 1);

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

	void ScriptInstance::InvokeOnCollisionBegin(const CollisionInfo& collisionInfo) const
	{
		if (m_OnCollisionBeginMethod)
		{
			void* param = nullptr; // TODO:
			ZE_CORE_ASSERT(false);
			m_ScriptClass->InvokeMethod(GetMonoInstance(), m_OnCollisionBeginMethod, &param);
		}
	}

	void ScriptInstance::InvokeOnCollisionEnd(const CollisionInfo& collisionInfo) const
	{
		if (m_OnCollisionEndMethod)
		{
			void* param = nullptr; // TODO:
			ZE_CORE_ASSERT(false); // TODO:
			m_ScriptClass->InvokeMethod(GetMonoInstance(), m_OnCollisionEndMethod, &param);
		}
	}

	void ScriptInstance::InvokeOnTriggerBegin(Entity otherEntity) const
	{
		if (m_OnTriggerBeginMethod)
		{
			UUID otherEntityID = otherEntity.GetUUID();
			void* param = &otherEntityID;
			m_ScriptClass->InvokeMethod(GetMonoInstance(), m_OnTriggerBeginMethod, &param);
		}
	}

	void ScriptInstance::InvokeOnTriggerEnd(Entity otherEntity) const
	{
		if (m_OnTriggerEndMethod)
		{
			UUID otherEntityID = otherEntity.GetUUID();
			void* param = &otherEntityID;
			m_ScriptClass->InvokeMethod(GetMonoInstance(), m_OnTriggerEndMethod, &param);
		}
	}

	void ScriptInstance::InvokeOnCharacterControllerCollide(const CharacterControllerHit& collisionInfo) const
	{
		if (m_OnCharacterControllerCollideMethod)
		{
			void* param = nullptr; // TODO:
			ZE_CORE_ASSERT(false);
			m_ScriptClass->InvokeMethod(GetMonoInstance(), m_OnCharacterControllerCollideMethod, &param);
		}
	}

}
