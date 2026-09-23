#include "SceneHierarchyPanel.h"

namespace Volcano
{

	void SceneHierarchyPanel::DrawCameraComponent(CameraComponent& component)
	{
		ImGui::Checkbox("Enabled", &component.enabled);

		auto& camera = component.Camera;

		ImGui::Checkbox("Primary", &component.primary);

		const char* projectionTypeStrings[] = { "Prespective", "Orthographic" };
		const char* currentProjectionTypeString = projectionTypeStrings[(int)camera.GetProjectionType()];
		if (ImGui::BeginCombo("Projection", currentProjectionTypeString))
		{
			for (int i = 0; i < 2; i++)
			{
				bool isSelected = currentProjectionTypeString == projectionTypeStrings[i];
				if (ImGui::Selectable(projectionTypeStrings[i], isSelected))
				{
					if (!isSelected)
					{
						currentProjectionTypeString = projectionTypeStrings[i];
						camera.SetProjectionType((SceneCamera::ProjectionType)i);
					}
				}
				if (isSelected)
					ImGui::SetItemDefaultFocus();
			}
			ImGui::EndCombo();
		}

		if (camera.GetProjectionType() == SceneCamera::ProjectionType::Prespective)
		{
			float verticalFOV = glm::degrees(camera.GetPerspectiveVerticalFOV());
			if (ImGui::DragFloat("Vertical FOV", &verticalFOV))
				camera.SetPerspectiveVerticalFOV(glm::radians(verticalFOV));

			float perspNear = camera.GetPerspectiveNearClip();
			if (ImGui::DragFloat("Near", &perspNear))
				camera.SetPerspectiveNearClip(perspNear);

			float perspFar = camera.GetPerspectiveFarClip();
			if (ImGui::DragFloat("Far", &perspFar))
				camera.SetPerspectiveFarClip(perspFar);
		}

		if (camera.GetProjectionType() == SceneCamera::ProjectionType::Orthographic)
		{
			float orthoSize = camera.GetOrthographicSize();
			if (ImGui::DragFloat("Size", &orthoSize))
				camera.SetOrthographicSize(orthoSize);

			float orthoNear = camera.GetOrthographicNearClip();
			if (ImGui::DragFloat("Near", &orthoNear))
				camera.SetOrthographicNearClip(orthoNear);

			float orthoFar = camera.GetOrthographicFarClip();
			if (ImGui::DragFloat("Far", &orthoFar))
				camera.SetOrthographicFarClip(orthoFar);

			ImGui::Checkbox("Fixed Aspect Ratio", &component.fixedAspectRatio);
		}
	}
}