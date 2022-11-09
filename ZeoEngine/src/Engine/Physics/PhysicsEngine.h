#pragma once

#include "Engine/Physics/PhysicsSettings.h"

namespace ZeoEngine {

	class PhysicsEngine
	{
	public:
		static void Init();
		static void Shutdown();

		static PhysicsSettings& GetSettings() { return s_Settings; }

	private:
		static PhysicsSettings s_Settings;
	};

}
