#pragma once

#include "Volcano/Renderer/Texture.h"
#include "Volcano/Renderer/RendererItem/Mesh.h"

namespace Volcano
{

	struct SkyboxComponent
	{

		bool enabled = true;

		bool primary = false;
		int textureType = 0; // 0: TextureCube, 1: Texture2D * 6

		std::array<TextureLibraryKey, 6> textures;
		Ref<TextureCube> textureCubeMap = Mesh::GetBlackTextureCube();
		Ref<TextureCube> textureCubeSixSided = Mesh::GetBlackTextureCube();

		SkyboxComponent() = default;
		SkyboxComponent(const SkyboxComponent&) = default;

		void ResetTextureCubeMap()
		{
			textureCubeMap = Mesh::GetBlackTextureCube();
		}

		void ResetTextureCubeSixSided()
		{
			std::array<Ref<Texture2D>, 6> faces;
			for (int i = 0; i != 6; i++)
			{
				if (textures[i].key.empty())
				{
					faces[i] = nullptr;
				}
				else
				{
					faces[i] = Texture::GetTextureLibrary()->Get(
						textures[i].key,
						textures[i].keyHash,
						textures[i].flip
					);
				}
			}

			bool isFacesEmpty = true;
			for (const auto& face : faces)
			{
				if (face != nullptr)
				{
					isFacesEmpty = false;
					break;
				}
			}
			if (isFacesEmpty)
			{
				textureCubeSixSided = Mesh::GetBlackTextureCube();
			}
			else
			{
				textureCubeSixSided = TextureCube::Create(faces);
			}

		}
	};

}