#include "ZEpch.h"
#include "Engine/Profile/RenderDoc.h"

#include "Engine/Core/CommonPaths.h"
#include "Engine/Core/Console.h"
#include "Engine/Core/Project.h"
#include "Engine/Utils/EngineUtils.h"

namespace ZeoEngine {

	RenderDoc::RenderDoc()
	{
		try
		{
			// RenderDoc library must be loaded before creating the rendering context
			m_Lib.open("renderdoc", dylib::extension);
			const auto getAPI = m_Lib.get_function<int(RENDERDOC_Version, void**)>("RENDERDOC_GetAPI");
			int res = getAPI(eRENDERDOC_API_Version_1_5_0, reinterpret_cast<void**>(&m_API));
			// Make sure that the code is compatible with the current installed version
			ZE_CORE_ASSERT(res == 1, "Render doc API is not compatible!");

			m_API->SetCaptureKeys(nullptr, 0);
			m_API->MaskOverlayBits(~eRENDERDOC_Overlay_Enabled, 0);
			Project::GetProjectLoadedDelegate().connect<&RenderDoc::SetCapturePath>(this);

			Console::Get().RegisterCommand("profile.StartGPUCapture", [this](const std::vector<std::string>&)
			{
				m_bEnableCapture = true;
				m_bCaptureOneFrameOnly = true;
				ZE_CORE_INFO("Captured files have been saved to: '{0}'/", CommonPaths::GetGPUProfileOutputDirectory());
			}, "Start capturing one frame for RenderDoc.");
		}
		catch (const dylib::handle_error&)
		{
			ZE_CORE_ERROR("Failed to load renderdoc library!");
		}
		catch (const dylib::symbol_error&)
		{
			ZE_CORE_ERROR("Failed to get renderdoc API!");
		}		
	}

	// Press F9 to trigger
	void RenderDoc::ToggleEnableCapture()
	{
		m_bEnableCapture = !m_bEnableCapture;
		if (m_bEnableCapture)
		{
			ZE_CORE_INFO("Start capturing frames for RenderDoc");
		}
		else
		{
			ZE_CORE_INFO("Stop capturing frames for RenderDoc, captured files have been saved to: '{0}'/", CommonPaths::GetGPUProfileOutputDirectory());
		}
	}

	void RenderDoc::StartFrameCapture() const
	{
		if (m_API && m_bEnableCapture)
		{
			m_API->StartFrameCapture(nullptr, nullptr);
		}
	}

	void RenderDoc::StopFrameCapture()
	{
		if (m_API && m_bEnableCapture)
		{
			m_API->EndFrameCapture(nullptr, nullptr);
			if (m_bCaptureOneFrameOnly)
			{
				m_bEnableCapture = false;
				m_bCaptureOneFrameOnly = false;
			}
		}
	}

	void RenderDoc::SetCapturePath() const
	{
		if (m_API)
		{
			const std::string outputPathTemplate = fmt::format("{}/{}", CommonPaths::GetGPUProfileOutputDirectory(), EngineUtils::GetCurrentTimeAndDate());
			m_API->SetCaptureFilePathTemplate(outputPathTemplate.c_str());
		}
	}

}
