#pragma once

#include "ZEpch.h"

namespace ZeoEngine {

	template <typename Signature>
	struct Delegate;

	template <typename... Args>
	struct Delegate<void(Args...)>
	{
		Delegate() = default;
		Delegate(const Delegate&) = delete;
		void operator=(const Delegate&) = delete;

		template <typename T>
		Delegate& operator+=(T&& callback)
		{
			Callbacks.emplace_back(new CallbackFn<T>(std::forward<T>(callback)));
			return *this;
		}
		template <typename T>
		Delegate& operator-=(T&& callback)
		{
			CallbackFn<T> tmp(std::forward<T>(callback));
			// NOTE: This function does not actually remove elements, it just move them to the back instead!
			auto it = std::remove_if(Callbacks.begin(), Callbacks.end(),
				[&](std::unique_ptr<BaseFn>& other) {
					return tmp.Compare(other.get());
				});
			Callbacks.erase(it, Callbacks.end());
			return *this;
		}

		void operator()(Args... args)
		{
			for (auto& callback : Callbacks) {
				callback->Invoke(args...);
			}
		}

		struct BaseFn
		{
			virtual ~BaseFn() {}
			virtual bool Compare(BaseFn* other) = 0;
			virtual void Invoke(Args... args) = 0;
		};

		template <typename T>
		struct CallbackFn : BaseFn
		{
			template <typename S>
			CallbackFn(S&& callback) : Callback(std::forward<S>(callback)) {}

			virtual bool Compare(BaseFn* other) override
			{
				CallbackFn<T>* tmp = dynamic_cast<CallbackFn<T>*>(other);
				return tmp && this->Callback == tmp->Callback;
			}

			virtual void Invoke(Args... args) override
			{
				return Callback(std::forward<Args>(args)...);
			}

			T Callback;
		};

		std::vector<std::unique_ptr<BaseFn>> Callbacks;
	};

	template <typename RC, typename Class, typename... Args>
	class MemberFunc
	{
	public:
		MemberFunc(Class* object, RC(Class::*memberFn)(Args...))
			: m_Object(object)
			, m_MemberFn(memberFn) {}

		RC operator()(Args... args) {
			return (m_Object->*this->m_MemberFn)(std::forward<Args>(args)...);
		}

		bool operator==(MemberFunc const& other) const
		{
			return m_Object == other.m_Object && m_MemberFn == other.m_MemberFn;
		}

		bool operator!=(MemberFunc const& other) const
		{
			return !(*this == other);
		}

	private:
		Class* m_Object;
		RC(Class::*m_MemberFn)(Args...);
	};

}
