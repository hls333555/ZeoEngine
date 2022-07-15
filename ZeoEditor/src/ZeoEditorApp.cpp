#include "ZeoEngine.h"

#include "Engine/Core/EntryPoint.h"

#include "EditorLayer.h"
#include "Test/TestLayer.h"

namespace ZeoEngine {

	class ZeoEditor : public Application
	{
	public:
		ZeoEditor(const ApplicationSpecification& spec)
			: Application(spec)
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
		ApplicationSpecification spec;
		spec.Name = "ZeoEditor";
		spec.CommandLineArgs = args;
		return new ZeoEditor(spec);
	}
}
