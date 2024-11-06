#pragma once

#include "Volcano/Renderer/RendererItem/Mesh.h"

namespace Volcano {

	class CylinderMesh : public Mesh
	{
	public:
		CylinderMesh();
		void Draw() override;
	};
}