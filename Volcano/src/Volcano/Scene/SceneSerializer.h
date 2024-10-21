#pragma once
#include "Scene.h"

namespace Volcano {

	class Animation;

	class SceneSerializer
	{
	public:
		SceneSerializer(const Ref<Scene>& scene);

		void Serialize(const std::string& filepath);
		void SerializeRuntime(const std::string& filepath);

		bool Deserialize(const std::string& filepath);
		bool DeserializeRuntime(const std::string& filepath);
	private:
		Ref<Scene> m_Scene;
	};

	class MeshSerializer
	{
	public:
		MeshSerializer(Ref<Entity> entity);

		bool Serialize(const std::filesystem::path filepath);
		bool Deserialize(const std::filesystem::path filepath);
	private:
		Ref<Entity> m_Entity;
	};

	class AnimationSerializer
	{
	public:
		AnimationSerializer(Ref<Animation> animation);

		bool Serialize(const std::filesystem::path filepath);
		bool Deserialize(const std::filesystem::path filepath);
	private:
		Ref<Animation> m_Animation;
	};
}