#pragma once

#include <entt.hpp>

namespace ZeoEngine {

	enum class AssetType
	{
		NONE,

		Scene,
		ParticleTemplate,
		Texture2D,
	};

	extern const char* g_EngineAssetExtension;
	extern const char* g_AssetTypeToken;
	extern const char* g_AssetNameToken;

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
