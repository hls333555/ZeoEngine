#pragma once

#include <entt.hpp>

#include "Engine/Core/Core.h"
#include "Engine/Core/EngineTypes.h"
#include "Engine/Core/UUID.h"

namespace ZeoEngine {

	using AssetTypeID = U32;
	using AssetHandle = UUID;

	class IAsset
	{
	public:
		virtual ~IAsset() = default;

		virtual AssetTypeID GetTypeID() const = 0;
		virtual AssetHandle GetHandle() const = 0;
		virtual void SetHandle(AssetHandle handle) = 0;
		virtual const entt::sigh<void()>& GetAssetReloadedDelegate() const = 0;
	};

	template<typename AssetClass>
	class AssetBase : public IAsset, public std::enable_shared_from_this<AssetClass>
	{
		using std::enable_shared_from_this<AssetClass>::shared_from_this;

	protected:
		Ref<AssetClass> SharedFromThis()
		{
			return std::static_pointer_cast<AssetClass>(shared_from_this());
		}
		Ref<const AssetClass> SharedFromThis() const
		{
			return std::static_pointer_cast<const AssetClass>(shared_from_this());
		}

	public:
		static constexpr AssetTypeID TypeID()
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

		virtual AssetTypeID GetTypeID() const override final { return TypeID(); }
		virtual AssetHandle GetHandle() const override final { return m_Handle; }
		virtual void SetHandle(AssetHandle handle) override final { m_Handle = handle; }
		virtual const entt::sigh<void()>& GetAssetReloadedDelegate() const override final { return m_OnAssetReloadedDel; }

		virtual bool operator==(const AssetBase& other) const
		{
			return m_Handle == other.m_Handle;
		}

		virtual bool operator!=(const AssetBase& other) const
		{
			return !(*this == other);
		}

	public:
		entt::sink<entt::sigh<void()>> m_OnAssetReloaded{ m_OnAssetReloadedDel };
		entt::sigh<void()> m_OnAssetReloadedDel;

	private:
		AssetHandle m_Handle = 0;
	};

}
