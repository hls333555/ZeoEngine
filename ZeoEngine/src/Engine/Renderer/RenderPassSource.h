#pragma once

namespace ZeoEngine {

	class BufferResource;
	class Bindable;

	class RenderPassSource
	{
	protected:
		RenderPassSource(std::string name);

	public:
		const std::string& GetName() const { return m_Name; }

		virtual Ref<BufferResource> GetBufferResource() const;
		virtual Ref<Bindable> GetBindable() const;
		virtual void PostLinkValidate() const {}

	private:
		std::string m_Name;
	};

	template<class T>
	class RenderPassBufferSource : public RenderPassSource
	{
	public:
		RenderPassBufferSource(std::string name, Ref<T>& bufferResource)
			: RenderPassSource(std::move(name))
			, m_BufferResource(bufferResource) {}

		static Scope<RenderPassBufferSource> Create(std::string name, Ref<T>& bufferResource)
		{
			return CreateScope<RenderPassBufferSource>(std::move(name), bufferResource);
		}

		virtual Ref<BufferResource> GetBufferResource() const override
		{
			ZE_CORE_ASSERT(!m_HasLinked, "Mutable output bound twice: {0}", GetName());

			m_HasLinked = true;
			return m_BufferResource;
		}

	private:
		Ref<T>& m_BufferResource;
		mutable bool m_HasLinked = false;
	};

	template<class T>
	class RenderPassBindableSource : public RenderPassSource
	{
	public:
		RenderPassBindableSource(std::string name, Ref<T>& bindable)
			: RenderPassSource(std::move(name))
			, m_Bindable(bindable) {}

		static Scope<RenderPassBindableSource> Create(std::string name, Ref<T>& bindable)
		{
			return CreateScope<RenderPassBindableSource>(std::move(name), bindable);
		}

		virtual Ref<Bindable> GetBindable() const override
		{
			return m_Bindable;
		}

	private:
		Ref<T>& m_Bindable;
	};

}
