#pragma once

#include "Mesh.h"

namespace Volcano {

	class ModelMesh : public Mesh
	{
	public:
		ModelMesh(std::vector<MeshVertex> vertices, std::vector<uint32_t> indices);
	};
}