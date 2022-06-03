#include "ZEpch.h"
#include "Tools/RenderDoc/RenderDoc.h"

namespace ZeoEngine {

	RenderDoc::RenderDoc()
	{
		// TODO: Path should be moved to a global config
		// RenderDoc library must be loaded before creating the rendering context
		m_Lib.open("renderdoc", dylib::extension);
		const auto getAPI = m_Lib.get_function<int(RENDERDOC_Version, void**)>("RENDERDOC_GetAPI");
		// Make sure that the code is compatible with the current installed version
		ZE_CORE_ASSERT(getAPI(eRENDERDOC_API_Version_1_5_0, reinterpret_cast<void**>(&m_API)) == 1, "Render doc API is not compatible!");
		m_API->SetCaptureKeys(nullptr, 0);
		m_API->MaskOverlayBits(~eRENDERDOC_Overlay_Enabled, 0);
	}

	// Press F9 to trigger
	void RenderDoc::ToggleEnableCapture()
	{
		m_bEnableCapture = !m_bEnableCapture;
		ZE_CORE_INFO(m_bEnableCapture ? "Start capturing frames for RenderDoc" : "Stop capturing frames for RenderDoc");
	}

	void RenderDoc::StartFrameCapture() const
	{
		if (m_API && m_bEnableCapture)
		{
			m_API->StartFrameCapture(nullptr, nullptr);
		}
	}

	void RenderDoc::StopFrameCapture() const
	{
		if (m_API && m_bEnableCapture)
		{
			m_API->EndFrameCapture(nullptr, nullptr);
		}
	}
}
