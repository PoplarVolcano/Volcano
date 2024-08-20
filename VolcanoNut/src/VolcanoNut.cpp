#include "Volcano.h"
#include "Volcano/EntryPoint.h"

#include "EditorLayer.h"
#include "ExampleLayer.h"

class VolcanoNut : public Volcano::Application
{
public:
	VolcanoNut(const Volcano::ApplicationSpecification& spec)
		: Application(spec)
	{
		PushLayer(new Volcano::ExampleLayer());
	}

	~VolcanoNut() {

	}
};

//创建应用
Volcano::Application* Volcano::CreateApplication(ApplicationCommandLineArgs args)
{
	ApplicationSpecification spec;
	spec.Name = "Volcanonut";
	spec.CommandLineArgs = args;

	return new VolcanoNut(spec);
}