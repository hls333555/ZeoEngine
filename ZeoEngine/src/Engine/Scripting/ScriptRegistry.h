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
		static void RegisterMonoAsset(char* monoAssetName, U32 typeID);
		static void ReloadMonoAssets();

		static U32 GetComponentIDFromType(MonoReflectionType* compType);
		static U32 GetAssetTypeIDFromType(MonoReflectionType* assetType);

	private:
		/** Map from mono component type to native component type ID */
		inline static std::unordered_map<MonoType*, U32> s_RegisteredMonoComponents;
		/** Map from mono component name to native component type ID, used for reloading */
		inline static std::unordered_map<std::string, U32> s_RegisteredMonoComponentNames;
		/** Map from mono asset type to native asset type ID */
		inline static std::unordered_map<MonoType*, U32> s_RegisteredMonoAssets;
		/** Map from mono asset name to native asset type ID, used for reloading */
		inline static std::unordered_map<std::string, U32> s_RegisteredMonoAssetNames;
	};
	
}
