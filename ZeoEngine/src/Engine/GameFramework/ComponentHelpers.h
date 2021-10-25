#pragma once

#include <any>

#include "Engine/Core/Core.h"

namespace ZeoEngine {

	class Entity;

	class IComponentHelper
	{
	protected:
		IComponentHelper();
		virtual ~IComponentHelper();

	public:
		/**  Called after component being added to the owner entity. */
		virtual void OnComponentAdded() {}
		/**  Called after component being copied to the owner entity. */
		virtual void OnComponentCopied() {}
		/** Called before component being removed from the owner entity. */
		virtual void OnComponentDestroy() {}

		/** Called every time this data is changed in the editor. (e.g. DURING dragging a slider to tweak the value) */
		virtual void OnComponentDataValueEditChange(uint32_t dataId, std::any oldValue) {}
		/** Called only when this data is changed and deactivated in the editor. (e.g. AFTER dragging a slider to tweak the value) */
		virtual void PostComponentDataValueEditChange(uint32_t dataId, std::any oldValue) {}

		Entity* GetOwnerEntity() const;
		void SetOwnerEntity(Entity* entity);

	private:
		struct Impl;
		Scope<Impl> m_Impl;
	};

	class ParticleSystemComponentHelper : public IComponentHelper
	{
	public:
		virtual void OnComponentCopied() override;
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
