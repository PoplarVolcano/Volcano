#pragma once

#include "Volcano/Renderer/RendererItem/Mesh.h"

namespace Volcano {

	class Cube : public Mesh
	{
	public:
		Cube();
		~Cube();

		virtual void FlushInstances() override;
	};
}