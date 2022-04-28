#pragma once

#include <entt.hpp>

namespace ZeoEngine {

	using AssetTypeId = uint32_t;

	template<typename AssetClass>
	using AssetHandle = entt::resource<AssetClass>;

	class AssetPath
	{
	public:
		AssetPath() = default;
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
