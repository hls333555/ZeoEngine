#include "ZeoEngine.h"

#include "Engine/Core/EntryPoint.h"

#include "EditorLayer.h"
#include "Test/TestLayer.h"

namespace ZeoEngine {

	class ZeoEditor : public Application
	{
	public:
		ZeoEditor()
			: Application("Zeo Editor")
		{
			PushLayer(new EditorLayer());
			PushLayer(new TestLayer());
		}

		~ZeoEditor()
		{
		}

	};

	Application* CreateApplication()
	{
		return new ZeoEditor();
	}
}
