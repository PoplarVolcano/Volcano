#include "Volcano.h"
#include "Volcano/EntryPoint.h"

#include "ExampleLayer.h"
//#include "Game/GameLayer.h"

class VolcanoNutApplication : public Volcano::Application
{
public:
	VolcanoNutApplication(const Volcano::ApplicationProps& props)
		: Application(props)
	{
	}

	virtual void OnInit() override
	{
		PushLayer(new Volcano::ExampleLayer());
		//PushLayer(new Volcano::GameLayer());
	}

	~VolcanoNutApplication() {

	}
};

//创建应用
Volcano::Application* Volcano::CreateApplication() {
	return new VolcanoNutApplication({ "Volcano", 1600, 900 });
}