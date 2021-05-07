#include "ZeoEngine.h"

#include "Engine/Core/EntryPoint.h"

#include "EditorLayer.h"
#include "Test/TestLayer.h"

namespace ZeoEngine {

	class ZeoEditor : public Application
	{
	public:
		ZeoEditor(ApplicationCommandLineArgs args)
			: Application("Zeo Editor", args)
		{
			PushLayer(new EditorLayer());
			PushLayer(new TestLayer());
		}

		~ZeoEditor()
		{
		}

	};

	Application* CreateApplication(ApplicationCommandLineArgs args)
	{
		return new ZeoEditor(args);
	}
}
