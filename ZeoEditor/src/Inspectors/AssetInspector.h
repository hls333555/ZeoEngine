#pragma once

#include "Inspectors/ComponentInspector.h"

namespace ZeoEngine {

	class AssetPreviewWorldBase;

	class AssetInspector : public ComponentInspector
	{
	public:
		AssetInspector(AssetPreviewWorldBase* world, U32 compID);

		virtual void Draw(Entity entity) override final;

	private:
		virtual void ProcessDraw(Entity entity) {}

	private:
		AssetPreviewWorldBase* m_AssetWorld = nullptr;
	};

}
