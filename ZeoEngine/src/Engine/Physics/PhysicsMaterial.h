#pragma once

#include "Engine/Asset/Asset.h"

namespace ZeoEngine {

	class PhysicsMaterial : public AssetBase<PhysicsMaterial>
	{
		friend struct PhysicsMaterialDetailComponent;

	public:
		float GetStaticFriction() const { return m_StaticFriction; }
		float GetDynamicFriction() const { return m_DynamicFriction; }
		float GetBounciness() const { return m_Bounciness; }

	private:
		float m_StaticFriction = 0.6f;
		float m_DynamicFriction = 0.6f;
		float m_Bounciness = 0.0f;
	};
	
}
