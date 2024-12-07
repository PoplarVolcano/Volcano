#pragma once

#include "Volcano/Renderer/RendererItem/Mesh.h"

namespace Volcano {

	class CylinderMesh : public Mesh
	{
	public:
		CylinderMesh();
		void Draw() override;

		static Scope<CylinderMesh>& GetInstance();
		static Ref<CylinderMesh> CloneRef();

		void DrawCylinder(glm::mat4 transform);
	private:
		static std::once_flag init_flag;
		static Scope<CylinderMesh> m_instance;
	};
}