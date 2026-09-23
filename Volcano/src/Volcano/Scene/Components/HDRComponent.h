#pragma once

#include "Volcano/Core/Core.h"
#include "Volcano/Renderer/Texture.h"
#include "Volcano/Renderer/RendererItem/Mesh.h"

namespace Volcano
{
	struct HDRComponent
	{

		bool enabled = true;

		bool primary = false;

		// 等距柱状投影图
		TextureLibraryKey equirectangularMapKey;
		// 环境光立方体贴图
		Ref<TextureCube> envCubeMap = nullptr;
		// 辐照度立方体贴图
		Ref<TextureCube> irradianceMap = nullptr;
		// 预滤波环境贴图
		Ref<TextureCube> prefilterMap = nullptr;

		HDRComponent() = default;
		HDRComponent(const HDRComponent&) = default;

		void VOL_API UpdateEnvCubeMap();
	};

}