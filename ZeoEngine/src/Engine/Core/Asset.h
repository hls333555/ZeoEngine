#pragma once

#include <entt.hpp>

#include "Engine/Core/Core.h"
#include "Engine/Core/EngineTypes.h"

namespace ZeoEngine {

	class IAsset
	{
	public:
		virtual ~IAsset() = default;

		virtual std::string GetPath() const = 0;
		virtual void SetPath(const std::string& path) = 0;
		virtual std::string GetName() const = 0;

		virtual void Serialize(const std::string& path) = 0;
		virtual void Deserialize() = 0;

		virtual void Reload(bool bIsCreate) = 0;
	};

	template<typename AssetClass>
	class AssetBase : public IAsset, public std::enable_shared_from_this<AssetClass>
	{
	protected:
		explicit AssetBase(const std::string& path)
			: m_AssetPath(path) {}

	private:
		using std::enable_shared_from_this<AssetClass>::shared_from_this;

	public:
		AssetHandle<AssetClass> GetAssetHandle()
		{
			return AssetHandle<AssetClass>(std::static_pointer_cast<AssetClass>(shared_from_this()));
		}
		AssetHandle<const AssetClass> GetAssetHandle() const
		{
			return AssetHandle<AssetClass>(std::static_pointer_cast<const AssetClass>(shared_from_this()));
		}

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

		virtual std::string GetPath() const override { return m_AssetPath.GetPath(); }
		virtual void SetPath(const std::string& path) override { m_AssetPath = path; }
		virtual std::string GetName() const override { return m_AssetPath.GetName(); }

	private:
		AssetPath m_AssetPath;
	};

}
