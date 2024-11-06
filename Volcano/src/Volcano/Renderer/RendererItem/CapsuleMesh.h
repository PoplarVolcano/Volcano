#pragma once

#include "Volcano/Renderer/RendererItem/Mesh.h"

namespace Volcano {

	class CapsuleMesh : public Mesh
	{
	public:
		CapsuleMesh();
		void Draw() override;
	};
}