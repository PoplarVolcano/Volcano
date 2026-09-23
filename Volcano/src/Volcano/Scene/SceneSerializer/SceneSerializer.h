#pragma once

#include "Volcano/Core/Core.h"
#include "Volcano/Scene/Scene.h"

namespace Volcano {

	class VOL_API SceneSerializer
	{
	public:
		SceneSerializer(const Ref<Scene>& scene);

		void Serialize(const std::string& filepath);
		bool Deserialize(const std::string& filepath);

	private:
		Ref<Scene> m_Scene;
	};

}