#pragma once

#include <entt.hpp>

namespace ZeoEngine {

	extern const char* g_EngineAssetExtension;
	extern const char* g_AssetTypeToken;

	using AssetTypeId = uint32_t;
	using AssetTypeName = std::string_view;

	extern AssetTypeId g_InvalidAssetType;

	template<typename AssetClass>
	class AssetType
	{
	public:
		static constexpr AssetTypeId Id()
		{
			return entt::type_hash<AssetClass>::value();
		}

		/** For debug purposes only. */
		static constexpr AssetTypeName Name()
		{
			auto view =  entt::type_name<AssetClass>::value();
			auto separator = view.rfind("::");
			return view.substr(separator + 2, view.size() - separator);
		}
	};

	template<typename Resource>
	using Asset = entt::resource_handle<Resource>;

	template<typename Loader, typename Resource>
	using AssetLoader = entt::resource_loader<Loader, Resource>;

	template<typename T>
	bool operator==(const Asset<T>& lhs, const Asset<T>& rhs)
	{
		return (!lhs && !rhs) || (lhs && rhs && std::addressof(*lhs) == std::addressof(*rhs));
	}
	template<typename T>
	bool operator!=(const Asset<T>& lhs, const Asset<T>& rhs)
	{
		return !(lhs == rhs);
	}

	class AssetPath
	{
	public:
		AssetPath(const char* path);
		AssetPath(const std::string& path);

		bool IsEmpty() { return m_RelativePath.empty(); }

		std::string ToString() { return m_RelativePath; }
		uint32_t ToId() { return entt::hashed_string{ m_RelativePath.c_str() }; }

	private:
		std::string m_RelativePath;
	};

}
