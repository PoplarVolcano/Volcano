#pragma once

#include "Volcano/Renderer/RendererItem/Mesh.h"

namespace Volcano {

	class Cone : public Mesh
	{
	public:
		Cone();
		~Cone();

		virtual void FlushInstances() override;
	};
}