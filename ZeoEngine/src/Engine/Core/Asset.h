#pragma once

#include <entt.hpp>

#include "Engine/Core/EngineTypes.h"

namespace ZeoEngine {

	class IAsset
	{
	public:
		virtual ~IAsset() = default;

		virtual const std::string& GetID() const = 0;
		virtual void SetID(std::string ID) = 0;

		virtual void Serialize(const std::string& path) = 0;
		virtual void Deserialize() = 0;

		// TODO:
		virtual void Reload(bool bIsCreate) {}
	};

	template<typename AssetClass>
	class AssetBase : public IAsset, public std::enable_shared_from_this<AssetClass>
	{
		using std::enable_shared_from_this<AssetClass>::shared_from_this;

	protected:
		explicit AssetBase(std::string ID)
			: m_ID(std::move(ID)) {}

		AssetHandle<AssetClass> GetAssetHandle()
		{
			return AssetHandle<AssetClass>(std::static_pointer_cast<AssetClass>(shared_from_this()));
		}
		AssetHandle<const AssetClass> GetAssetHandle() const
		{
			return AssetHandle<AssetClass>(std::static_pointer_cast<const AssetClass>(shared_from_this()));
		}

	public:
		static constexpr AssetTypeId TypeId()
		{
			return entt::type_hash<AssetClass>::value();
		}

		/** For debug purposes only. */
		static constexpr auto TypeName()
		{
			auto view = entt::type_name<AssetClass>::value();
			auto separator = view.rfind("::");
			return view.substr(separator + 2, view.size() - separator);
		}

		virtual const std::string& GetID() const override final { return m_ID; }
		virtual void SetID(std::string ID) override final { m_ID = std::move(ID); }

	private:
		/** Asset path or ID */
		std::string m_ID;
	};

#define REGISTER_ASSET(assetClass, loaderStructBody, libraryClassBody)																\
	static_assert(std::is_base_of<AssetBase<assetClass>, assetClass>::value, "Asset class is not derived from 'AssetBase'!");		\
	struct ZE_CAT(assetClass, Loader) final																							\
	{																																\
		using result_type = Ref<assetClass>;																						\
		loaderStructBody																											\
	};																																\
	class ZE_CAT(assetClass, Library) : public AssetLibrary<ZE_CAT(assetClass, Library), assetClass, ZE_CAT(assetClass, Loader)>	\
	{																																\
	public:																															\
		libraryClassBody																											\
	};

}
