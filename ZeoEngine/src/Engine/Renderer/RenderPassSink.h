#pragma once

namespace ZeoEngine {

	class RenderPassSource;

	class RenderPassSink
	{
	protected:
		explicit RenderPassSink(std::string name);

	public:
		virtual ~RenderPassSink() = default;

		const std::string& GetName() const { return m_Name; }
		const std::string& GetTargetRenderPassName() const { return m_TargetPassName; }
		const std::string& GetTargetOutputName() const { return m_TargetOutputName; }

		virtual void Bind(const RenderPassSource* source) = 0;
		virtual void PostLinkValidate() const;

		void SetLinkTarget(std::string targetPass, std::string targetOutput);

	protected:
		bool m_HasLinked = false;
	private:
		std::string m_Name;
		std::string m_TargetPassName;
		std::string m_TargetOutputName;
	};

	template<typename T>
	class RenderPassBufferSink : public RenderPassSink
	{
		static_assert(std::is_base_of_v<BufferResource, T>, "RenderPassBufferSink target type must be a BufferResource type!");

	public:
		RenderPassBufferSink(std::string name, Ref<T>& bind)
			: RenderPassSink(std::move(name))
			, m_BindTarget(bind) {}

		static Scope<RenderPassSink> Create(std::string name, Ref<T>& target)
		{
			return CreateScope<RenderPassBufferSink>(std::move(name), target);
		}

		virtual void Bind(const RenderPassSource* source) override
		{
			if (!source) return;

			auto bufferResource = std::dynamic_pointer_cast<T>(source->GetBufferResource());
			if (!bufferResource)
			{
				ZE_CORE_ERROR("Failed to bind render pass input {0} to output {1}.{2}! {3} is not compatible with {4}.",
					GetName(), GetTargetRenderPassName(), GetTargetOutputName(), typeid(T).name(), typeid(*source->GetBufferResource().get()).name());
				return;
			}

			m_BindTarget = std::move(bufferResource);
			m_HasLinked = true;
		}

	private:
		Ref<T>& m_BindTarget;
	};

	template<typename T>
	class RenderPassBindableSink : public RenderPassSink
	{
		static_assert(std::is_base_of_v<Bindable, T>, "RenderPassBindableSink target type must be a Bindable type!");

	public:
		RenderPassBindableSink(std::string name, Ref<T>& bind)
			: RenderPassSink(std::move(name))
			, m_BindTarget(bind) {}

		static Scope<RenderPassSink> Create(std::string name, Ref<T>& target)
		{
			return CreateScope<RenderPassBindableSink>(std::move(name), target);
		}

		virtual void Bind(const RenderPassSource* source) override
		{
			if (!source) return;

			auto bindable = std::dynamic_pointer_cast<T>(source->GetBindable());
			if (!bindable)
			{
				ZE_CORE_ERROR("Failed to bind render pass input {0} to output {1}.{2}! {3} is not compatible with {4}.",
					GetName(), GetTargetRenderPassName(), GetTargetOutputName(), typeid(T).name(), typeid(*source->GetBindable().get()).name());
				return;
			}

			m_BindTarget = std::move(bindable);
			m_HasLinked = true;
		}

	private:
		Ref<T>& m_BindTarget;
	};

}
