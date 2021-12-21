#pragma once

#include <any>

#include "Engine/Core/Core.h"
#include "Engine/Math/BoxSphereBounds.h"

namespace ZeoEngine {

	class Entity;
	struct IComponent;
	class RenderGraph;

	class IComponentHelper
	{
	public:
		explicit IComponentHelper(Entity* entity);
		virtual ~IComponentHelper();

		/** Called after component being added to the owner entity. If bIsDeserialize is true, the component is added during deserialization. */
		virtual void OnComponentAdded(bool bIsDeserialize) {}
		/** Called after component being copied to the owner entity. You should handle logic here instead of in the component's copy ctor. */
		virtual void OnComponentCopied(IComponent* otherComp) {}
		/**
		 * Called before component being removed from the owner entity.
		 * There are mainly three ways to remove a component:
		 * Call Entity::RemoveComponent
		 * Call Entity::RemoveComponentById
		 * Destroy the owner entity
		 */
		virtual void OnComponentDestroy() {}

		/** Called every time this data is changed in the editor. (e.g. DURING dragging a slider to tweak the value) */
		virtual void OnComponentDataValueEditChange(uint32_t dataId, std::any oldValue, int32_t elementIndex) {}
		/** Called only when this data is changed and deactivated in the editor. (e.g. AFTER dragging a slider to tweak the value) */
		virtual void PostComponentDataValueEditChange(uint32_t dataId, std::any oldValue, int32_t elementIndex) {}

		/** Called after certain data has been deserialized. */
		virtual void PostDataDeserialize(uint32_t dataId) {}

		/** Calculate component bounds or return an invalid one. */
		virtual BoxSphereBounds GetBounds() { return {}; }

		Entity* GetOwnerEntity() const;

	private:
		struct Impl;
		Scope<Impl> m_Impl;
	};

	class TransformComponentHelper : public IComponentHelper
	{
	public:
		using IComponentHelper::IComponentHelper;

		virtual void OnComponentDataValueEditChange(uint32_t dataId, std::any oldValue, int32_t elementIndex = -1) override;
		virtual void PostComponentDataValueEditChange(uint32_t dataId, std::any oldValue, int32_t elementIndex = -1) override;
	};

	class CameraComponentHelper : public IComponentHelper
	{
	public:
		using IComponentHelper::IComponentHelper;

		virtual void OnComponentAdded(bool bIsDeserialize) override;
		virtual void OnComponentDestroy() override;
	};

	class ParticleSystemComponentHelper : public IComponentHelper
	{
	public:
		using IComponentHelper::IComponentHelper;

		virtual void OnComponentCopied(IComponent* otherComp) override;
		virtual void OnComponentDestroy() override;
		virtual void OnComponentDataValueEditChange(uint32_t dataId, std::any oldValue, int32_t elementIndex = -1) override;
		virtual void PostComponentDataValueEditChange(uint32_t dataId, std::any oldValue, int32_t elementIndex = -1) override;
	};

	class ParticleSystemPreviewComponentHelper : public IComponentHelper
	{
	public:
		using IComponentHelper::IComponentHelper;

		virtual void OnComponentDataValueEditChange(uint32_t dataId, std::any oldValue, int32_t elementIndex = -1) override;
		virtual void PostComponentDataValueEditChange(uint32_t dataId, std::any oldValue, int32_t elementIndex = -1) override;
	};

	class MeshRendererComponentHelper : public IComponentHelper
	{
	public:
		using IComponentHelper::IComponentHelper;

		virtual void OnComponentAdded(bool bIsDeserialize) override;
		virtual void OnComponentCopied(IComponent* otherComp) override;
		virtual void PostComponentDataValueEditChange(uint32_t dataId, std::any oldValue, int32_t elementIndex = -1) override;
		virtual void PostDataDeserialize(uint32_t dataId) override;
		virtual BoxSphereBounds GetBounds() override;

	private:
		const RenderGraph& GetRenderGraph(Entity* contextEntity) const;
	};

	class LightComponentHelper : public IComponentHelper
	{
	public:
		using IComponentHelper::IComponentHelper;

		virtual void OnComponentAdded(bool bIsDeserialize) override;
		virtual void OnComponentCopied(IComponent* otherComp) override;
		virtual void OnComponentDestroy() override;
		virtual void OnComponentDataValueEditChange(uint32_t dataId, std::any oldValue, int32_t elementIndex = -1) override;
		virtual void PostComponentDataValueEditChange(uint32_t dataId, std::any oldValue, int32_t elementIndex = -1) override;
		virtual void PostDataDeserialize(uint32_t dataId) override;
		virtual BoxSphereBounds GetBounds() override;

		void InitLight();
	};

}
