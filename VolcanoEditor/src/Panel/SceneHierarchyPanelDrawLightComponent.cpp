#include "SceneHierarchyPanel.h"

namespace Volcano
{
	void SceneHierarchyPanel::DrawLightComponent(LightComponent& component)
	{

		ImGui::Checkbox("Enable", &component.enabled);

		bool shadow = component.shadowEnabled == 0 ? false : true;
		if (ImGui::Checkbox("Shadow", &shadow))
		{
			component.shadowEnabled = shadow ? 1 : 0;
		}

		ImGui::Columns(2);
		ImGui::SetColumnWidth(0, 100.0f);
		ImGui::Text("LightType");
		ImGui::NextColumn();

		const char* lightTypeStrings[] = { "DirectionalLight", "PointLight", "SpotLight" };
		const char* currentLightTypeString = lightTypeStrings[(int)component.type];
		if (ImGui::BeginCombo("##LightType", currentLightTypeString))
		{
			for (int i = 0; i < 3; i++)
			{
				bool isSelected = currentLightTypeString == lightTypeStrings[i];
				if (ImGui::Selectable(lightTypeStrings[i], isSelected))
				{
					if (!isSelected)
					{
						currentLightTypeString = lightTypeStrings[i];
						component.type = (LightComponent::LightType)i;
					}
				}
				if (isSelected)
					ImGui::SetItemDefaultFocus();
			}
			ImGui::EndCombo();
		}

		ImGui::Columns(1);

		if (component.type == LightComponent::LightType::DirectionalLight)
		{
			DrawVec3Control("Ambient", component.ambient, 0.0f);
			DrawVec3Control("Diffuse", component.diffuse, 0.5f);
			DrawVec3Control("Specular", component.specular, 1.0f);
		}

		if (component.type == LightComponent::LightType::PointLight)
		{

			ImGui::DragFloat("Radius", &component.radius, 0.1f, 0.0f, 1000.0f, "%.3f");
			DrawVec3Control("Ambient", component.ambient, 0.0f);
			DrawVec3Control("Diffuse", component.diffuse, 0.5f);
			DrawVec3Control("Specular", component.specular, 1.0f);
			ImGui::DragFloat("Constant", &component.constant, 0.1f, 0.0f, 0.0f, "%.3f");
			ImGui::DragFloat("Linear", &component.linear, 0.1f, 0.0f, 0.0f, "%.3f");
			ImGui::DragFloat("Quadratic", &component.quadratic, 0.1f, 0.0f, 0.0f, "%.3f");
		}

		if (component.type == LightComponent::LightType::SpotLight)
		{
			ImGui::DragFloat("Radius", &component.radius, 0.1f, 0.0f, 0.0f, "%.3f");

			if (ImGui::DragFloat("CutOff Angle", &component.cutoffAngle, 0.1f, 0.0f, 0.0f, "%.3f"))
			{
				component.cutoff = glm::cos(glm::radians(component.cutoffAngle));
			}
			if (ImGui::DragFloat("Outer Cutoff Angle", &component.outerCutoffAngle, 0.1f, 0.0f, 0.0f, "%.3f"))
			{
				component.outerCutoff = glm::cos(glm::radians(component.outerCutoffAngle));
			}
			DrawVec3Control("Ambient", component.ambient, 0.0f);
			DrawVec3Control("Diffuse", component.diffuse, 0.5f);
			DrawVec3Control("Specular", component.specular, 1.0f);
			ImGui::DragFloat("Constant", &component.constant, 0.1f, 0.0f, 0.0f, "%.3f");
			ImGui::DragFloat("Linear", &component.linear, 0.1f, 0.0f, 0.0f, "%.3f");
			ImGui::DragFloat("Quadratic", &component.quadratic, 0.1f, 0.0f, 0.0f, "%.3f");
			float cutoff = glm::degrees(glm::acos(component.cutoff));
			float outerCutOff = glm::degrees(glm::acos(component.outerCutoff));
			ImGui::DragFloat("Cutoff", &cutoff, 0.1f, 0.0f, 0.0f, "%.3f");
			ImGui::DragFloat("OuterCutoff", &outerCutOff, 0.1f, 0.0f, 0.0f, "%.3f");
			component.cutoff = glm::cos(glm::radians(cutoff));
			component.outerCutoff = glm::cos(glm::radians(outerCutOff));
		}

	}
}