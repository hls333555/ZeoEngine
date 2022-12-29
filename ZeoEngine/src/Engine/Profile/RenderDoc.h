#pragma once

#include <dylib.hpp>
#include <renderdoc_app.h>

namespace ZeoEngine {

	class RenderDoc
	{
	public:
		RenderDoc();

		void ToggleEnableCapture();

		void StartFrameCapture() const;
		void StopFrameCapture();

	private:
		void SetCapturePath() const;

	private:
		dylib m_Lib;
		RENDERDOC_API_1_5_0* m_API = nullptr;
		bool m_bEnableCapture = false;
		bool m_bCaptureOneFrameOnly = false;
	};

}
