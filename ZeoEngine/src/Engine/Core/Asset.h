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

		virtual void Reload() = 0;
	};

	template<typename AssetClass>
	class AssetBase : public IAsset
	{
	protected:
		explicit AssetBase(const std::string& path)
			: m_AssetPath(path) {}

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

		virtual std::string GetPath() const override { return m_AssetPath.GetPath(); }
		virtual void SetPath(const std::string& path) override { m_AssetPath = path; }
		virtual std::string GetName() const override { return m_AssetPath.GetName(); }

	private:
		AssetPath m_AssetPath;
	};

}
