#include "volpch.h"
#include "LightRenderer.h"

namespace Volcano
{
	LightRenderer::LightRenderer()
	{
		m_DirectionalLightBuffer = new DirectionalLight[MaxDirectionalLight];
		m_PointLightBuffer = new PointLight[MaxPointLight];
		m_SpotLightBuffer = new SpotLight[MaxSpotLight];
	}

	LightRenderer::~LightRenderer()
	{
		delete[] m_DirectionalLightBuffer;
		delete[] m_PointLightBuffer;
		delete[] m_SpotLightBuffer;
	}
}