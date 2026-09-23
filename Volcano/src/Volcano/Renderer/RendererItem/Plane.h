#pragma once

#include "Volcano/Renderer/RendererItem/Mesh.h"

namespace Volcano {

	class Plane : public Mesh
	{
	public:
		Plane();
		~Plane();

		virtual void FlushInstances() override;
	};
}