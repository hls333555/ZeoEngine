#pragma once

#include <any>

#include "Engine/GameFramework/Entity.h"

namespace ZeoEngine {

	class IComponentHelper
	{
	protected:
		IComponentHelper() = default;
		IComponentHelper(const IComponentHelper&) = default;

	public:
		/**  Called after component being added to the owner entity. */
		virtual void OnComponentAdded() {}
		/** Called before component being removed from the owner entity. */
		virtual void OnComponentDestroy() {}

		/** Called every time this data is changed in the editor. (e.g. DURING dragging a slider to tweak the value) */
		virtual void OnComponentDataValueEditChange(uint32_t dataId, std::any oldValue) {}
		/** Called only when this data is changed and deactivated in the editor. (e.g. AFTER dragging a slider to tweak the value) */
		virtual void PostComponentDataValueEditChange(uint32_t dataId, std::any oldValue) {}

		void SetOwnerEntity(Entity entity) { m_OwnerEntity = entity; }

	protected:
		Entity m_OwnerEntity;
	};

	class ParticleSystemComponentHelper : public IComponentHelper
	{
	public:
		virtual void OnComponentDestroy() override;

		virtual void OnComponentDataValueEditChange(uint32_t dataId, std::any oldValue) override;
		virtual void PostComponentDataValueEditChange(uint32_t dataId, std::any oldValue) override;
	};

	class ParticleSystemPreviewComponentHelper : public IComponentHelper
	{
	public:
		virtual void OnComponentDataValueEditChange(uint32_t dataId, std::any oldValue) override;
		virtual void PostComponentDataValueEditChange(uint32_t dataId, std::any oldValue) override;
	};

}
