#pragma once

#include "Engine/Core/Core.h"
#include "Engine/Math/BoxSphereBounds.h"

namespace ZeoEngine {

	struct IComponent;
	class Entity;
	class RenderGraph;

	class ComponentHelperRegistry
	{
	public:
		template<typename ComponentHelper, typename Component>
		static void RegisterComponentHelper()
		{
			AddComponentHelper(entt::type_hash<Component>::value(), CreateScope<ComponentHelper>());
		}
		static class IComponentHelper* GetComponentHelper(U32 compID);

	private:
		static void AddComponentHelper(U32 compID, Scope<IComponentHelper> helper);
	};

	class IComponentHelper
	{
	public:
		/** Called after component being added to the owner entity. If bIsDeserialize is true, the component is added during deserialization. */
		virtual void OnComponentAdded(IComponent* comp, bool bIsDeserialize) {}
		/** Called after component being copied to the owner entity. You should handle logic here instead of in the component's copy ctor. */
		virtual void OnComponentCopied(IComponent* comp, IComponent* otherComp) {}
		/**
		 * Called before component being removed from the owner entity.
		 * There are mainly three ways to remove a component:
		 * Call Entity::RemoveComponent
		 * Call Entity::RemoveComponentByID
		 * Destroy the owner entity
		 */
		virtual void OnComponentDestroy(IComponent* comp) {}

		/**
		 * Called when this field is changed in the editor.
		 * @param comp - Current component used for retrieving fields
		 * @param fieldID - Current field ID, use GetFieldIDByName() to identify
		 * @param oldValue - Value before editing. For containers, this value is container element value
		 * @param elementIndex - Only valid when current field is a sequence container
		 */
		virtual void PostComponentFieldValueEditChange(IComponent* comp, U32 fieldID, const void* oldValue, U32 elementIndex) {}

		/** Called after certain field has been deserialized. */
		virtual void PostFieldDeserialize(IComponent* comp, U32 fieldID) {}

		virtual std::string GetCustomSequenceContainerElementName(IComponent* comp, U32 index) const { return {}; }

		/** Calculate component bounds or return an invalid one. */
		virtual BoxSphereBounds GetBounds(IComponent* comp) { return {}; }

	protected:
		template<typename Component>
		U32 GetFieldIDByName(const char* name) const
		{
			const U32 fieldID = entt::hashed_string(name);
			const auto data = entt::resolve<Component>().data(fieldID);
			if (!data)
			{
				ZE_CORE_ERROR("Failed to resolve field by name: {0}::{1}!", entt::type_name<Component>::value(), name);
				return 0;
			}
			return fieldID;
		}
	};

	class TransformComponentHelper : public IComponentHelper
	{
	public:
		using IComponentHelper::IComponentHelper;

		virtual void PostComponentFieldValueEditChange(IComponent* comp, U32 fieldID, const void* oldValue, U32 elementIndex = -1) override;
	};

	class CameraComponentHelper : public IComponentHelper
	{
	public:
		using IComponentHelper::IComponentHelper;

		virtual void OnComponentAdded(IComponent* comp, bool bIsDeserialize) override;
		virtual void PostComponentFieldValueEditChange(IComponent* comp, U32 fieldID, const void* oldValue, U32 elementIndex = -1) override;
		virtual void PostFieldDeserialize(IComponent* comp, U32 fieldID) override;
		virtual void OnComponentDestroy(IComponent* comp) override;
	};

	class ScriptComponentHelper : public IComponentHelper
	{
	public:
		using IComponentHelper::IComponentHelper;

		virtual void OnComponentDestroy(IComponent* comp) override;
		virtual void PostComponentFieldValueEditChange(IComponent* comp, U32 fieldID, const void* oldValue, U32 elementIndex = -1) override;
		virtual void PostFieldDeserialize(IComponent* comp, U32 fieldID) override;
	};

	class ParticleSystemComponentHelper : public IComponentHelper
	{
	public:
		using IComponentHelper::IComponentHelper;

		virtual void OnComponentCopied(IComponent* comp, IComponent* otherComp) override;
		virtual void OnComponentDestroy(IComponent* comp) override;
		virtual void PostComponentFieldValueEditChange(IComponent* comp, U32 fieldID, const void* oldValue, U32 elementIndex = -1) override;
	};

	class ParticleSystemPreviewComponentHelper : public IComponentHelper
	{
	public:
		using IComponentHelper::IComponentHelper;

		virtual void OnComponentDestroy(IComponent* comp) override;
		virtual void PostComponentFieldValueEditChange(IComponent* comp, U32 fieldID, const void* oldValue, U32 elementIndex = -1) override;
	};

	class MeshRendererComponentHelper : public IComponentHelper
	{
	public:
		using IComponentHelper::IComponentHelper;

		virtual void OnComponentAdded(IComponent* comp, bool bIsDeserialize) override;
		virtual void OnComponentCopied(IComponent* comp, IComponent* otherComp) override;
		virtual void PostComponentFieldValueEditChange(IComponent* comp, U32 fieldID, const void* oldValue, U32 elementIndex = -1) override;
		virtual void PostFieldDeserialize(IComponent* comp, U32 fieldID) override;
		virtual BoxSphereBounds GetBounds(IComponent* comp) override;
		virtual std::string GetCustomSequenceContainerElementName(IComponent* comp, U32 index) const override;
	};

	class MeshPreviewComponentHelper : public IComponentHelper
	{
	public:
		using IComponentHelper::IComponentHelper;

		virtual void PostComponentFieldValueEditChange(IComponent* comp, U32 fieldID, const void* oldValue, U32 elementIndex = -1) override;
		virtual BoxSphereBounds GetBounds(IComponent* comp) override;
		virtual std::string GetCustomSequenceContainerElementName(IComponent* comp, U32 index) const override;
	};

	class MaterialPreviewComponentHelper : public IComponentHelper
	{
	public:
		using IComponentHelper::IComponentHelper;

		virtual void PostComponentFieldValueEditChange(IComponent* comp, U32 fieldID, const void* oldValue, U32 elementIndex = -1) override;
		virtual void PostFieldDeserialize(IComponent* comp, U32 fieldID) override;
	};

	class TexturePreviewComponentHelper : public IComponentHelper
	{
	public:
		using IComponentHelper::IComponentHelper;

		virtual void PostComponentFieldValueEditChange(IComponent* comp, U32 fieldID, const void* oldValue, U32 elementIndex = -1) override;
	};

	class LightComponentHelper : public IComponentHelper
	{
	public:
		using IComponentHelper::IComponentHelper;

		virtual void OnComponentAdded(IComponent* comp, bool bIsDeserialize) override;
		virtual void OnComponentCopied(IComponent* comp, IComponent* otherComp) override;
		virtual void OnComponentDestroy(IComponent* comp) override;
		virtual void PostComponentFieldValueEditChange(IComponent* comp, U32 fieldID, const void* oldValue, U32 elementIndex = -1) override;
		virtual void PostFieldDeserialize(IComponent* comp, U32 fieldID) override;
		virtual BoxSphereBounds GetBounds(IComponent* comp) override;

		void InitLight(Entity* entity) const;
	};

}
