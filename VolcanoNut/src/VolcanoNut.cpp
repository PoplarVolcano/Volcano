#include "Volcano.h"
#include "Volcano/EntryPoint.h"

//#include "EditorLayer.h"
#include "ExampleLayer.h"

class VolcanoNut : public Volcano::Application
{
public:
	VolcanoNut(const Volcano::ApplicationSpecification& spec)
		: Application(spec)
	{
		PushLayer(new Volcano::ExampleLayer());
	}
};

//����Ӧ��
Volcano::Application* Volcano::CreateApplication(ApplicationCommandLineArgs args)
{
	ApplicationSpecification spec;
	spec.Name = "VolcanoNut";
	spec.WorkingDirectory = "../VolcanoNut";
	spec.CommandLineArgs = args;

	return new VolcanoNut(spec);
}