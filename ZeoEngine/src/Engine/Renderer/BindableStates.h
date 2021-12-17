#pragma once

#include <magic_enum.hpp>

#include "Engine/Renderer/Bindable.h"
#include "Engine/Core/Core.h"

namespace ZeoEngine {

	class BindableStates
	{
	public:
		template<class T, typename...Args>
		static Ref<T> Resolve(Args&&...args)
		{
			static_assert(std::is_base_of<Bindable, T>::value, "Can only resolve classes derived from Bindable!");
			return Get().ResolveImpl<T>(std::forward<Args>(args)...);
		}

	private:
		BindableStates() = default;
		BindableStates(const BindableStates&) = delete;
		BindableStates& operator=(const BindableStates&) = delete;

		template<class T, typename...Args>
		Ref<T> ResolveImpl(Args&&...args)
		{
			const auto uid = T::GenerateUID(std::forward<Args>(args)...);
			const auto it = m_BindableStates.find(uid);
			if (it == m_BindableStates.end())
			{
				auto bindableState = CreateRef<T>(std::forward<Args>(args)...);
				m_BindableStates[uid] = bindableState;
				return bindableState;
			}
			else
			{
				return std::static_pointer_cast<T>(it->second);
			}
		}

		static BindableStates& Get()
		{
			static BindableStates instance;
			return instance;
		}

	private:
		std::unordered_map<std::string, Ref<Bindable>> m_BindableStates;
	};

	class Depth : public Bindable
	{
	public:
		enum class State
		{
			ReadWrite, ReadOnly, Disable
		};

		explicit Depth(State state)
			: m_State(state) {}

		static Ref<Depth> Resolve(State state)
		{
			return BindableStates::Resolve<Depth>(state);
		}

		static std::string GenerateUID(State state)
		{
			using namespace std::string_literals;
			auto stateStr = magic_enum::enum_name(state);
			return typeid(Depth).name() + "#"s + std::string(stateStr);
		}

		virtual void Bind(uint32_t slot = 0) const override;

	private:
		State m_State;
	};

	class TwoSided : public Bindable
	{
	public:
		explicit TwoSided(bool bEnable)
			: m_bEnable(bEnable) {}

		static Ref<TwoSided> Resolve(bool bEnable)
		{
			return BindableStates::Resolve<TwoSided>(bEnable);
		}

		static std::string GenerateUID(bool bEnable)
		{
			using namespace std::string_literals;
			return typeid(TwoSided).name() + "#"s + std::string(bEnable ? "Enable" : "Disable");
		}

		virtual void Bind(uint32_t slot = 0) const override;

	private:
		bool m_bEnable;
	};

}
