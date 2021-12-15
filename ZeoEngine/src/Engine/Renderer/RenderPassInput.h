#pragma once

namespace ZeoEngine {

	class RenderPassOutput;

	class RenderPassInput
	{
	protected:
		explicit RenderPassInput(std::string name);

	public:
		virtual ~RenderPassInput() = default;

		const std::string& GetName() const { return m_Name; }
		const std::string& GetTargetRenderPassName() const { return m_TargetPassName; }
		const std::string& GetTargetOutputName() const { return m_TargetOutputName; }

		virtual void Bind(const RenderPassOutput* output) = 0;
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
	class RenderPassBufferInput : public RenderPassInput
	{
		static_assert(std::is_base_of_v<BufferResource, T>, "RenderPassBufferInput target type must be a BufferResource type!");

	public:
		RenderPassBufferInput(std::string name, Ref<T>& bind)
			: RenderPassInput(std::move(name))
			, m_BindTarget(bind) {}

		static Scope<RenderPassInput> Create(std::string name, Ref<T>& target)
		{
			return CreateScope<RenderPassBufferInput>(std::move(name), target);
		}

		virtual void Bind(const RenderPassOutput* output) override
		{
			if (!output) return;

			auto bufferResource = std::dynamic_pointer_cast<T>(output->GetBufferResource());
			if (!bufferResource)
			{
				ZE_CORE_ERROR("Failed to bind render pass input {0} to output {1}.{2}! {3} is not compatible with {4}.",
					GetName(), GetTargetRenderPassName(), GetTargetOutputName(), typeid(T).name(), typeid(*output->GetBufferResource().get()).name());
				return;
			}

			m_BindTarget = std::move(bufferResource);
			m_HasLinked = true;
		}

	private:
		Ref<T>& m_BindTarget;
	};

	template<typename T>
	class RenderPassBindableInput : public RenderPassInput
	{
		static_assert(std::is_base_of_v<Bindable, T>, "RenderPassBindableInput target type must be a Bindable type!");

	public:
		RenderPassBindableInput(std::string name, Ref<T>& bind)
			: RenderPassInput(std::move(name))
			, m_BindTarget(bind) {}

		static Scope<RenderPassInput> Create(std::string name, Ref<T>& target)
		{
			return CreateScope<RenderPassBindableInput>(std::move(name), target);
		}

		virtual void Bind(const RenderPassOutput* output) override
		{
			if (!output) return;

			auto bindable = std::dynamic_pointer_cast<T>(output->GetBindable());
			if (!bindable)
			{
				ZE_CORE_ERROR("Failed to bind render pass input {0} to output {1}.{2}! {3} is not compatible with {4}.",
					GetName(), GetTargetRenderPassName(), GetTargetOutputName(), typeid(T).name(), typeid(*output->GetBindable().get()).name());
				return;
			}

			m_BindTarget = std::move(bindable);
			m_HasLinked = true;
		}

	private:
		Ref<T>& m_BindTarget;
	};

}
