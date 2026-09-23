#pragma once

#include "imgui/imgui.h"
#include "imgui/imgui_internal.h"

namespace Volcano
{

	template<typename Func>
	inline void SceneHierarchyPanel::DrawVec3Control(const std::string& label, glm::vec3& values, float resetValue, float columnWidth, Func&& func)
	{
		ImGuiIO& io = ImGui::GetIO();
		auto boldFont = io.Fonts->Fonts[0];

		ImGui::PushID(label.c_str());

		ImGui::Columns(2);
		ImGui::SetColumnWidth(0, columnWidth);
		ImGui::Text(label.c_str());
		ImGui::NextColumn();

		ImGui::PushMultiItemsWidths(3, ImGui::CalcItemWidth());
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{ 0, 0 });

		//计算行高
		//float lineHeight = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 2.0f;
		float lineHeight = ImGui::GetFontSize() + ImGui::GetCurrentContext()->Style.FramePadding.y * 2.0f;
		ImVec2 buttonSize = { lineHeight + 3.0f, lineHeight };

		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.8f, 0.1f, 0.15f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.9f, 0.2f, 0.2f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.8f, 0.1f, 0.15f, 1.0f });
		ImGui::PushFont(boldFont);
		if (ImGui::Button("X", buttonSize))
		{
			values.x = resetValue;
			if constexpr (!std::is_same_v<std::decay_t<Func>, std::nullptr_t>)
			{
				std::forward<Func>(func)();
			}
		}
		ImGui::PopFont();
		ImGui::PopStyleColor(3);

		ImGui::SameLine();
		if (ImGui::DragFloat("##X", &values.x, 0.1f, 0.0f, 0.0f, "%.2f"))
		{
			if constexpr (!std::is_same_v<std::decay_t<Func>, std::nullptr_t>)
			{
				// 如果 func 不是 nullptr，则调用它
				std::forward<Func>(func)();
			}
		}
		ImGui::PopItemWidth();
		ImGui::SameLine();

		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.2f, 0.7f, 0.3f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.3f, 0.8f, 0.3f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.2f, 0.7f, 0.2f, 1.0f });
		ImGui::PushFont(boldFont);
		if (ImGui::Button("Y", buttonSize))
		{
			values.y = resetValue;
			if constexpr (!std::is_same_v<std::decay_t<Func>, std::nullptr_t>)
			{
				std::forward<Func>(func)();
			}
		}
		ImGui::PopFont();
		ImGui::PopStyleColor(3);

		ImGui::SameLine();
		if (ImGui::DragFloat("##Y", &values.y, 0.1f, 0.0f, 0.0f, "%.2f"))
		{
			if constexpr (!std::is_same_v<std::decay_t<Func>, std::nullptr_t>)
			{
				std::forward<Func>(func)();
			}
		}
		ImGui::PopItemWidth();
		ImGui::SameLine();

		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.1f, 0.25f, 0.8f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.2f, 0.35f, 0.9f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.1f, 0.25f, 0.8f, 1.0f });
		ImGui::PushFont(boldFont);
		if (ImGui::Button("Z", buttonSize))
		{
			values.z = resetValue;
			if constexpr (!std::is_same_v<std::decay_t<Func>, std::nullptr_t>)
			{
				std::forward<Func>(func)();
			}
		}
		ImGui::PopFont();
		ImGui::PopStyleColor(3);

		ImGui::SameLine();
		if (ImGui::DragFloat("##Z", &values.z, 0.1f, 0.0f, 0.0f, "%.2f"))
		{
			if constexpr (!std::is_same_v<std::decay_t<Func>, std::nullptr_t>)
			{
				std::forward<Func>(func)();
			}
		}
		ImGui::PopItemWidth();

		ImGui::PopStyleVar();

		ImGui::Columns(1);

		ImGui::PopID();

	}
}