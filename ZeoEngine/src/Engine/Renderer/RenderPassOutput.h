#pragma once

namespace ZeoEngine {

	class BufferResource;
	class Bindable;

	class RenderPassOutput
	{
	protected:
		RenderPassOutput(std::string name);

	public:
		const std::string& GetName() const { return m_Name; }

		virtual Ref<BufferResource> GetBufferResource() const;
		virtual Ref<Bindable> GetBindable() const;
		virtual void PostLinkValidate() const {}

	private:
		std::string m_Name;
	};

	template<class T>
	class RenderPassBufferOutput : public RenderPassOutput
	{
	public:
		RenderPassBufferOutput(std::string name, Ref<T>& bufferResource)
			: RenderPassOutput(std::move(name))
			, m_BufferResource(bufferResource) {}

		static Scope<RenderPassBufferOutput> Create(std::string name, Ref<T>& bufferResource)
		{
			return CreateScope<RenderPassBufferOutput>(std::move(name), bufferResource);
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
	class RenderPassBindableOutput : public RenderPassOutput
	{
	public:
		RenderPassBindableOutput(std::string name, Ref<T>& bindable)
			: RenderPassOutput(std::move(name))
			, m_Bindable(bindable) {}

		static Scope<RenderPassBindableOutput> Create(std::string name, Ref<T>& bindable)
		{
			return CreateScope<RenderPassBindableOutput>(std::move(name), bindable);
		}

		virtual Ref<Bindable> GetBindable() const override
		{
			return m_Bindable;
		}

	private:
		Ref<T>& m_Bindable;
	};

}
