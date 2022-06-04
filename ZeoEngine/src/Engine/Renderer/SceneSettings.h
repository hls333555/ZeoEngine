#pragma once

namespace ZeoEngine {

	struct SceneSettings
	{
		static constexpr U32 MaxPointLights = 32;
		static constexpr U32 MaxSpotLights = 32;
		static constexpr U32 MaxCascades = 4;
		static constexpr U32 ShadowMapResolution = 2048;
	};

}
