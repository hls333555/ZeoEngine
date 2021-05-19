#pragma once

#include <entt.hpp>

#include "Engine/Utils/EngineUtils.h"

namespace ZeoEngine {

	enum class AssetType
	{
		NONE,

		Scene,
		ParticleTemplate,
		Texture,
	};

	template<typename T>
	using Asset = entt::resource_handle<T>;

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
		AssetPath(const char* path)
			: m_RelativePath(GetRelativePath(path))
		{
		}
		AssetPath(const std::string& path)
			: m_RelativePath(GetRelativePath(path))
		{
		}

		bool IsEmpty() { return m_RelativePath.empty(); }

		std::string ToString() { return m_RelativePath; }
		uint32_t ToId() { return entt::hashed_string{ m_RelativePath.c_str() }; }

	private:
		std::string m_RelativePath;
	};

}
