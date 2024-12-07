#pragma once

#include "Mesh.h"

namespace Volcano {

	class PlaneMesh : public Mesh
	{
	public:
		PlaneMesh();

		static Scope<PlaneMesh>& GetInstance();
		static Ref<PlaneMesh> CloneRef();

		void DrawPlane(glm::mat4 transform);
	private:
		static std::once_flag init_flag;
		static Scope<PlaneMesh> m_instance;
	};
}