#pragma once

extern "C" {
	typedef struct _MonoType MonoType;
}

namespace ZeoEngine {

	class ScriptRegistry
	{
	public:
		static void RegisterFunctions();

		/** Map from mono component type to native component type ID */
		static std::unordered_map<MonoType*, U32> s_RegisteredMonoComponents;
	};
	
}
