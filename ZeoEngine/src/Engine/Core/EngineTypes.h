#pragma once

#include <entt.hpp>

namespace ZeoEngine {

	using AssetTypeId = uint32_t;

	template<typename AssetClass>
	using AssetHandle = entt::resource<AssetClass>;

}
