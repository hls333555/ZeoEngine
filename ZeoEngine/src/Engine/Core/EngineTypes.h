#pragma once

#include <entt.hpp>

namespace ZeoEngine {

	using AssetTypeId = uint32_t;

	template<typename AssetClass>
	using AssetHandle = entt::resource_handle<AssetClass>;

	template<typename AssetLoaderClass, typename AssetClass>
	using AssetLoader = entt::resource_loader<AssetLoaderClass, AssetClass>;

	template<typename T>
	bool operator==(const AssetHandle<T>& lhs, const AssetHandle<T>& rhs)
	{
		return (!lhs && !rhs) || (lhs && rhs && std::addressof(*lhs) == std::addressof(*rhs));
	}
	template<typename T>
	bool operator!=(const AssetHandle<T>& lhs, const AssetHandle<T>& rhs)
	{
		return !(lhs == rhs);
	}

	class AssetPath
	{
	public:
		AssetPath(const char* path);
		AssetPath(const std::string& path);

		const std::string& GetPath() const { return m_RelativePath; }
		std::string GetName() const;

		bool IsEmpty() { return m_RelativePath.empty(); }

		uint32_t ToId() const { return entt::hashed_string{ m_RelativePath.c_str() }; }

	private:
		std::string m_RelativePath;
	};

}
