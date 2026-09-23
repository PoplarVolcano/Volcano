#include "Volcano.h"
#include "Volcano/EntryPoint.h"

#include "VolcanoEditorLayer/VolcanoEditorLayer.h"

class VolcanoEditor : public Volcano::Application
{
public:
	VolcanoEditor()
	{
		PushLayer(new Volcano::VolcanoEditorLayer());
	}
};

//创建应用
Volcano::Application* Volcano::CreateApplication()
{
	return new VolcanoEditor();
}