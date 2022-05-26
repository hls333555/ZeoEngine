#pragma once

#include <magic_enum.hpp>

#include "Engine/Renderer/Bindable.h"
#include "Engine/Core/Core.h"

namespace ZeoEngine {

	class BindableState : public Bindable {};

	class BindableStateLibrary
	{
	public:
		BindableStateLibrary(const BindableStateLibrary&) = delete;
		BindableStateLibrary& operator=(const BindableStateLibrary&) = delete;

		template<class T, typename...Args>
		static Ref<T> Resolve(Args&&...args)
		{
			static_assert(std::is_base_of<BindableState, T>::value, "Can only resolve classes derived from BindableState!");
			return Get().ResolveImpl<T>(std::forward<Args>(args)...);
		}

	private:
		BindableStateLibrary() = default;

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

		static BindableStateLibrary& Get()
		{
			static BindableStateLibrary instance;
			return instance;
		}

	private:
		std::unordered_map<std::string, Ref<Bindable>> m_BindableStates;
	};

	class Depth : public BindableState
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
			return BindableStateLibrary::Resolve<Depth>(state);
		}

		static std::string GenerateUID(State state)
		{
			using namespace std::string_literals;
			const auto stateStr = magic_enum::enum_name(state);
			return typeid(Depth).name() + "#"s + std::string(stateStr);
		}

		virtual void Bind() const override;

	private:
		State m_State;
	};

	class DepthClamp : public BindableState
	{
	public:
		enum class State
		{
			Disable, Enable
		};

		explicit DepthClamp(State state)
			: m_State(state) {}

		static Ref<DepthClamp> Resolve(State state)
		{
			return BindableStateLibrary::Resolve<DepthClamp>(state);
		}

		static std::string GenerateUID(State state)
		{
			using namespace std::string_literals;
			const auto stateStr = magic_enum::enum_name(state);
			return typeid(DepthClamp).name() + "#"s + std::string(stateStr);
		}

		virtual void Bind() const override;

	private:
		State m_State;
	};

	class Blend : public BindableState
	{
	public:
		enum class State
		{
			Enable, Disable
		};

		explicit Blend(State state)
			: m_State(state) {}

		static Ref<Blend> Resolve(State state)
		{
			return BindableStateLibrary::Resolve<Blend>(state);
		}

		static std::string GenerateUID(State state)
		{
			using namespace std::string_literals;
			const auto stateStr = magic_enum::enum_name(state);
			return typeid(Blend).name() + "#"s + std::string(stateStr);
		}

		virtual void Bind() const override;

	private:
		State m_State;
	};

	class TwoSided : public BindableState
	{
	public:
		enum class State
		{
			CullBack, CullFront, Disable
		};

		explicit TwoSided(State state)
			: m_State(state) {}

		static Ref<TwoSided> Resolve(State state)
		{
			return BindableStateLibrary::Resolve<TwoSided>(state);
		}

		static std::string GenerateUID(State state)
		{
			using namespace std::string_literals;
			const auto stateStr = magic_enum::enum_name(state);
			return typeid(TwoSided).name() + "#"s + std::string(stateStr);
		}

		virtual void Bind() const override;

	private:
		State m_State;
	};

	class Clear : public BindableState
	{
	public:
		enum class State
		{
			ClearColorDepthStencil, ClearDepth
		};

		explicit Clear(State state)
			: m_State(state) {}

		static Ref<Clear> Resolve(State state)
		{
			return BindableStateLibrary::Resolve<Clear>(state);
		}

		static std::string GenerateUID(State state)
		{
			using namespace std::string_literals;
			const auto stateStr = magic_enum::enum_name(state);
			return typeid(Clear).name() + "#"s + std::string(stateStr);
		}

		virtual void Bind() const override;

	private:
		State m_State;
	};

}
