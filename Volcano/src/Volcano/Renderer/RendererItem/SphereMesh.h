#pragma once

#include "Volcano/Renderer/RendererItem/MeshTemp.h"

namespace Volcano {

	class SphereMesh : public MeshTemp
	{
	public:
		SphereMesh();
		void Draw() override;
	};
}