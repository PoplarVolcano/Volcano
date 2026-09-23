#pragma once

#include "Volcano/Renderer/RendererItem/Mesh.h"

namespace Volcano {

	class Cylinder : public Mesh
	{
	public:
		Cylinder();
		~Cylinder();

		virtual void FlushInstances() override;
	};
}