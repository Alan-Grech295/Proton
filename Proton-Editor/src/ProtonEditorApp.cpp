#include "EditorLayer.h"
#include <Proton\Core\EntryPoint.h>

namespace Proton
{
	class ProtonEditor : public Application
	{
	public:
		ProtonEditor()
		{
			PushLayer(new EditorLayer());
		}

		~ProtonEditor()
		{

		}
	};

	Application* Proton::CreateApplication()
	{
		return new ProtonEditor();
	}
}