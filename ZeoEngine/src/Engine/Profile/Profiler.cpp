#include "ZEpch.h"
#include "Engine/Profile/Profiler.h"

#include "Engine/Core/Console.h"
#include "Engine/Asset/AssetRegistry.h"
#include "Engine/Utils/EngineUtils.h"
#include "Engine/Utils/PathUtils.h"

namespace ZeoEngine {

	void Optick::StartCapture(std::string name)
	{
		ZE_CORE_INFO("Start capturing frames for Optick");
		OPTICK_START_CAPTURE();
		m_CaptureName = std::move(name);
	}

	void Optick::StopCapture() const
	{
		OPTICK_STOP_CAPTURE();
		std::string outputPath = AssetRegistry::GetCPUProfileOutputDirectory();
		if (!PathUtils::Exists(outputPath))
		{
			PathUtils::CreateDirectory(outputPath);
		}
		outputPath += m_CaptureName;
		OPTICK_SAVE_CAPTURE(outputPath.c_str());
		ZE_CORE_INFO("Stop capturing frames for Optick, captured files have been saved to: {0}", outputPath);
	}

	Profiler::Profiler()
	{
		Console::Get().RegisterCommand("p.StartCPUCapture", [this](const std::vector<std::string>& params)
		{
			std::string captureName = params.empty() ? "capture" : params[0];
			m_Optick.StartCapture(std::move(captureName));
		}, "Start capturing frames for Optick. Param1(optional): Name of the capture");

		Console::Get().RegisterCommand("p.StopCPUCapture", [this](const std::vector<std::string>&)
		{
			m_Optick.StopCapture();
		}, "Stop capturing frames for Optick.");
	}

}
