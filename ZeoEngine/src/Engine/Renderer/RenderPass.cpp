#include "ZEpch.h"
#include "Engine/Renderer/RenderPass.h"

#include "Engine/Utils/EngineUtils.h"
#include "Engine/Renderer/BindableStates.h"
#include "Engine/Renderer/RenderCommand.h"
#include "Engine/Renderer/SceneSettings.h"

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

	void BindingPass::UnbindBindables() const
	{
		for (const auto& bindable : m_Bindables)
		{
			bindable->Unbind();
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
		ClearFrameBufferAttachment();
		ExecuteTasks();
		UnbindBindables();
	}

	void RenderQueuePass::Reset()
	{
		m_Tasks.clear();
	}

	void FullscreenPass::Execute() const
	{
		if (!IsActive()) return;

		BindAll();
		RenderCommand::DrawArrays(6);
		UnbindBindables();
	}

	Ref<FrameBuffer> ShadowMappingPass::s_FBO = nullptr;

	ShadowMappingPass::ShadowMappingPass(std::string name, bool bAutoActive)
		: RenderQueuePass(std::move(name), bAutoActive)
	{
		CreateDepthBuffer();

		AddBindable(ShaderLibrary::Get().LoadAsset("assets/editor/shaders/ShadowMap.glsl").to_ref());
		// Front face culling can perfectly solve Shadow Acne and Peter Panning artifacts but it will have issues with thin objects
		//AddBindable(TwoSided::Resolve(TwoSided::State::CullFront));
		// Enable depth clamping so that the shadow maps keep from moving through objects which causes shadows to disappear
		AddBindable(Depth::Resolve(Depth::State::ToggleClamp));
		AddBindable(Clear::Resolve(Clear::State::ClearDepth));

		RegisterOutput(RenderPassBindableOutput<FrameBuffer>::Create("ShadowMap", m_FBO));
	}

	void ShadowMappingPass::CreateDepthBuffer()
	{
		FrameBufferSpec fbSpec;
		fbSpec.Width = SceneSettings::ShadowMapResolution;
		fbSpec.Height = SceneSettings::ShadowMapResolution;
		fbSpec.Attachments = {
			{ TextureFormat::DEPTH32F, { SamplerType::ShadowDepth, SamplerType::ShadowPCF }, SceneSettings::MaxCascades }
		};
		s_FBO = m_FBO = FrameBuffer::Create(fbSpec, 0, static_cast<uint32_t>(TextureBinding::ShadowMap));
	}

	Ref<FrameBuffer> ScreenSpaceShadowPass::s_FBO = nullptr;

	ScreenSpaceShadowPass::ScreenSpaceShadowPass(std::string name, bool bAutoActive)
		: RenderQueuePass(std::move(name), bAutoActive)
	{
		CreateShadowBuffer();

		RegisterBindableInput<Bindable>("ShadowMap");
		// NOTE: Bind shader after all its required bindings being bound properly!
		// Or OpenGL warnings will keep poping up
		AddBindable(ShaderLibrary::Get().LoadAsset("assets/editor/shaders/ScreenSpaceShadow.glsl").to_ref());
		AddBindable(Depth::Resolve(Depth::State::ReadWrite));
		AddBindable(TwoSided::Resolve(TwoSided::State::CullBack));
		AddBindable(Clear::Resolve(Clear::State::ClearColorDepthStencil));

		RegisterOutput(RenderPassBindableOutput<FrameBuffer>::Create("ShadowMap", m_FBO));
	}

	void ScreenSpaceShadowPass::CreateShadowBuffer()
	{
		FrameBufferSpec fbSpec;
		fbSpec.Attachments = {
			{ TextureFormat::RGBA8, { SamplerType::BilinearClamp } },
			{ TextureFormat::DEPTH24STENCIL8, { SamplerType::BilinearClamp } }
		};
		s_FBO = m_FBO = FrameBuffer::Create(fbSpec, 0, static_cast<uint32_t>(TextureBinding::ScreenSpaceShadowMap));
	}

	Ref<FrameBuffer> HorizontalBlurPass::s_FBO = nullptr;

	HorizontalBlurPass::HorizontalBlurPass(std::string name, bool bAutoActive)
		: FullscreenPass(std::move(name), bAutoActive)
	{
		CreateHorizontalBlurBuffer();

		RegisterBindableInput<Bindable>("ShadowMap");
		AddBindable(ShaderLibrary::Get().LoadAsset("assets/editor/shaders/HorizontalBlur.glsl").to_ref());
		AddBindable(Clear::Resolve(Clear::State::ClearColorDepthStencil));

		RegisterOutput(RenderPassBindableOutput<FrameBuffer>::Create("ShadowMap", m_FBO));
	}

	void HorizontalBlurPass::CreateHorizontalBlurBuffer()
	{
		FrameBufferSpec fbSpec;
		fbSpec.Attachments = {
			{ TextureFormat::RGBA8, { SamplerType::BilinearClamp } },
			{ TextureFormat::DEPTH24STENCIL8, { SamplerType::BilinearClamp } }
		};
		s_FBO = m_FBO = FrameBuffer::Create(fbSpec, 0, static_cast<uint32_t>(TextureBinding::ScreenSpaceShadowMap));
	}

	Ref<FrameBuffer> VerticalBlurPass::s_FBO = nullptr;

	VerticalBlurPass::VerticalBlurPass(std::string name, bool bAutoActive)
		: FullscreenPass(std::move(name), bAutoActive)
	{
		CreateVerticalBlurBuffer();

		RegisterBindableInput<Bindable>("ShadowMap");
		AddBindable(ShaderLibrary::Get().LoadAsset("assets/editor/shaders/VerticalBlur.glsl").to_ref());
		AddBindable(Clear::Resolve(Clear::State::ClearColorDepthStencil));

		RegisterOutput(RenderPassBindableOutput<FrameBuffer>::Create("ShadowMap", m_FBO));
	}

	void VerticalBlurPass::CreateVerticalBlurBuffer()
	{
		FrameBufferSpec fbSpec;
		fbSpec.Attachments = {
			{ TextureFormat::RGBA8, { SamplerType::BilinearClamp } },
			{ TextureFormat::DEPTH24STENCIL8, { SamplerType::BilinearClamp } }
		};
		s_FBO = m_FBO = FrameBuffer::Create(fbSpec, 0, static_cast<uint32_t>(TextureBinding::ScreenSpaceShadowMap));
	}

	OpaqueRenderPass::OpaqueRenderPass(std::string name, bool bAutoActive)
		: RenderQueuePass(std::move(name), bAutoActive)
	{
		RegisterBindableInput<Bindable>("ShadowMap");
		AddBindable(Depth::Resolve(Depth::State::ReadWrite));
		AddBindable(TwoSided::Resolve(TwoSided::State::CullBack));
		AddBindable(Clear::Resolve(Clear::State::ClearColorDepthStencil));
		
		RegisterInput(RenderPassBufferInput<FrameBuffer>::Create("FrameBuffer", m_FBO));
		RegisterOutput(RenderPassBufferOutput<FrameBuffer>::Create("FrameBuffer", m_FBO));
	}

	void OpaqueRenderPass::ClearFrameBufferAttachment() const
	{
		// Clear entity ID buffer to -1
		m_FBO->ClearColorAttachment(1, { -1.0f, 0.0f, 0.0f, 0.0f });
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
		auto gridUniformBuffer = UniformBuffer::Create(sizeof(GridData), static_cast<uint32_t>(UniformBufferBinding::Grid));
		gridUniformBuffer->SetData(&gridBuffer);

		AddBindable(gridUniformBuffer);
		AddBindable(ShaderLibrary::Get().LoadAsset("assets/editor/shaders/Grid.glsl").to_ref());
		AddBindable(Depth::Resolve(Depth::State::ReadOnly));
		AddBindable(TwoSided::Resolve(TwoSided::State::Disable));

		RegisterInput(RenderPassBufferInput<FrameBuffer>::Create("FrameBuffer", m_FBO));
		RegisterOutput(RenderPassBufferOutput<FrameBuffer>::Create("FrameBuffer", m_FBO));
	}

	void GridRenderPass::Execute() const
	{
		if (!IsActive()) return;

		BindAll();
		static constexpr int32_t gridInstanceCount = 10;
		RenderCommand::DrawInstanced(gridInstanceCount);
		UnbindBindables();
	}

}
