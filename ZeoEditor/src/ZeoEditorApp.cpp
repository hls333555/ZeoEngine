#include "ZeoEngine.h"

#include "Engine/Core/EntryPoint.h"

#include "EditorLayer.h"

namespace ZeoEngine {

	class ZeoEditor : public Application
	{
	public:
		ZeoEditor()
			: Application("Zeo Editor")
		{
			PushLayer(new EditorLayer());
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
