#pragma once

#include "Volcano/Renderer/RendererItem/Mesh.h"

namespace Volcano {

	class CapsuleMesh : public Mesh
	{
	public:
		static Scope<CapsuleMesh>& GetInstance();
		static Ref<CapsuleMesh> CloneRef();

		CapsuleMesh();
		void Draw() override;


		void DrawCapsule(glm::mat4 transform);

	private:
		static std::once_flag init_flag;
		static Scope<CapsuleMesh> m_instance;
	};
}