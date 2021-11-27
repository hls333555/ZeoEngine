#pragma once

#include <any>

#include "Engine/Core/Core.h"

namespace ZeoEngine {

	class Entity;
	struct IComponent;

	class IComponentHelper
	{
	public:
		explicit IComponentHelper(Entity* entity);
		virtual ~IComponentHelper();

		/**  Called after component being added to the owner entity. */
		virtual void OnComponentAdded() {}
		/**  Called after component being copied to the owner entity. You should handle logic here instead of in the component's copy ctor. */
		virtual void OnComponentCopied(IComponent* otherComp) {}
		/** Called before component being removed from the owner entity. */
		virtual void OnComponentDestroy() {}

		/** Called every time this data is changed in the editor. (e.g. DURING dragging a slider to tweak the value) */
		virtual void OnComponentDataValueEditChange(uint32_t dataId, std::any oldValue) {}
		/** Called only when this data is changed and deactivated in the editor. (e.g. AFTER dragging a slider to tweak the value) */
		virtual void PostComponentDataValueEditChange(uint32_t dataId, std::any oldValue) {}

		Entity* GetOwnerEntity() const;

	private:
		struct Impl;
		Scope<Impl> m_Impl;
	};

	class ParticleSystemComponentHelper : public IComponentHelper
	{
	public:
		using IComponentHelper::IComponentHelper;

		virtual void OnComponentCopied(IComponent* otherComp) override;
		virtual void OnComponentDestroy() override;

		virtual void OnComponentDataValueEditChange(uint32_t dataId, std::any oldValue) override;
		virtual void PostComponentDataValueEditChange(uint32_t dataId, std::any oldValue) override;
	};

	class ParticleSystemPreviewComponentHelper : public IComponentHelper
	{
	public:
		using IComponentHelper::IComponentHelper;

		virtual void OnComponentDataValueEditChange(uint32_t dataId, std::any oldValue) override;
		virtual void PostComponentDataValueEditChange(uint32_t dataId, std::any oldValue) override;
	};

	class LightComponentHelper : public IComponentHelper
	{
	public:
		using IComponentHelper::IComponentHelper;

		virtual void OnComponentAdded() override;
		virtual void OnComponentCopied(IComponent* otherComp) override;
		virtual void PostComponentDataValueEditChange(uint32_t dataId, std::any oldValue) override;

		void InitLight();
	};

}
