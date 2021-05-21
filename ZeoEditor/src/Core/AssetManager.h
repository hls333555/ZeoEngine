#pragma once

#include <unordered_map>

#include "Engine/Core/Core.h"
#include "Engine/Core/EngineTypes.h"

namespace ZeoEngine {

	class AssetActions;

	class AssetManager
	{
		friend class EditorLayer;

	public:
		static AssetManager& Get()
		{
			static AssetManager instance;
			return instance;
		}

		bool OpenAsset(const std::string& path);

	private:
		void Init();

	protected:
		AssetManager() = default;
		AssetManager(const AssetManager&) = delete;
		AssetManager& operator=(const AssetManager&) = delete;

	private:
		/** Map from asset type to asset actions */
		std::unordered_map<AssetType, Ref<AssetActions>> m_AssetActions;
	};

}
