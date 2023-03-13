#include "EditorLayer.h"
#include <Proton\Core\EntryPoint.h>

namespace Proton
{
	class ProtonEditor : public Application
	{
	public:
		ProtonEditor(const ApplicationSpecification& specification)
			: Application(specification)
		{
			PushLayer(new EditorLayer());
		}

		~ProtonEditor()
		{

		}
	};

	Application* Proton::CreateApplication(ApplicationCommandLineArgs args)
	{
		Project::New()->SaveActive("Sandbox.pproj");
		ApplicationSpecification spec;
		spec.Name = "Proton Editor";
		spec.CommandLineArgs = args;

		return new ProtonEditor(spec);
	}
}