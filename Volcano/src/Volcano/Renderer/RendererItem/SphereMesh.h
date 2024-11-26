#pragma once

#include "Volcano/Renderer/RendererItem/Mesh.h"

namespace Volcano {

	class SphereMesh : public Mesh
	{
	public:
		static Scope<SphereMesh>& GetInstance();
		static Ref<SphereMesh> CloneRef();
		SphereMesh();
		void Draw() override;

		void DrawSphere(glm::mat4 transform);
	private:
		static std::once_flag init_flag;
		static Scope<SphereMesh> m_instance;
	};
}