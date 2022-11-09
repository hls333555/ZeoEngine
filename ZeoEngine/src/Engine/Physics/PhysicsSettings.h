#pragma once

namespace ZeoEngine {

	struct PhysicsSettings
	{
		float FixedDeltaTime = 1.0f / 60.0f;
		glm::vec3 Gravity{ 0.0f, -9.81f, 0.0f };
	};
	
}
