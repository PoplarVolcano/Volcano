#pragma once

#include "Volcano/Renderer/RendererItem/Mesh.h"

namespace Volcano {

	class Quad : public Mesh
	{
	public:
		Quad();
		~Quad();

		virtual void FlushInstances() override;
	};
}