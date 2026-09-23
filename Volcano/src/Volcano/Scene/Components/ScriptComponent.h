#pragma once

namespace Volcano
{

	struct ScriptComponent
	{
		std::string ClassName;

		bool enabled = true;

		ScriptComponent() = default;
		ScriptComponent(const ScriptComponent&) = default;
	};

}