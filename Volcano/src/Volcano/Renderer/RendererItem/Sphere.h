#pragma once

#include "Volcano/Renderer/RendererItem/Mesh.h"

namespace Volcano {

	class Sphere : public Mesh
	{
	public:
		Sphere();
		~Sphere();

		virtual void FlushInstances() override;
	};
}