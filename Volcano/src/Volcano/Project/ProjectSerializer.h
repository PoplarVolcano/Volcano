#pragma once

#include "Project.h"

namespace Volcano {

	class ProjectSerializer
	{
	public:
		ProjectSerializer(Ref<Project> project);

		bool Serialize(const std::filesystem::path& projectFileAbsolutePath);
		bool Deserialize(const std::filesystem::path& projectFileAbsolutePath);
	private:
		Ref<Project> m_Project;
	};

}