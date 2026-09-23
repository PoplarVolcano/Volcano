#pragma once

#include "Volcano/Renderer/RendererItem/Mesh.h"

namespace Volcano {

	// 胶囊
	class Capsule : public Mesh
	{
	public:
		Capsule();
		~Capsule();

		virtual void FlushInstances() override;
	};
}