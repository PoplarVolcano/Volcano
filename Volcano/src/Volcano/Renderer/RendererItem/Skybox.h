#pragma once

#include "Volcano/Renderer/RendererItem/Mesh.h"

namespace Volcano {

	class Skybox : public Mesh
	{
	public:
		Skybox();
		~Skybox();

		void DrawSkybox();
		virtual void FlushInstances() override;
	};
}