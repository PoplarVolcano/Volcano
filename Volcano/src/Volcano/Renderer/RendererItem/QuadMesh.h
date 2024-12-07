#pragma once

#include "Mesh.h"

namespace Volcano {

	class QuadMesh : public Mesh
	{
	public:
		static Scope<QuadMesh>& GetInstance();
		static Ref<QuadMesh> CloneRef();
		QuadMesh();

		void DrawQuad(glm::mat4 transform);
	private:
		static std::once_flag init_flag;
		static Scope<QuadMesh> m_instance;
	};
}