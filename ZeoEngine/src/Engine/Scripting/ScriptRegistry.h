#pragma once

extern "C" {
	typedef struct _MonoType MonoType;
	typedef struct _MonoReflectionType MonoReflectionType;
}

namespace ZeoEngine {

	class ScriptRegistry
	{
	public:
		static void RegisterFunctions();
		static void RegisterMonoComponent(char* monoCompName, U32 compID);
		static void ReloadMonoComponents();

		static U32 GetComponentIDFromType(MonoReflectionType* compType);

	private:
		/** Map from mono component type to native component type ID */
		static std::unordered_map<MonoType*, U32> s_RegisteredMonoComponents;
		/** Map from mono component name to native component type ID, used for reloading */
		static std::unordered_map<std::string, U32> s_RegisteredMonoComponentNames;
	};
	
}
