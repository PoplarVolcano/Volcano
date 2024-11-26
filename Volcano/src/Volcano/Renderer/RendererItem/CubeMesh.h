#pragma once

#include "Mesh.h"

namespace Volcano {

	class CubeMesh : public Mesh
	{
	public:
		static Scope<CubeMesh>& GetInstance();
		static Ref<CubeMesh> CloneRef();
		CubeMesh();

		void DrawCube(glm::mat4 transform);
	private:
		static std::once_flag init_flag;
		static Scope<CubeMesh> m_instance;
	};
}