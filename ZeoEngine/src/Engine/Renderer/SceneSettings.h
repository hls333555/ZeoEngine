#pragma once

namespace ZeoEngine {

	struct SceneSettings
	{
		static constexpr uint32_t MaxPointLights = 32;
		static constexpr uint32_t MaxSpotLights = 32;
		static constexpr uint32_t MaxCascades = 4;
		static constexpr uint32_t ShadowMapResolution = 2048;
	};

}
