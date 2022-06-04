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
		virtual void OnComponentDataValueEditChange(U32 dataId, std::any oldValue, I32 elementIndex) {}
		/** Called only when this data is changed and deactivated in the editor. (e.g. AFTER dragging a slider to tweak the value) */
		virtual void PostComponentDataValueEditChange(U32 dataId, std::any oldValue, I32 elementIndex) {}

		/** Called after certain data has been deserialized. */
		virtual void PostDataDeserialize(U32 dataId) {}

		/** Calculate component bounds or return an invalid one. */
		virtual BoxSphereBounds GetBounds() { return {}; }

		Entity* GetOwnerEntity() const;

	protected:
		template<typename Component>
		U32 GetDataIdByName(const char* dataName) const
		{
			const U32 dataId = entt::hashed_string(dataName);
			const auto data = entt::resolve<Component>().data(dataId);
			if (!data)
			{
				ZE_CORE_ERROR("Failed to resolve data by name: {0}::{1}!", entt::type_name<Component>::value(), dataName);
				return 0;
			}
			return dataId;
		}

	private:
		struct Impl;
		Scope<Impl> m_Impl;
	};

	class TransformComponentHelper : public IComponentHelper
	{
	public:
		using IComponentHelper::IComponentHelper;

		virtual void OnComponentDataValueEditChange(U32 dataId, std::any oldValue, I32 elementIndex = -1) override;
		virtual void PostComponentDataValueEditChange(U32 dataId, std::any oldValue, I32 elementIndex = -1) override;
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
		virtual void OnComponentDataValueEditChange(U32 dataId, std::any oldValue, I32 elementIndex = -1) override;
		virtual void PostComponentDataValueEditChange(U32 dataId, std::any oldValue, I32 elementIndex = -1) override;
	};

	class ParticleSystemPreviewComponentHelper : public IComponentHelper
	{
	public:
		using IComponentHelper::IComponentHelper;

		virtual void OnComponentDataValueEditChange(U32 dataId, std::any oldValue, I32 elementIndex = -1) override;
		virtual void PostComponentDataValueEditChange(U32 dataId, std::any oldValue, I32 elementIndex = -1) override;
	};

	class MeshRendererComponentHelper : public IComponentHelper
	{
	public:
		using IComponentHelper::IComponentHelper;

		virtual void OnComponentAdded(bool bIsDeserialize) override;
		virtual void OnComponentCopied(IComponent* otherComp) override;
		virtual void PostComponentDataValueEditChange(U32 dataId, std::any oldValue, I32 elementIndex = -1) override;
		virtual void PostDataDeserialize(U32 dataId) override;
		virtual BoxSphereBounds GetBounds() override;
	};

	class LightComponentHelper : public IComponentHelper
	{
	public:
		using IComponentHelper::IComponentHelper;

		virtual void OnComponentAdded(bool bIsDeserialize) override;
		virtual void OnComponentCopied(IComponent* otherComp) override;
		virtual void OnComponentDestroy() override;
		virtual void OnComponentDataValueEditChange(U32 dataId, std::any oldValue, I32 elementIndex = -1) override;
		virtual void PostComponentDataValueEditChange(U32 dataId, std::any oldValue, I32 elementIndex = -1) override;
		virtual void PostDataDeserialize(U32 dataId) override;
		virtual BoxSphereBounds GetBounds() override;

		void InitLight();
	};

}
