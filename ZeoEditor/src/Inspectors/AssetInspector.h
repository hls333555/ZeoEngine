#pragma once

#include "Inspectors/ComponentInspector.h"

namespace ZeoEngine {

	class AssetPreviewWorldBase;

	class AssetInspector : public ComponentInspector
	{
	public:
		AssetInspector(const Ref<AssetPreviewWorldBase>& world, U32 compID);

		virtual void Draw(Entity entity) override final;

	private:
		virtual void ProcessDraw(Entity entity) {}

	private:
		Weak<AssetPreviewWorldBase> m_AssetWorld;
	};

}
