#pragma once

namespace ZeoEngine {

	class Tag
	{
	public:
		inline static constexpr auto AnyTransformDirty = "AnyTransformDirty"_hs;
		inline static constexpr auto LocalTransformDirty = "LocalTransformDirty"_hs;
		inline static constexpr auto ChildCollider = "ChildCollider"_hs;
		inline static constexpr auto IsDeserializing = "IsDeserializing"_hs;
		inline static constexpr auto HideEntity = "HideEntity"_hs;

	};
	
}
