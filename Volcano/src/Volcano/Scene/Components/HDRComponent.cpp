#include "volpch.h"
#include "HDRComponent.h"

#include "Volcano/Renderer/SceneRenderer.h"

namespace Volcano
{
	void HDRComponent::UpdateEnvCubeMap()
	{
		SceneRenderer::UpdateEnvCubeMap(this);
	}

}