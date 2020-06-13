#include "TiledMap.h"

RTTR_REGISTRATION
{
	using namespace rttr;
	using namespace ZeoEngine;
	registration::class_<TiledMap>("TiledMap")
		.constructor(&TiledMap::SpawnGameObject, policy::ctor::as_raw_ptr);
}
