#include "ZEpch.h"
#include "Engine/Renderer/RenderPass.h"

#include "Engine/Utils/EngineUtils.h"
#include "Engine/Renderer/BindableStates.h"
#include "Engine/Renderer/RenderCommand.h"

namespace ZeoEngine {

	RenderPass::RenderPass(std::string name, bool bAutoActive)
		: m_Name(std::move(name)), m_bIsActive(bAutoActive)
	{
	}

	RenderPassInput* RenderPass::GetInput(const std::string& name) const
	{
		for (const auto& input : m_Inputs)
		{
			if (input->GetName() == name)
			{
				return input.get();
			}
		}

		ZE_CORE_ERROR("Render pass input {0} not found in pass {1}!", m_Name);
		return nullptr;
	}

	RenderPassOutput* RenderPass::GetOuput(const std::string& name) const
	{
		for (const auto& output : m_Outputs)
		{
			if (output->GetName() == name)
			{
				return output.get();
			}
		}

		ZE_CORE_ERROR("Render pass output {0} not found in pass {1}!", m_Name);
		return nullptr;
	}

	void RenderPass::Finalize()
	{
		for (const auto& input : m_Inputs)
		{
			input->PostLinkValidate();
		}
		for (const auto& output : m_Outputs)
		{
			output->PostLinkValidate();
		}
	}

	void RenderPass::SetInputLinkage(const std::string& inputName, const std::string& targetOutputName)
	{
		auto* input = GetInput(inputName);
		if (!input) return;

		auto targetSplit = EngineUtils::SplitString(targetOutputName, '.');
		if (targetSplit.size() != 2)
		{
			ZE_CORE_ERROR("Failed to link input to output with incorrect target output name format \"{0}\"!", targetOutputName);
			return;
		}

		input->SetLinkTarget(std::move(targetSplit[0]), std::move(targetSplit[1]));
	}

	void RenderPass::RegisterInput(Scope<RenderPassInput> input)
	{
		for (const auto& i : m_Inputs)
		{
			if (i->GetName() == input->GetName())
			{
				ZE_CORE_ERROR("Render pass input {0} already registered in pass {1}!", input->GetName(), m_Name);
				return;
			}
		}
		m_Inputs.emplace_back(std::move(input));
	}

	void RenderPass::RegisterOutput(Scope<RenderPassOutput> output)
	{
		for (const auto& o : m_Outputs)
		{
			if (o->GetName() == output->GetName())
			{
				ZE_CORE_ERROR("Render pass output {0} already registered in pass {1}!", output->GetName(), m_Name);
				return;
			}
		}
		m_Outputs.emplace_back(std::move(output));
	}

	void BindingPass::AddBindable(Ref<Bindable> bindable)
	{
		m_Bindables.emplace_back(std::move(bindable));
	}

	void BindingPass::BindAll() const
	{
		BindBufferResource();
		for (const auto& bindable : m_Bindables)
		{
			bindable->Bind();
		}
	}

	void BindingPass::Finalize()
	{
		RenderPass::Finalize();
		if (!m_FBO)
		{
			ZE_CORE_ERROR("BindingPass {0} needs a FrameBuffer!");
		}
	}

	void BindingPass::BindBufferResource() const
	{
		m_FBO->BindAsBuffer();
	}

	void RenderQueuePass::AddTask(RenderTask task)
	{
		// TODO:
		m_Tasks.emplace_back(std::move(task));
	}

	void RenderQueuePass::ExecuteTasks() const
	{
		for (const auto& task : m_Tasks)
		{
			task.Execute();
		}
	}

	void RenderQueuePass::Execute() const
	{
		if (!IsActive()) return;

		BindAll();
		ExecuteTasks();
	}

	void RenderQueuePass::Reset()
	{
		m_Tasks.clear();
	}

	static const uint32_t Shadow_Width = 1024, Shadow_Height = 1024;

	ShadowMappingPass::ShadowMappingPass(std::string name, bool bAutoActive)
		: RenderQueuePass(std::move(name), bAutoActive)
	{
		CreateDepthBuffer();
		AddBindable(Shader::Create("assets/editor/shaders/Depth.glsl"));
		// Front face culling can perfectly solve Shadow Acne and Peter Panning artifacts but it will have issues with thin objects
		//AddBindable(TwoSided::Resolve(TwoSided::State::CullFront));
		RegisterOutput(RenderPassBindableOutput<FrameBuffer>::Create("ShadowMap", m_FBO));
	}

	void ShadowMappingPass::Execute() const
	{
		if (!IsActive()) return;

		BindAll();
		RenderCommand::Clear(RendererAPI::ClearType::Depth);
		ExecuteTasks();
	}

	void ShadowMappingPass::CreateDepthBuffer()
	{
		FrameBufferSpec fbSpec;
		fbSpec.Width = Shadow_Width;
		fbSpec.Height = Shadow_Height;
		fbSpec.Attachments = { FrameBufferTextureFormat::DEPTH32F };
		m_FBO = FrameBuffer::Create(fbSpec, 0);
	}

	OpaqueRenderPass::OpaqueRenderPass(std::string name, bool bAutoActive)
		: RenderQueuePass(std::move(name), bAutoActive)
	{
		RegisterBindableInput<Bindable>("ShadowMap");
		RegisterInput(RenderPassBufferInput<FrameBuffer>::Create("FrameBuffer", m_FBO));
		RegisterOutput(RenderPassBufferOutput<FrameBuffer>::Create("FrameBuffer", m_FBO));
		AddBindable(Depth::Resolve(Depth::State::ReadWrite));
		AddBindable(TwoSided::Resolve(TwoSided::State::CullBack));
	}

	GridRenderPass::GridRenderPass(std::string name, bool bAutoActive)
		: BindingPass(std::move(name), bAutoActive)
	{
		struct GridData
		{
			glm::mat4 Transform = glm::mat4(1.0f);
			glm::vec4 ThinLinesColor{ 0.2f, 0.2f, 0.2f, 0.3f };
			glm::vec4 ThickLinesColor{ 0.5f, 0.5f, 0.5f, 0.3f };
			glm::vec4 OriginAxisXColor{ 1.0f, 0.0f, 0.0f, 0.3f };
			glm::vec4 OriginAxisZColor{ 0.0f, 0.0f, 1.0f, 0.3f };
			float Extent = 101.0f;
			float CellSize = 0.025f;
		};
		GridData gridBuffer;
		auto gridUniformBuffer = UniformBuffer::Create(sizeof(GridData), 1);
		gridUniformBuffer->SetData(&gridBuffer);

		RegisterInput(RenderPassBufferInput<FrameBuffer>::Create("FrameBuffer", m_FBO));
		RegisterOutput(RenderPassBufferOutput<FrameBuffer>::Create("FrameBuffer", m_FBO));
		AddBindable(Shader::Create("assets/editor/shaders/Grid.glsl"));
		AddBindable(gridUniformBuffer);
		AddBindable(Depth::Resolve(Depth::State::ReadOnly));
		AddBindable(TwoSided::Resolve(TwoSided::State::Disable));
	}

	void GridRenderPass::Execute() const
	{
		if (!IsActive()) return;

		BindAll();
		static int32_t gridInstanceCount = 10;
		RenderCommand::DrawInstanced(gridInstanceCount);
	}

}
