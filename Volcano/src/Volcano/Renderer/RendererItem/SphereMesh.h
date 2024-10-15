#pragma once

#include "Volcano/Renderer/RendererItem/Mesh.h"

namespace Volcano {

	class SphereMesh : public Mesh
	{
	public:
		SphereMesh();
		void Draw() override;
	};
}