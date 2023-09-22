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
#include "Engine/Profile/BenchmarkTimer.h"

namespace ZeoEngine {

	static std::unordered_map<std::string, FieldType> s_ScriptFieldCustomTypes =
	{
		{ "ZeoEngine.Vector2",		FieldType::Vec2 },
		{ "ZeoEngine.Vector3",		FieldType::Vec3 },
		{ "ZeoEngine.Vector4",		FieldType::Vec4 },
		{ "ZeoEngine.AssetHandle",	FieldType::Asset },
		{ "ZeoEngine.Entity",		FieldType::Entity }
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

		static bool IsListType(MonoType* type)
		{
			auto* typeUtilsClass = mono_class_from_name(ScriptEngine::GetCoreAssemblyImage(), "ZeoEngine", "TypeUtils");
			auto* method = mono_class_get_method_from_name(typeUtilsClass, "IsListType", 1);
			MonoObject* exception = nullptr;
			auto* reflectionType = mono_type_get_object(mono_domain_get(), type);
			void* param = reflectionType;
			auto* res = mono_runtime_invoke(method, nullptr, &param, &exception);
			return *static_cast<bool*>(mono_object_unbox(res));
		}

		static FieldType MonoTypeToFieldType(MonoType* type)
		{
			const std::string typeName = mono_type_get_name(type);
			switch (mono_type_get_type(type))
			{
				case MONO_TYPE_BOOLEAN:
					return FieldType::Bool; // bool
				case MONO_TYPE_I1:
					return FieldType::I8; // sbyte
				case MONO_TYPE_I2:
					return FieldType::I16; // short
				case MONO_TYPE_I4:
					return FieldType::I32; // int
				case MONO_TYPE_I8:
					return FieldType::I64; // long
				case MONO_TYPE_U1:
					return FieldType::U8; // byte
				case MONO_TYPE_U2:
					return FieldType::U16; // ushort
				case MONO_TYPE_U4:
					return FieldType::U32; // uint
				case MONO_TYPE_U8:
					return FieldType::U64; // ulong
				case MONO_TYPE_R4:
					return FieldType::Float; // float
				case MONO_TYPE_R8:
					return FieldType::Double; // double
				case MONO_TYPE_ENUM:
					ZE_CORE_ASSERT(false); // TODO:
					return FieldType::Enum; // enum
				case MONO_TYPE_STRING:
					return FieldType::String; // string
				case MONO_TYPE_SZARRAY:
					ZE_CORE_ASSERT(false); // TODO:
					return FieldType::SeqCon; // One-dimensional array
				case MONO_TYPE_GENERICINST:
					if (IsListType(type))
					{
						return FieldType::SeqCon; // List
					}
			}

			const auto it = s_ScriptFieldCustomTypes.find(typeName);
			if (it == s_ScriptFieldCustomTypes.end())
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
		using ContainerStringBufferMap = std::unordered_map<std::string, std::vector<std::string>>; // Map from script field name to list of string buffers
		std::unordered_map<UUID, StringBufferMap> EntityScriptFieldStringBuffer; // Map from entity UUID to string buffer map
		std::unordered_map<UUID, ContainerStringBufferMap> EntityScriptFieldContainerStringBuffer; // Map from entity UUID to container string buffer map

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
			ScriptRegistry::ReloadMonoAssets();

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
		s_Data->EntityScriptFieldContainerStringBuffer[entityID].clear();

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
				fieldInstance->OnReload(fieldPtr);
				continue;
			}

			// Create a new field instance
			fieldInstance = fieldPtr->Type == FieldType::SeqCon ? CreateRef<ScriptSequenceContainerFieldInstance>(fieldPtr, entityID) : CreateRef<ScriptFieldInstance>(fieldPtr, entityID);
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
		ZE_PROFILE_FUNC();

		if (const auto instance = GetEntityScriptInstance(entity.GetUUID()))
		{
			instance->InvokeOnCreate();
		}
	}

	void ScriptEngine::OnUpdateEntity(Entity entity, DeltaTime dt)
	{
		ZE_PROFILE_FUNC();

		if (const auto instance = GetEntityScriptInstance(entity.GetUUID()))
		{
			instance->InvokeOnUpdate(dt);
		}
	}

	void ScriptEngine::OnDestroyEntity(Entity entity)
	{
		ZE_PROFILE_FUNC();

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
		const auto it4 = s_Data->EntityScriptFieldContainerStringBuffer.find(entityID);
		if (it4 != s_Data->EntityScriptFieldContainerStringBuffer.end())
		{
			s_Data->EntityScriptFieldContainerStringBuffer.erase(it4);
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
		s_Data->EntityScriptFieldContainerStringBuffer.clear();
	}

	ScriptFieldInstance::ScriptFieldInstance(ScriptField* field, UUID entityID)
		: FieldInstanceBase(field->Type)
		, m_Field(field)
		, m_EntityID(entityID)
	{
		ZE_CORE_ASSERT(m_Field, "Failed to create a script field instance from an emtpy field!");

		if (GetFieldType() == FieldType::String || // For string, we have a global buffer to store them, so our buffer just points to that global buffer
			GetFieldType() == FieldType::SeqCon) return;

		const U32 fieldSize = GetFieldSize();
		m_Buffer = Buffer(fieldSize);
		m_RuntimeBuffer = Buffer(fieldSize);
		m_bIsBufferAllocated = true;
	}

	ScriptFieldInstance::ScriptFieldInstance(const ScriptFieldInstance& other)
		: FieldInstanceBase(other)
		, m_Field(other.m_Field)
		, m_EntityID(other.m_EntityID)
	{
		if (m_bIsBufferAllocated)
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

		if (m_bIsBufferAllocated)
		{
			m_Buffer = std::move(other.m_Buffer);
			m_RuntimeBuffer = std::move(other.m_RuntimeBuffer);
		}
		else
		{
			m_Buffer = other.m_Buffer;
			m_RuntimeBuffer = other.m_RuntimeBuffer;
			other.m_Buffer = {};
			other.m_RuntimeBuffer = {};
		}
	}

	ScriptFieldInstance::~ScriptFieldInstance()
	{
		if (m_bIsBufferAllocated)
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
			if (m_bIsBufferAllocated)
			{
				m_Buffer.Release();
				m_RuntimeBuffer.Release();
			}

			m_Field = other.m_Field;
			m_EntityID = other.m_EntityID;
			if (m_bIsBufferAllocated)
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

			if (m_bIsBufferAllocated)
			{
				m_Buffer = std::move(other.m_Buffer);
				m_RuntimeBuffer = std::move(other.m_RuntimeBuffer);
			}
			else
			{
				m_Buffer = other.m_Buffer;
				m_RuntimeBuffer = other.m_RuntimeBuffer;
				other.m_Buffer = {};
				other.m_RuntimeBuffer = {};
			}
		}

		return *this;
	}

	const char* ScriptFieldInstance::GetFieldName() const
	{
		return m_Field->Name.c_str();
	}

	std::string ScriptFieldInstance::GetFieldTooltip() const
	{
		return m_Field->GetAttributeValue<std::string>("Tooltip").value_or("");
	}

	float ScriptFieldInstance::GetDragSpeed() const
	{
		return m_Field->GetAttributeValue<float>("DragSensitivity").value_or(1.0f);
	}

	bool ScriptFieldInstance::IsClampOnlyDuringDragging() const
	{
		return m_Field->HasAttribute("ClampOnlyDuringDragging");
	}

	AssetTypeID ScriptFieldInstance::GetAssetTypeID() const
	{
		const auto value = m_Field->GetAttributeValue<MonoReflectionType*>("AssetType");
		if (!value) return 0;
		auto* assetType = *value;
		return ScriptRegistry::GetAssetTypeIDFromType(assetType);
	}

	bool ScriptFieldInstance::IsHidden() const
	{
		return m_Field->HasAttribute("HiddenInEditor");

		// NOTE: "HideCondition" works depends on the changes made in editor to be synchronized to the script instance in real time
		//bool bHiddenInEditor = m_Field->HasAttribute("HiddenInEditor");
		//if (bHiddenInEditor) return true;

		//auto hideConditionMethodName = m_Field->GetAttributeValue<std::string>("HideCondition");
		//if (hideConditionMethodName.empty()) return false;

		//const auto scriptInstance = ScriptEngine::GetEntityScriptInstance(m_EntityID);
		//const auto scriptClass = scriptInstance->GetScriptClass();
		//auto* hideConditionMethod = scriptClass->GetMethod(hideConditionMethodName, 0);
		//if (!hideConditionMethod) return false;

		//auto* res = scriptClass->InvokeMethod(scriptInstance->GetMonoInstance(), hideConditionMethod);
		//return *static_cast<bool*>(mono_object_unbox(res));
	}

	bool ScriptFieldInstance::IsTransient() const
	{
		return m_Field->HasAttribute("Transient");
	}

	std::string ScriptFieldInstance::GetCategory() const
	{
		return m_Field->GetAttributeValue<std::string>("Category").value_or("");
	}

	void* ScriptFieldInstance::GetValueRaw()
	{
		if (SceneUtils::IsLevelRuntime())
		{
			GetRuntimeValueInternal(m_RuntimeBuffer.Data);
			return m_RuntimeBuffer.Data;
		}
		return m_Buffer.Data;
	}

	void ScriptFieldInstance::SetValueRaw(const void* value)
	{
		if (SceneUtils::IsLevelRuntime())
		{
			SetRuntimeValueInternal(value);
		}
		else
		{
			SetValueInternal(value);
		}
	}

	void ScriptFieldInstance::OnFieldValueChanged()
	{
		const U32 fieldID = GetFieldID();
		// TODO:
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

	void ScriptFieldInstance::CopyValueToRuntime()
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

	void ScriptFieldInstance::GetValueInternal(void* outValue) const
	{
		memcpy(outValue, m_Buffer.Data, GetFieldSize());
	}

	void ScriptFieldInstance::GetValueInternal(std::string& outValue) const
	{
		outValue = *m_Buffer.As<std::string>();
	}

	void ScriptFieldInstance::SetValueInternal(const void* value) const
	{
		memcpy(m_Buffer.Data, value, GetFieldSize());
		//CopyValueToRuntime();
	}

	void ScriptFieldInstance::SetValueInternal(const std::string& value) const
	{
		m_Buffer.As<std::string>()->assign(value);
		//CopyValueToRuntime();
	}

	void ScriptFieldInstance::GetRuntimeValueInternal(void* outValue) const
	{
		const auto scriptInstance = ScriptEngine::GetEntityScriptInstance(m_EntityID);
		if (GetFieldType() == FieldType::Entity)
		{
			// Get managed object (C# class)
			MonoObject* object = nullptr;
			mono_field_get_value(scriptInstance->GetMonoInstance(), m_Field->ClassField, &object);
			if (object) // Entity may have not been assigned (not newed yet) // TODO:
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

	SizeT ScriptSequenceContainerFieldInstance::GetContainerSize() const
	{
		return SceneUtils::IsLevelRuntime() ? mono_array_length(GetListArray()) : m_ContainerSize;
	}

	bool ScriptSequenceContainerFieldInstance::InsertDefault(U32 index, bool bNeedsUpdateArray)
	{
		if (index < 0 || index > GetContainerSize()) return false;

		if (SceneUtils::IsLevelRuntime())
		{
			if (auto* listObject = GetListObject())
			{
				auto* listClass = mono_object_get_class(listObject);
				auto* insertMethod = mono_class_get_method_from_name(listClass, "Insert", 2);
				MonoObject* exception = nullptr;
				if (GetElementType() == FieldType::String)
				{
					auto* monoStr = mono_string_new(mono_domain_get(), "");
					void* params[] = { &index, monoStr }; // NOTE how MonoString* is passes as parameter 
					mono_runtime_invoke(insertMethod, listObject, params, &exception);
				}
				else
				{
					const ScopedBuffer buffer(GetElementSize());
					void* params[] = { &index, buffer.Data() };
					mono_runtime_invoke(insertMethod, listObject, params, &exception);
				}

				if (bNeedsUpdateArray)
				{
					// We have to update the array instance as array size is changed during looping
					GetRuntimeArrayData();
				}
			}
			else
			{
				return false;
			}
		}
		else
		{
			if (GetElementType() == FieldType::String)
			{
				auto* containerStringBuffer = m_Buffer.As<std::vector<std::string>>();
				containerStringBuffer->insert(containerStringBuffer->begin() + index, "");
			}
			else
			{
				const U32 elementSize = GetElementSize();
				if (m_ContainerSize * elementSize == m_Buffer.Size)
				{
					Buffer temp(m_Buffer.Size > 0 ? m_Buffer.Size * 2 : 2 * elementSize);
					memcpy(temp.Data, m_Buffer.Data, m_Buffer.Size);
					m_Buffer = std::move(temp);
				}

				for (I64 i = m_ContainerSize - 1; i >= index; --i)
				{
					memcpy(m_Buffer.Data + (i + 1) * elementSize, m_Buffer.Data + i * elementSize, elementSize);
				}
				memset(m_Buffer.Data + index * elementSize, 0, elementSize); // We assume element's default value is 0
			}
			++m_ContainerSize;
		}

		return true;
	}

	bool ScriptSequenceContainerFieldInstance::Erase(U32 index)
	{
		if (index < 0 || index >= GetContainerSize()) return false;

		if (SceneUtils::IsLevelRuntime())
		{
			if (auto* listObject = GetListObject())
			{
				auto* listClass = mono_object_get_class(listObject);
				auto* removeAtMethod = mono_class_get_method_from_name(listClass, "RemoveAt", 1);
				MonoObject* exception = nullptr;
				void* params = &index;
				mono_runtime_invoke(removeAtMethod, listObject, &params, &exception);

				// We have to update the array instance as array size is changed during looping
				GetRuntimeArrayData();
			}
			else
			{
				return false;
			}
		}
		else
		{
			if (GetElementType() == FieldType::String)
			{
				auto* containerStringBuffer = m_Buffer.As<std::vector<std::string>>();
				containerStringBuffer->erase(containerStringBuffer->begin() + index);
			}
			else
			{
				const U32 elementSize = GetElementSize();
				// We do not call dtor for that element as we "constructed" it via memset
				for (U64 i = index + 1; i < m_ContainerSize; ++i)
				{
					memcpy(m_Buffer.Data + (i - 1) * elementSize, m_Buffer.Data + i * elementSize, elementSize);
				}
			}
			--m_ContainerSize;
		}
		return true;
	}

	void ScriptSequenceContainerFieldInstance::Clear()
	{
		if (SceneUtils::IsLevelRuntime())
		{
			if (auto* listObject = GetListObject())
			{
				auto* listClass = mono_object_get_class(listObject);
				auto* clearMethod = mono_class_get_method_from_name(listClass, "Clear", 0);
				MonoObject* exception = nullptr;
				mono_runtime_invoke(clearMethod, listObject, nullptr, &exception);

				// We do not need to update the array instance here as it will be updated later before drawing element widgets
			}
		}
		else
		{
			if (GetElementType() == FieldType::String)
			{
				auto* containerStringBuffer = m_Buffer.As<std::vector<std::string>>();
				containerStringBuffer->clear();
			}
			else
			{
				// We do not call dtor for all elements as we "constructed" them via memset
				m_Buffer.Release();
			}
			m_ContainerSize = 0;
		}
	}

	void ScriptSequenceContainerFieldInstance::ResizeWithDefault(SizeT size)
	{
		ZE_CORE_ASSERT(!SceneUtils::IsLevelRuntime());

		if (GetElementType() == FieldType::String)
		{
			auto* containerStringBuffer = m_Buffer.As<std::vector<std::string>>();
			containerStringBuffer->resize(size);
		}
		else
		{
			const auto targetSize = size * GetElementSize();
			if (targetSize > m_Buffer.Size)
			{
				m_Buffer = Buffer(targetSize);
			}
			else
			{
				// We do not call dtor for redundant elements as we "constructed" them via memset
				memset(m_Buffer.Data, 0, targetSize);
			}
		}
		m_ContainerSize = size;
	}

	void ScriptSequenceContainerFieldInstance::MoveElement(U32 from, U32 to)
	{
		if (from < 0 || from >= GetContainerSize() || to < 0 || to >= GetContainerSize() || from == to) return;

		if (SceneUtils::IsLevelRuntime())
		{
			if (auto* listObject = GetListObject())
			{
				auto* listClass = mono_object_get_class(listObject);
				MonoObject* exception = nullptr;

				// Backup element
				auto* getItemMethod = mono_class_get_method_from_name(listClass, "get_Item", 1); // NOTE how to get indexer getter method
				void* fromParam = &from;
				auto* elementObject = mono_runtime_invoke(getItemMethod, listObject, &fromParam, &exception);
				void* element = GetElementType() == FieldType::String/* NOTE: Add types that are essentially MonoObject* */ ? elementObject : mono_object_unbox(elementObject);

				// Remove element
				auto* removeMethod = mono_class_get_method_from_name(listClass, "RemoveAt", 1);
				mono_runtime_invoke(removeMethod, listObject, &fromParam, &exception);

				// Insert that element to the new location
				auto* insertMethod = mono_class_get_method_from_name(listClass, "Insert", 2);
				void* params[] = { &to, element };
				mono_runtime_invoke(insertMethod, listObject, params, &exception);

				// We have to update the array instance as array size is changed during looping
				GetRuntimeArrayData();
			}
		}
		else
		{
			if (GetElementType() == FieldType::String)
			{
				auto* containerStringBuffer = m_Buffer.As<std::vector<std::string>>();
				if (from < to)
				{
					std::rotate(containerStringBuffer->begin() + from, containerStringBuffer->begin() + from + 1, containerStringBuffer->begin() + to + 1);
				}
				else
				{
					std::rotate(containerStringBuffer->begin() + to, containerStringBuffer->begin() + from, containerStringBuffer->begin() + from + 1);
				}
			}
			else
			{
				const U32 elementSize = GetElementSize();
				if (from < to)
				{
					std::rotate(m_Buffer.Data + from * elementSize, m_Buffer.Data + (from + 1) * elementSize, m_Buffer.Data + (to + 1) * elementSize);
				}
				else
				{
					std::rotate(m_Buffer.Data + to * elementSize, m_Buffer.Data + from * elementSize, m_Buffer.Data + (from + 1) * elementSize);
				}
			}
		}
	}

	void* ScriptSequenceContainerFieldInstance::GetValueRaw()
	{
		if (SceneUtils::IsLevelRuntime())
		{
			GetRuntimeValueInternal(m_RuntimeBuffer.Data);
			return m_RuntimeBuffer.Data;
		}

		if (m_ElementType == FieldType::String)
		{
			auto& containerStringBuffer = *m_Buffer.As<std::vector<std::string>>();
			return &containerStringBuffer[m_CurrentElementIndex];
		}
		return m_Buffer.Data + GetElementSize() * m_CurrentElementIndex;
	}

	void ScriptSequenceContainerFieldInstance::GetValueInternal(void* outValue) const
	{
		memcpy(outValue, m_Buffer.Data + GetElementSize() * m_CurrentElementIndex, GetElementSize());
	}

	void ScriptSequenceContainerFieldInstance::GetValueInternal(std::string& outValue) const
	{
		const auto& containerStringBuffer = *m_Buffer.As<std::vector<std::string>>();
		outValue = containerStringBuffer[m_CurrentElementIndex];
	}

	void ScriptSequenceContainerFieldInstance::SetValueInternal(const void* value) const
	{
		memcpy(m_Buffer.Data + GetElementSize() * m_CurrentElementIndex, value, GetElementSize());
		//CopyValueToRuntime();
	}

	void ScriptSequenceContainerFieldInstance::SetValueInternal(const std::string& value) const
	{
		auto& containerStringBuffer = *m_Buffer.As<std::vector<std::string>>();
		containerStringBuffer[m_CurrentElementIndex].assign(value);
		//CopyValueToRuntime();
	}

	void ScriptSequenceContainerFieldInstance::GetRuntimeValueInternal(void* outValue) const
	{
		if (auto* monoArray = GetListArray()) // The array instance is updated at runtime every frame before drawing element widgets
		{
			const U32 elementSize = GetElementSize();

			// Retrieve current element value
			auto* addr = mono_array_addr_with_size(monoArray, elementSize, m_CurrentElementIndex);
			memcpy(outValue, addr, elementSize);
		}
	}

	void ScriptSequenceContainerFieldInstance::GetRuntimeValueInternal(std::string& outValue) const
	{
		if (auto* monoArray = GetListArray())
		{
			// Retrieve current element value
			auto* monoStr = mono_array_get(monoArray, MonoString*, m_CurrentElementIndex);
			char* str = mono_string_to_utf8(monoStr);
			outValue = monoStr != nullptr ? str : "";
			mono_free(str);
		}
	}

	void ScriptSequenceContainerFieldInstance::SetRuntimeValueInternal(const void* value) const
	{
		auto* listObject = GetListObject();
		auto* listClass = mono_object_get_class(listObject);
		auto* setItemMethod = mono_class_get_method_from_name(listClass, "set_Item", 2); // NOTE how to get indexer setter method
		void* param[] = { const_cast<U32*>(&m_CurrentElementIndex), const_cast<void*>(value) };
		MonoObject* exception = nullptr;
		mono_runtime_invoke(setItemMethod, listObject, param, &exception);
	}

	void ScriptSequenceContainerFieldInstance::SetRuntimeValueInternal(const std::string& value) const
	{
		auto* listObject = GetListObject();
		auto* listClass = mono_object_get_class(listObject);
		auto* setItemMethod = mono_class_get_method_from_name(listClass, "set_Item", 2);
		MonoString* monoStr = mono_string_new(ScriptEngine::GetAppDomain(), value.c_str());
		void* param[] = { const_cast<U32*>(&m_CurrentElementIndex), monoStr };
		MonoObject* exception = nullptr;
		mono_runtime_invoke(setItemMethod, listObject, param, &exception);
	}

	void ScriptSequenceContainerFieldInstance::CopyValueFromRuntime()
	{
		GetRuntimeArrayData();
		if (auto* monoArray = GetListArray())
		{
			// Store array info
			m_ContainerSize = mono_array_length(monoArray);
			m_ElementClass = mono_class_get_element_class(mono_object_get_class(reinterpret_cast<MonoObject*>(monoArray)));
			auto* elementType = mono_class_get_type(m_ElementClass);
			m_ElementType = Utils::MonoTypeToFieldType(elementType);

			if (GetElementType() == FieldType::String)
			{
				auto& containerStringBuffer = s_Data->EntityScriptFieldContainerStringBuffer[m_EntityID][m_Field->Name];
				for (SizeT i = 0; i < m_ContainerSize; ++i)
				{
					m_CurrentElementIndex = static_cast<U32>(i);
					std::string outStr;
					GetRuntimeValueInternal(outStr);
					containerStringBuffer.emplace_back(std::move(outStr));
				}
				m_Buffer.Data = reinterpret_cast<U8*>(&containerStringBuffer);
			}
			else
			{
				const U32 elementSize = GetElementSize();

				// Allocate memory to store all the elements
				m_Buffer = Buffer(m_ContainerSize * elementSize);
				// Allocate memory to store one element value
				m_RuntimeBuffer = Buffer(elementSize);
				m_bIsBufferAllocated = true;

				// Copy array elements to buffer
				auto* startAddr = mono_array_addr_with_size(monoArray, elementSize, 0);
				memcpy(m_Buffer.Data, startAddr, m_Buffer.Size);
			}
		}
	}

	void ScriptSequenceContainerFieldInstance::CopyValueToRuntime()
	{
		auto* monoArray = GetListArray();
		ZE_CORE_ASSERT(monoArray);

		// Array size has been changed before runtime starts, we have to create a new array and free the old one
		if (m_ContainerSize != mono_array_length(monoArray))
		{
			monoArray = mono_array_new(mono_domain_get(), m_ElementClass, m_ContainerSize);
			ZE_CORE_ASSERT(monoArray);

			// Free last array handle
			mono_gchandle_free(m_ArrayHandle);
			// Request new array handle
			m_ArrayHandle = mono_gchandle_new(reinterpret_cast<MonoObject*>(monoArray), false);
		}

		SetRuntimeArrayData([this](MonoArray* monoArray)
		{
			// Copy buffer data back to array
			if (GetElementType() == FieldType::String)
			{
				for (U32 i = 0; i < m_ContainerSize; ++i)
				{
					auto** elementAddr = reinterpret_cast<MonoString**>(mono_array_addr_with_size(monoArray, sizeof(MonoString*), i));
					const auto& containerStringBuffer = *m_Buffer.As<std::vector<std::string>>();
					auto* monoStr = mono_string_new(ScriptEngine::GetAppDomain(), containerStringBuffer[i].c_str());
					*elementAddr = monoStr;
				}
			}
			else
			{
				auto* elementAddr = mono_array_addr_with_size(monoArray, GetElementSize(), 0);
				memcpy(elementAddr, m_Buffer.Data, m_Buffer.Size);
			}
		});
	}

	void ScriptSequenceContainerFieldInstance::OnReload(ScriptField* field)
	{
		ScriptFieldInstance::OnReload(field);

		GetRuntimeArrayData();
	}

	void ScriptSequenceContainerFieldInstance::GetRuntimeArrayData()
	{
		if (auto* listObject = GetListObject())
		{
			auto* listClass = mono_object_get_class(listObject);

			// Convert list to array
			auto* toArrayMethod = mono_class_get_method_from_name(listClass, "ToArray", 0);
			MonoObject* exception = nullptr;
			auto* arrayObject = mono_runtime_invoke(toArrayMethod, listObject, nullptr, &exception);

			// We have to invalidate and retrieve array every frame as array is generated from list and we don't know if list has made any changes
			// Free array handle first
			mono_gchandle_free(m_ArrayHandle);

			// Request new array handle which will prevent gc and can be used later this frame when setting back value
			m_ArrayHandle = mono_gchandle_new(arrayObject, false);
		}
	}

	MonoObject* ScriptSequenceContainerFieldInstance::GetListObject() const
	{
		const auto scriptInstance = ScriptEngine::GetEntityScriptInstance(m_EntityID);
		MonoObject* listObject = nullptr;
		mono_field_get_value(scriptInstance->GetMonoInstance(), m_Field->ClassField, &listObject);
		return listObject;
	}

	MonoArray* ScriptSequenceContainerFieldInstance::GetListArray() const
	{
		return reinterpret_cast<MonoArray*>(mono_gchandle_get_target(m_ArrayHandle));
	}

	ScriptClass::ScriptClass(std::string nameSpace, std::string className, bool bIsCore)
		: m_Namespace(std::move(nameSpace)), m_ClassName(std::move(className))
	{
		m_MonoClass = mono_class_from_name(bIsCore ? s_Data->CoreAssemblyImage : s_Data->AppAssemblyImage, m_Namespace.c_str(), m_ClassName.c_str());

		void* it = nullptr;
		while (MonoClassField* field = mono_class_get_fields(m_MonoClass, &it))
		{
			std::string fieldName = mono_field_get_name(field);
			const auto fieldFlags = mono_field_get_flags(field);
			if (fieldFlags & FIELD_ATTRIBUTE_PUBLIC)
			{
				const FieldType fieldType = Utils::MonoTypeToFieldType(mono_field_get_type(field));
				if (fieldType == FieldType::None) continue; // Skip unknown field

				m_Fields[fieldName] = { fieldName, fieldType, field };
			}
		}

		// TODO: Gather properties
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
