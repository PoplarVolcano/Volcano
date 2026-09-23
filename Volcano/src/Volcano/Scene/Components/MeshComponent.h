#pragma once
#include "Volcano/Renderer/RendererItem/Mesh.h"

namespace Volcano
{

	struct MeshComponent
	{
		bool enabled = true;

		MeshType meshType = MeshType::None;
		std::string modelPath = std::string();

		MeshComponent() = default;
		MeshComponent(const MeshComponent&) = default;
		void SetMeshType(MeshType meshType, std::string modelPath = std::string()) {
			this->meshType = meshType;
			this->modelPath = modelPath;
		}
	};

}