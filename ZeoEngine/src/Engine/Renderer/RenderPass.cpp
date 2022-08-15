#include "ZEpch.h"
#include "Engine/Renderer/RenderPass.h"

#include "Engine/Utils/EngineUtils.h"
#include "Engine/Renderer/BindableStates.h"
#include "Engine/Renderer/RenderCommand.h"
#include "Engine/Renderer/SceneSettings.h"
#include "Engine/Asset/AssetLibrary.h"

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

	void ComputePass::Execute() const
	{
		if (!IsActive()) return;

		BindComputeShader();
		for (const auto& stage : m_ComputeStages)
		{
			stage.TextureToRead->GetTexture()->BindAsImage(stage.TextureToRead->BindingSlot, true);
			stage.TextureToWrite->GetTexture()->BindAsImage(stage.TextureToWrite->BindingSlot, false);
			RenderCommand::DispatchCompute(stage.DispatchParams.x, stage.DispatchParams.y, stage.DispatchParams.z);
			RenderCommand::SetImageAccessBarrier();
		}
		UnbindComputeShader();
	}

	void ComputePass::AddComputeStage(ComputeStage stage)
	{
		m_ComputeStages.emplace_back(std::move(stage));
	}

	void ComputePass::BindComputeShader() const
	{
		m_ComputeShader->Bind();
	}

	void ComputePass::UnbindComputeShader() const
	{
		m_ComputeShader->Unbind();
	}

	BindingPass::BindingPass(std::string name, bool bAutoActive)
		: RenderPass(std::move(name), bAutoActive)
	{
		// Init default states
		AddBindable(Blend::Resolve(Blend::State::Enable));
		AddBindable(Depth::Resolve(Depth::State::ReadWrite));
		AddBindable(DepthClamp::Resolve(DepthClamp::State::Disable));
		AddBindable(TwoSided::Resolve(TwoSided::State::CullBack));
	}

	void BindingPass::AddBindable(Ref<Bindable> bindable)
	{
		// For bindable states, previous state will be replaced by the new state
		if (dynamic_cast<BindableState*>(bindable.get()))
		{
			const auto it = std::find_if(m_Bindables.begin(), m_Bindables.end(), [&bindable](const Ref<Bindable>& _bindable)
			{
				return _bindable && typeid(*_bindable) == typeid(*bindable);
			});
			if (it != m_Bindables.end())
			{
				*it = std::move(bindable);
				return;
			}
		}
		m_Bindables.emplace_back(std::move(bindable));
	}

	void BindingPass::BindAll() const
	{
		BindBufferResource();
		BindBindables();
	}

	void BindingPass::BindBindables() const
	{
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

	Ref<Bindable> BindingPass::GetBindableByIndex(U32 index)
	{
		if (index >= m_Bindables.size()) return nullptr;

		return m_Bindables[index];
	}

	void BindingPass::Finalize()
	{
		RenderPass::Finalize();
		if (!m_FBO)
		{
			ZE_CORE_ERROR("BindingPass {0} needs a FrameBuffer!");
		}
	}

	void BindingPass::OnViewportResize(U32 width, U32 height) const
	{
		m_FBO->Resize(width, height);
	}

	void BindingPass::BindBufferResource() const
	{
		m_FBO->BindAsBuffer();
	}

	void RenderQueuePass::AddTask(const Drawable* drawable, const RenderStep* step)
	{
		const SubRenderTask subTask(drawable);
		for (auto& task : m_Tasks)
		{
			if (task.HasSameMaterial(step))
			{
				task.AddSubTask(subTask);
				return;
			}
		}

		RenderTask task(step);
		task.AddSubTask(subTask);
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

		AddBindable(AssetLibrary::LoadAsset<Shader>("assets/editor/shaders/ShadowMap.glsl.zasset"));
		// Front face culling can perfectly solve Shadow Acne and Peter Panning artifacts but it will have issues with thin objects
		//AddBindable(TwoSided::Resolve(TwoSided::State::CullFront));
		// Enable depth clamping so that the shadow maps keep from moving through objects which causes shadows to disappear
		AddBindable(DepthClamp::Resolve(DepthClamp::State::Enable));
		AddBindable(Clear::Resolve(Clear::State::ClearDepth));

		RegisterOutput(RenderPassBindableOutput<FrameBuffer>::Create("ShadowMap", m_FBO));
	}

	void ShadowMappingPass::CreateDepthBuffer()
	{
		FrameBufferSpec fbSpec;
		fbSpec.Width = SceneSettings::ShadowMapResolution;
		fbSpec.Height = SceneSettings::ShadowMapResolution;
		fbSpec.bFixedSize = true;
		fbSpec.Attachments = {
			{ TextureFormat::DEPTH32F, { SamplerType::ShadowDepth, SamplerType::ShadowPCF }, SceneSettings::MaxCascades }
		};
		s_FBO = m_FBO = FrameBuffer::Create(fbSpec, 0, TextureBinding::ShadowMap);
	}

	Ref<FrameBuffer> ScreenSpaceShadowPass::s_FBO = nullptr;
	Ref<ShaderInstance> ScreenSpaceShadowPass::s_ShadowShader = nullptr;

	ScreenSpaceShadowPass::ScreenSpaceShadowPass(std::string name, bool bAutoActive)
		: RenderQueuePass(std::move(name), bAutoActive)
	{
		CreateShadowBuffer();

		RegisterBindableInput<FrameBuffer>("ShadowMap");
		// NOTE: Bind shader after all its required bindings being bound properly!
		// Or OpenGL warnings will keep popping up
		s_ShadowShader = AssetLibrary::LoadAsset<Shader>("assets/editor/shaders/ScreenSpaceShadow.glsl.zasset")->CreateInstance();
		AddBindable(s_ShadowShader);
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
			{ TextureFormat::DEPTH24STENCIL8 }
		};
		s_FBO = m_FBO = FrameBuffer::Create(fbSpec, 0, TextureBinding::ScreenSpaceShadowMap);
	}

	OpaqueRenderPass::OpaqueRenderPass(std::string name, bool bShouldClearIDBuffer, bool bAutoActive)
		: RenderQueuePass(std::move(name), bAutoActive)
		, m_bShouldClearIDBuffer(bShouldClearIDBuffer)
	{
		CreateFrameBuffer();

		RegisterBindableInput<FrameBuffer>("ShadowMap");
		AddBindable(Depth::Resolve(Depth::State::ReadWrite));
		AddBindable(TwoSided::Resolve(TwoSided::State::CullBack));
		AddBindable(Clear::Resolve(Clear::State::ClearColorDepthStencil));

		RegisterOutput(RenderPassBufferOutput<FrameBuffer>::Create("FrameBuffer", m_FBO));
		RegisterOutput(RenderPassBindableOutput<FrameBuffer>::Create("SceneTexture", m_FBO));
	}

	void OpaqueRenderPass::CreateFrameBuffer()
	{
		FrameBufferSpec fbSpec;
		fbSpec.Attachments = {
			{ TextureFormat::RGBA16F }, // 16f reduces banding
			{ TextureFormat::RGBA16F }, // Entity ID buffer
			{ TextureFormat::DEPTH24STENCIL8 }
		};
		m_FBO = FrameBuffer::Create(fbSpec, 0, TextureBinding::SceneTexture);
	}

	void OpaqueRenderPass::ClearFrameBufferAttachment() const
	{
		if (!m_bShouldClearIDBuffer) return;

		// Clear entity ID buffer to -1
		m_FBO->ClearColorAttachment(1, { -1.0f, 0.0f, 0.0f, 0.0f });
	}

	GridRenderPass::GridRenderPass(std::string name, bool bAutoActive)
		: BindingPass(std::move(name), bAutoActive)
	{
		struct GridData
		{
			Mat4 Transform = Mat4(1.0f);
			Vec4 ThinLinesColor{ 0.2f, 0.2f, 0.2f, 0.1f };
			Vec4 ThickLinesColor{ 0.3f, 0.3f, 0.3f, 0.5f };
			Vec4 OriginAxisXColor{ 1.0f, 0.0f, 0.0f, 0.5f };
			Vec4 OriginAxisZColor{ 0.0f, 0.0f, 1.0f, 0.5f };
			float Extent = 101.0f;
			float CellSize = 0.025f;
		};
		GridData gridBuffer;
		auto gridUniformBuffer = UniformBuffer::Create(sizeof(GridData), static_cast<U32>(UniformBufferBinding::Grid));
		gridUniformBuffer->SetData(&gridBuffer);

		AddBindable(gridUniformBuffer);
		AddBindable(AssetLibrary::LoadAsset<Shader>("assets/editor/shaders/Grid.glsl.zasset"));
		AddBindable(Depth::Resolve(Depth::State::ReadOnly));
		AddBindable(TwoSided::Resolve(TwoSided::State::Disable));

		RegisterInput(RenderPassBufferInput<FrameBuffer>::Create("FrameBuffer", m_FBO));
		RegisterOutput(RenderPassBindableOutput<FrameBuffer>::Create("SceneTexture", m_FBO));
	}

	void GridRenderPass::Execute() const
	{
		if (!IsActive()) return;

		BindAll();
		static constexpr I32 gridInstanceCount = 10;
		RenderCommand::DrawInstanced(gridInstanceCount);
		UnbindBindables();
	}

	PostProcessingPass::PostProcessingPass(std::string name, bool bAutoActive)
		: FullscreenPass(std::move(name), bAutoActive)
	{
		m_SceneBufferIndex = RegisterBindableInput<FrameBuffer>("SceneTexture");
		AddBindable(AssetLibrary::LoadAsset<Shader>("assets/editor/shaders/PostProcessing.glsl.zasset"));
		AddBindable(Clear::Resolve(Clear::State::ClearColorDepthStencil));

		RegisterInput(RenderPassBufferInput<FrameBuffer>::Create("BackFrameBuffer", m_FBO));
		RegisterOutput(RenderPassBufferOutput<FrameBuffer>::Create("FrameBuffer", m_FBO));
	}

	void PostProcessingPass::Execute() const
	{
		FullscreenPass::Execute();

		// Copy ID buffer and depth buffer
		m_SceneBuffer->BlitColorTo(m_FBO, 1, 1);
		m_SceneBuffer->BlitDepthTo(m_FBO);
	}

	void PostProcessingPass::Finalize()
	{
		FullscreenPass::Finalize();

		m_SceneBuffer = std::dynamic_pointer_cast<FrameBuffer>(GetBindableByIndex(static_cast<U32>(m_SceneBufferIndex)));
	}

}
