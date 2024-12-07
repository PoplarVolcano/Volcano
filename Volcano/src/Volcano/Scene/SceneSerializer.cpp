#include "volpch.h"
#include "SceneSerializer.h"

#include "Entity.h"
#include "Components.h"
#include "Volcano/Scripting/ScriptEngine.h"
#include "Volcano/Core/UUID.h"
#include "Volcano/Project/Project.h"
#include "Volcano/Renderer/RendererItem/Model.h"
#include "Volcano/Utils/FileUtils.h"
#include "Volcano/Utils/YAMLUtils.h"
#include "Volcano/Project/Project.h"
#include "Volcano/Scene/Prefab.h"
#include "Volcano/Renderer/RendererItem/QuadMesh.h"

#include <fstream>
#include <yaml-cpp/yaml.h>

namespace Volcano {

	static std::string RigidBody2DBodyTypeToString(Rigidbody2DComponent::BodyType bodyType)
	{
		switch (bodyType)
		{
		case Rigidbody2DComponent::BodyType::Static:    return "Static";
		case Rigidbody2DComponent::BodyType::Dynamic:   return "Dynamic";
		case Rigidbody2DComponent::BodyType::Kinematic: return "Kinematic";
		}

		VOL_CORE_ASSERT(false, "Unknown body type");
		return {};
	}

	static Rigidbody2DComponent::BodyType RigidBody2DBodyTypeFromString(const std::string& bodyTypeString)
	{
		if (bodyTypeString == "Static")    return Rigidbody2DComponent::BodyType::Static;
		if (bodyTypeString == "Dynamic")   return Rigidbody2DComponent::BodyType::Dynamic;
		if (bodyTypeString == "Kinematic") return Rigidbody2DComponent::BodyType::Kinematic;

		VOL_CORE_ASSERT(false, "Unknown body type");
		return Rigidbody2DComponent::BodyType::Static;
	}


	SceneSerializer::SceneSerializer(const Ref<Scene>& scene)
		: m_Scene(scene)
	{
	}

	void SerializeBoneNode(YAML::Emitter& out, AssimpNodeData& node, Animation& animation)
	{
		out << YAML::BeginMap;// BoneNode
		{
			out << YAML::Key << "Name" << YAML::Value << node.name;
			out << YAML::Key << "ID" << YAML::Value << animation.GetBoneIDMap()[node.name].id;
			out << YAML::Key << "NodeTransform" << YAML::Value << node.transformation;
			out << YAML::Key << "Offset" << YAML::Value << animation.GetBoneIDMap()[node.name].offset;
			auto* bone = animation.FindBone(node.name);
			if (bone != nullptr)
			{
				out << YAML::Key << "Bone" << YAML::BeginSeq;
				{
					auto& keyPosition = bone->GetPositions();
					auto& keyRotation = bone->GetRotations();
					auto& keyScale = bone->GetScales();
					for (int i = 0; i < bone->GetNumPosition(); i++)
					{
						out << YAML::BeginMap;
						out << YAML::Key << "KeyPosition"       << YAML::Value << keyPosition[i].position;
						out << YAML::Key << "PositionTimeStamp" << YAML::Value << keyPosition[i].timeStamp;
						out << YAML::Key << "KeyRotation"       << YAML::Value << keyRotation[i].orientation;
						out << YAML::Key << "RotationTimeStamp" << YAML::Value << keyRotation[i].timeStamp;
						out << YAML::Key << "KeyScale"          << YAML::Value << keyScale[i].scale;
						out << YAML::Key << "ScaleTimeStamp"    << YAML::Value << keyScale[i].timeStamp;
						out << YAML::EndMap;
					}
					out << YAML::EndSeq;
				}
			}
			out << YAML::Key << "Children" << YAML::BeginSeq;
			{
				for (auto& child : node.children)
					SerializeBoneNode(out, child, animation);
				out << YAML::EndSeq;
			}
			out << YAML::EndMap;// BoneNode
		}
	}


	static void SerializeEntity(YAML::Emitter& out, Entity& entity, UUID* uuid = nullptr)
	{
		VOL_CORE_ASSERT(entity.HasComponent<IDComponent>());

		out << YAML::BeginMap;//Entity
		if(uuid == nullptr)
		    out << YAML::Key << "EntityID" << YAML::Value << entity.GetUUID();
		else
			out << YAML::Key << "EntityID" << YAML::Value << *uuid;

		out << YAML::Key << "Active" << YAML::Value << entity.GetActive();

		if (entity.HasComponent<TagComponent>())
		{
			out << YAML::Key << "TagComponent";
			out << YAML::BeginMap;//TagComponent
			auto& tag = entity.GetComponent<TagComponent>().Tag;
			out << YAML::Key << "Tag" << YAML::Value << tag;
			out << YAML::EndMap;//TagComponent
		}

		if (entity.HasComponent<TransformComponent>())
		{
			out << YAML::Key << "TransformComponent";
			out << YAML::BeginMap;//TransformComponent
			auto& tc = entity.GetComponent<TransformComponent>();
			out << YAML::Key << "Translation" << YAML::Value << tc.Translation;
			out << YAML::Key << "Rotation" << YAML::Value << tc.Rotation;
			out << YAML::Key << "Scale" << YAML::Value << tc.Scale;
			out << YAML::EndMap;//TransformComponent
		}

		if (entity.HasComponent<LightComponent>())
		{
			out << YAML::Key << "LightComponent";
			out << YAML::BeginMap;//LightComponent

			auto& lightComponent = entity.GetComponent<LightComponent>();

			out << YAML::Key << "Enabled" << YAML::Value << lightComponent.enabled;

			out << YAML::Key << "Type" << YAML::Value << (int)lightComponent.Type;
			out << YAML::Key << "Ambient" << YAML::Value << lightComponent.Ambient;
			out << YAML::Key << "Diffuse" << YAML::Value << lightComponent.Diffuse;
			out << YAML::Key << "Specular" << YAML::Value << lightComponent.Specular;
			out << YAML::Key << "Constant" << YAML::Value << lightComponent.Constant;
			out << YAML::Key << "Linear" << YAML::Value << lightComponent.Linear;
			out << YAML::Key << "Quadratic" << YAML::Value << lightComponent.Quadratic;
			out << YAML::Key << "CutOff" << YAML::Value << lightComponent.CutOff;
			out << YAML::Key << "OuterCutOff" << YAML::Value << lightComponent.OuterCutOff;

			out << YAML::EndMap;//LightComponent
		}

		if (entity.HasComponent<CameraComponent>())
		{
			out << YAML::Key << "CameraComponent";
			out << YAML::BeginMap;//CameraComponent

			auto& cameraComponent = entity.GetComponent<CameraComponent>();
			auto& camera = cameraComponent.Camera;

			out << YAML::Key << "Enabled" << YAML::Value << cameraComponent.enabled;

			out << YAML::Key << "Camera" << YAML::Value;
			out << YAML::BeginMap;//Camera
			out << YAML::Key << "ProjectionType" << YAML::Value << (int)camera.GetProjectionType();
			out << YAML::Key << "PerspectiveFOV" << YAML::Value << camera.GetPerspectiveVerticalFOV();
			out << YAML::Key << "PerspectiveNear" << YAML::Value << camera.GetPerspectiveNearClip();
			out << YAML::Key << "PerspectiveFar" << YAML::Value << camera.GetPerspectiveFarClip();
			out << YAML::Key << "OrthographicSize" << YAML::Value << camera.GetOrthographicSize();
			out << YAML::Key << "OrthographicNear" << YAML::Value << camera.GetOrthographicNearClip();
			out << YAML::Key << "OrthographicFar" << YAML::Value << camera.GetOrthographicFarClip();
			out << YAML::EndMap;//Camera

			out << YAML::Key << "Primary" << YAML::Value << cameraComponent.Primary;
			out << YAML::Key << "FixedAspectRatio" << YAML::Value << cameraComponent.FixedAspectRatio;

			out << YAML::EndMap;//CameraComponent
		}

		if (entity.HasComponent<ScriptComponent>())
		{
			out << YAML::Key << "ScriptComponent";
			out << YAML::BeginMap; // ScriptComponent

			auto& scriptComponent = entity.GetComponent<ScriptComponent>();

			out << YAML::Key << "Enabled" << YAML::Value << scriptComponent.enabled;

			out << YAML::Key << "ClassName" << YAML::Value << scriptComponent.ClassName;

				// 保存字段Fields
			Ref<ScriptClass> entityClass = ScriptEngine::GetClass(scriptComponent.ClassName);

			if (entityClass != nullptr)
			{
				const auto& fields = entityClass->GetFields();
				if (fields.size() > 0)
				{
					out << YAML::Key << "ScriptFields" << YAML::Value;
					auto& entityFields = ScriptEngine::GetScriptFieldMap(entity);
					out << YAML::BeginSeq;
					for (const auto& [name, field] : fields)
					{
						if (entityFields.find(name) == entityFields.end())
							continue;

						out << YAML::BeginMap; // ScriptField
						out << YAML::Key << "Name" << YAML::Value << name;
						out << YAML::Key << "Type" << YAML::Value << Utils::ScriptFieldTypeToString(field.Type);

						out << YAML::Key << "Data" << YAML::Value;

						VOL_CORE_ASSERT(entityFields.find(name) != entityFields.end());
						ScriptFieldInstance& scriptField = entityFields.at(name);

						switch (field.Type)
						{
							WRITE_SCRIPT_FIELD(Float,         float);
							WRITE_SCRIPT_FIELD(Double,        double);
							WRITE_SCRIPT_FIELD(Bool,          bool);
							WRITE_SCRIPT_FIELD(Char,          char);
							WRITE_SCRIPT_FIELD(Byte,          int8_t);
							WRITE_SCRIPT_FIELD(Short,         int16_t);
							WRITE_SCRIPT_FIELD(Int,           int32_t);
							WRITE_SCRIPT_FIELD(Long,          int64_t);
							WRITE_SCRIPT_FIELD(UByte,         uint8_t);
							WRITE_SCRIPT_FIELD(UShort,        uint16_t);
							WRITE_SCRIPT_FIELD(UInt,          uint32_t);
							WRITE_SCRIPT_FIELD(ULong,         uint64_t);
							WRITE_SCRIPT_FIELD(String,        std::string);
							WRITE_SCRIPT_FIELD(Vector2,       glm::vec2);
							WRITE_SCRIPT_FIELD(Vector3,       glm::vec3);
							WRITE_SCRIPT_FIELD(Vector4,       glm::vec4);
							WRITE_SCRIPT_FIELD(Quaternion,    glm::quat);
							WRITE_SCRIPT_FIELD(Matrix4x4,     glm::mat4);
							WRITE_SCRIPT_FIELD(Entity,        UUID);
							WRITE_SCRIPT_FIELD(GameObject,    UUID);
							WRITE_SCRIPT_FIELD(Component,     UUID);
							WRITE_SCRIPT_FIELD(Transform,     UUID);
							WRITE_SCRIPT_FIELD(Behaviour,     UUID);
							WRITE_SCRIPT_FIELD(MonoBehaviour, UUID);
							WRITE_SCRIPT_FIELD(Collider,      UUID);
							WRITE_SCRIPT_FIELD(Rigidbody,     UUID);
						}
						out << YAML::EndMap; // ScriptFields
					}
					out << YAML::EndSeq;
				}
			}

			out << YAML::EndMap; // ScriptComponent
		}

		if (entity.HasComponent<SpriteRendererComponent>())
		{
			out << YAML::Key << "SpriteRendererComponent";
			out << YAML::BeginMap;//SpriteRendererComponent
			auto& spriteRendererComponent = entity.GetComponent<SpriteRendererComponent>();
			out << YAML::Key << "Color" << YAML::Value << spriteRendererComponent.Color;
			if (spriteRendererComponent.Texture)
				out << YAML::Key << "TexturePath" << YAML::Value << spriteRendererComponent.Texture->GetPath();

			out << YAML::Key << "TilingFactor" << YAML::Value << spriteRendererComponent.TilingFactor;
			out << YAML::EndMap;//SpriteRendererComponent
		}

		if (entity.HasComponent<MeshComponent>())
		{
			out << YAML::Key << "MeshComponent";
			out << YAML::BeginMap; // MeshComponent
			{
				auto& meshComponent = entity.GetComponent<MeshComponent>();
				out << YAML::Key << "MeshType" << YAML::Value << (int)meshComponent.meshType;
				if (meshComponent.modelPath.empty())
					out << YAML::Key << "ModelPath" << YAML::Value << meshComponent.modelPath;

				out << YAML::Key << "VertexBone" << YAML::Value << YAML::BeginSeq; // VertexBone
				for (auto& vertexBone : meshComponent.vertexBone)
				{
					out << YAML::BeginMap;
					out << YAML::Key << "VertexIndex1" << YAML::Value << vertexBone.vertexIndex1;
					out << YAML::Key << "VertexIndex2" << YAML::Value << vertexBone.vertexIndex2;
					out << YAML::Key << "BoneIndex" << YAML::Value << vertexBone.boneIndex;
					out << YAML::Key << "Weight" << YAML::Value << vertexBone.weight;
					out << YAML::EndMap;
				}
				out << YAML::EndSeq;// VertexBone
				out << YAML::EndMap; // MeshComponent
			}
		}

		if (entity.HasComponent<MeshRendererComponent>())
		{
			out << YAML::Key << "MeshRendererComponent";
			out << YAML::BeginMap; // MeshRendererComponent
			{
				auto& meshRendererComponent = entity.GetComponent<MeshRendererComponent>();

				out << YAML::Key << "Enabled" << YAML::Value << meshRendererComponent.enabled;

				auto& textures = meshRendererComponent.Textures;
				if (!textures.empty())
				{
					out << YAML::Key << "Textures" << YAML::Value << YAML::BeginSeq;
					for (uint32_t i = 0; i < textures.size(); i++)
					{
						out << YAML::BeginMap;
						out << YAML::Key << "ImageType" << YAML::Value << (int)textures[i].first;
						// 保存纹理相对路径
						out << YAML::Key << "Path" << YAML::Value << textures[i].second->GetPath();
						out << YAML::Key << "Filp" << YAML::Value << textures[i].second->GetFilp();
						out << YAML::EndMap;
					}
					out << YAML::EndSeq;
				}
				out << YAML::EndMap; // MeshRendererComponent
			}
		}

		if (entity.HasComponent<CircleRendererComponent>())
		{
			out << YAML::Key << "CircleRendererComponent";
			out << YAML::BeginMap; // CircleRendererComponent

			auto& circleRendererComponent = entity.GetComponent<CircleRendererComponent>();

			out << YAML::Key << "Enabled" << YAML::Value << circleRendererComponent.enabled;

			out << YAML::Key << "Color" << YAML::Value << circleRendererComponent.Color;
			if (circleRendererComponent.Texture)
				out << YAML::Key << "TexturePath" << YAML::Value << circleRendererComponent.Texture->GetPath();
			out << YAML::Key << "Thickness" << YAML::Value << circleRendererComponent.Thickness;
			out << YAML::Key << "Fade" << YAML::Value << circleRendererComponent.Fade;

			out << YAML::EndMap; // CircleRendererComponent
		}

		if (entity.HasComponent<AnimatorComponent>())
		{
			out << YAML::Key << "AnimatorComponent";
			out << YAML::BeginMap; // AnimatorComponent
			auto& animatorComponent = entity.GetComponent<AnimatorComponent>();
			out << YAML::Key << "Enabled" << YAML::Value << animatorComponent.enabled;
			out << YAML::EndMap; // AnimatorComponent
		}

		// Animation只存储资源文件夹的相对路径Path，用于读取动画文件
		if (entity.HasComponent<AnimationComponent>())
		{
			out << YAML::Key << "AnimationComponent";
			out << YAML::BeginMap; // AnimationComponent
			auto& animationComponent = entity.GetComponent<AnimationComponent>();

			out << YAML::Key << "Enabled" << YAML::Value << animationComponent.enabled;

			if (!animationComponent.animation->GetPath().empty())
				out << YAML::Key << "Path" << YAML::Value << Project::GetRelativeAssetDirectory(animationComponent.animation->GetPath()).string();
			else
				out << YAML::Key << "Path" << YAML::Value << animationComponent.animation->GetPath();
			out << YAML::EndMap; // AnimationComponent
		}

		if (entity.HasComponent<Rigidbody2DComponent>())
		{
			out << YAML::Key << "Rigidbody2DComponent";
			out << YAML::BeginMap; // Rigidbody2DComponent

			auto& rb2dComponent = entity.GetComponent<Rigidbody2DComponent>();
			out << YAML::Key << "BodyType" << YAML::Value << RigidBody2DBodyTypeToString(rb2dComponent.Type);
			out << YAML::Key << "FixedRotation" << YAML::Value << rb2dComponent.FixedRotation;

			out << YAML::EndMap; // Rigidbody2DComponent
		}

		if (entity.HasComponent<BoxCollider2DComponent>())
		{
			out << YAML::Key << "BoxCollider2DComponent";
			out << YAML::BeginMap; // BoxCollider2DComponent

			auto& bc2dComponent = entity.GetComponent<BoxCollider2DComponent>();
			out << YAML::Key << "Offset" << YAML::Value << bc2dComponent.Offset;
			out << YAML::Key << "Size" << YAML::Value << bc2dComponent.Size;
			out << YAML::Key << "Density" << YAML::Value << bc2dComponent.Density;
			out << YAML::Key << "Friction" << YAML::Value << bc2dComponent.Friction;
			out << YAML::Key << "Restitution" << YAML::Value << bc2dComponent.Restitution;
			out << YAML::Key << "RestitutionThreshold" << YAML::Value << bc2dComponent.RestitutionThreshold;

			out << YAML::EndMap; // BoxCollider2DComponent
		}

		if (entity.HasComponent<CircleCollider2DComponent>())
		{
			out << YAML::Key << "CircleCollider2DComponent";
			out << YAML::BeginMap; // CircleCollider2DComponent

			auto& cc2dComponent = entity.GetComponent<CircleCollider2DComponent>();
			out << YAML::Key << "Offset" << YAML::Value << cc2dComponent.Offset;
			out << YAML::Key << "Radius" << YAML::Value << cc2dComponent.Radius;
			out << YAML::Key << "Density" << YAML::Value << cc2dComponent.Density;
			out << YAML::Key << "Friction" << YAML::Value << cc2dComponent.Friction;
			out << YAML::Key << "Restitution" << YAML::Value << cc2dComponent.Restitution;
			out << YAML::Key << "RestitutionThreshold" << YAML::Value << cc2dComponent.RestitutionThreshold;

			out << YAML::EndMap; // CircleCollider2DComponent
		}

		if (entity.HasComponent<RigidbodyComponent>())
		{
			out << YAML::Key << "RigidbodyComponent";
			out << YAML::BeginMap; // RigidbodyComponent
			auto& rbComponent = entity.GetComponent<RigidbodyComponent>();
			out << YAML::Key << "Type" << YAML::Value << (int)rbComponent.Type;
			out << YAML::Key << "FixedRotation" << YAML::Value << rbComponent.FixedRotation;
			out << YAML::EndMap; // RigidbodyComponent
		}

		if (entity.HasComponent<BoxColliderComponent>())
		{
			out << YAML::Key << "BoxColliderComponent";
			out << YAML::BeginMap; // BoxColliderComponent
			auto& bcComponent = entity.GetComponent<BoxColliderComponent>();
			out << YAML::Key << "Enabled"              << YAML::Value << bcComponent.enabled;
			out << YAML::Key << "IsTrigger"            << YAML::Value << bcComponent.isTrigger;
			out << YAML::Key << "Center"               << YAML::Value << bcComponent.center;
			out << YAML::Key << "Density"              << YAML::Value << bcComponent.material.density;
			out << YAML::Key << "StaticFriction"       << YAML::Value << bcComponent.material.staticFriction;
			out << YAML::Key << "DynamicFriction"      << YAML::Value << bcComponent.material.dynamicFriction;
			out << YAML::Key << "Bounciness"           << YAML::Value << bcComponent.material.bounciness;
			out << YAML::Key << "RestitutionThreshold" << YAML::Value << bcComponent.material.restitutionThreshold;
			out << YAML::Key << "Size"                 << YAML::Value << bcComponent.size;
			out << YAML::EndMap; // BoxColliderComponent
		}

		if (entity.HasComponent<SphereColliderComponent>())
		{
			out << YAML::Key << "SphereColliderComponent";
			out << YAML::BeginMap; // SphereColliderComponent
			auto& scComponent = entity.GetComponent<SphereColliderComponent>();
			out << YAML::Key << "Enabled"              << YAML::Value << scComponent.enabled;
			out << YAML::Key << "IsTrigger"            << YAML::Value << scComponent.isTrigger;
			out << YAML::Key << "Center"               << YAML::Value << scComponent.center;
			out << YAML::Key << "Density"              << YAML::Value << scComponent.material.density;
			out << YAML::Key << "StaticFriction"       << YAML::Value << scComponent.material.staticFriction;
			out << YAML::Key << "DynamicFriction"      << YAML::Value << scComponent.material.dynamicFriction;
			out << YAML::Key << "Bounciness"           << YAML::Value << scComponent.material.bounciness;
			out << YAML::Key << "RestitutionThreshold" << YAML::Value << scComponent.material.restitutionThreshold;
			out << YAML::Key << "Radius"               << YAML::Value << scComponent.radius;
			out << YAML::EndMap; // SphereColliderComponent
		}

		if (entity.HasComponent<SkyboxComponent>())
		{
			out << YAML::Key << "SkyboxComponent";
			out << YAML::BeginMap; // SkyboxComponent

			auto& skyboxComponent = entity.GetComponent<SkyboxComponent>();

			out << YAML::Key << "Enabled" << YAML::Value << skyboxComponent.enabled;

			out << YAML::Key << "Primary" << YAML::Value << skyboxComponent.Primary;
			out << YAML::Key << "Path"    << YAML::Value << skyboxComponent.texture->GetPath();
			out << YAML::EndMap; // SkyboxComponent
		}

		if (entity.HasComponent<ParticleSystemComponent>())
		{
			out << YAML::Key << "ParticleSystemComponent";
			out << YAML::BeginMap; // ParticleSystemComponent

			auto& particleSystemComponent = entity.GetComponent<ParticleSystemComponent>();

			out << YAML::Key << "Enabled" << YAML::Value << particleSystemComponent.enabled;

			auto& particleSystem = particleSystemComponent.particleSystem;

			out << YAML::Key << "ParticleSystem";
			out << YAML::BeginMap; // ParticleSystem
			{
				out << YAML::Key << "PlaybackSpeed"        << YAML::Value << particleSystem->playbackSpeed;
				out << YAML::Key << "Duration"             << YAML::Value << particleSystem->duration;
				out << YAML::Key << "Looping"              << YAML::Value << particleSystem->looping;
				out << YAML::Key << "Prewarm"              << YAML::Value << particleSystem->prewarm;
				out << YAML::Key << "StartDelay1"          << YAML::Value << particleSystem->startDelay1;
				out << YAML::Key << "StartDelay2"          << YAML::Value << particleSystem->startDelay2;
				out << YAML::Key << "StartDelayType"       << YAML::Value << (int)particleSystem->startDelayType;
				out << YAML::Key << "StartLifetime1"       << YAML::Value << particleSystem->startLifetime1;
				out << YAML::Key << "StartLifetime2"       << YAML::Value << particleSystem->startLifetime2;
				out << YAML::Key << "StartLifetimeType"    << YAML::Value << (int)particleSystem->startLifetimeType;
				out << YAML::Key << "StartSpeed1"          << YAML::Value << particleSystem->startSpeed1;
				out << YAML::Key << "StartSpeed2"          << YAML::Value << particleSystem->startSpeed2;
				out << YAML::Key << "StartSpeedType"       << YAML::Value << (int)particleSystem->startSpeedType;
				out << YAML::Key << "ThreeDStartSize"      << YAML::Value << particleSystem->threeDStartSize;
				out << YAML::Key << "StartSize1"           << YAML::Value << particleSystem->startSize1;
				out << YAML::Key << "StartSize2"           << YAML::Value << particleSystem->startSize2;
				out << YAML::Key << "ThreeDStartSize1"     << YAML::Value << particleSystem->threeDStartSize1;
				out << YAML::Key << "ThreeDStartSize2"     << YAML::Value << particleSystem->threeDStartSize2;
				out << YAML::Key << "StartSizeType"        << YAML::Value << (int)particleSystem->startSizeType;
				out << YAML::Key << "ThreeDStartRotation"  << YAML::Value << particleSystem->threeDStartRotation;
				out << YAML::Key << "StartRotation1"       << YAML::Value << particleSystem->startRotation1;
				out << YAML::Key << "StartRotation2"       << YAML::Value << particleSystem->startRotation2;
				out << YAML::Key << "ThreeDStartRotation1" << YAML::Value << particleSystem->threeDStartRotation1;
				out << YAML::Key << "ThreeDStartRotation2" << YAML::Value << particleSystem->threeDStartRotation2;
				out << YAML::Key << "StartRotationType"    << YAML::Value << (int)particleSystem->startRotationType;
				out << YAML::Key << "FlipRotation"         << YAML::Value << particleSystem->flipRotation;
				out << YAML::Key << "StartColor1"          << YAML::Value << particleSystem->startColor1;
				out << YAML::Key << "StartColor2"          << YAML::Value << particleSystem->startColor2;
				out << YAML::Key << "StartColorType"       << YAML::Value << (int)particleSystem->startColorType;
				out << YAML::Key << "SimulationSpace"      << YAML::Value << (int)particleSystem->simulationSpace;
				out << YAML::Key << "SimulationSpeed"      << YAML::Value << particleSystem->simulationSpeed;
				out << YAML::Key << "PlayOnAwake"          << YAML::Value << particleSystem->playOnAwake;
				out << YAML::Key << "MaxParticles"         << YAML::Value << particleSystem->maxParticles;
				out << YAML::Key << "PlayOnAwake"          << YAML::Value << particleSystem->playOnAwake;

				out << YAML::EndMap; // ParticleSystem

			}

			auto& emission = particleSystem->emission;

			out << YAML::Key << "Emission";
			out << YAML::BeginMap; // Emission
			{
				out << YAML::Key << "Enabled"              << YAML::Value << emission.enabled;
				out << YAML::Key << "RateOverTime1"        << YAML::Value << emission.rateOverTime1;
				out << YAML::Key << "RateOverTime2"        << YAML::Value << emission.rateOverTime2;
				out << YAML::Key << "RateOverTimeType"     << YAML::Value << (int)emission.rateOverTimeType;
				out << YAML::Key << "RateOverDistance1"    << YAML::Value << emission.rateOverDistance1;
				out << YAML::Key << "RateOverDistance2"    << YAML::Value << emission.rateOverDistance2;
				out << YAML::Key << "RateOverDistanceType" << YAML::Value << (int)emission.rateOverDistanceType;

				auto& bursts = emission.bursts;
				out << YAML::Key << "Bursts";
				out << YAML::BeginSeq; // Bursts
				{
					for (int i = 0; i < bursts.size(); i++)
					{
						out << YAML::BeginMap;
						{
							out << YAML::Key << "Time"        << YAML::Value << bursts[i].time;
							out << YAML::Key << "Count1"      << YAML::Value << bursts[i].count1;
							out << YAML::Key << "Count2"      << YAML::Value << bursts[i].count2;
							out << YAML::Key << "CountType"   << YAML::Value << (int)bursts[i].countType;
							out << YAML::Key << "Cycles"      << YAML::Value << bursts[i].cycles;
							out << YAML::Key << "CyclesType"  << YAML::Value << (int)bursts[i].cyclesType;
							out << YAML::Key << "Interval"    << YAML::Value << bursts[i].interval;
							out << YAML::Key << "Probability" << YAML::Value << bursts[i].probability;
							out << YAML::BeginMap;
						}
					}
					out << YAML::EndSeq; // Bursts
				}
				out << YAML::EndMap; // Emission
			}

			auto& shape = particleSystem->shape;

			out << YAML::Key << "Shape";
			out << YAML::BeginMap; // Shape
			{
				out << YAML::Key << "Enabled"         << YAML::Value << shape.enabled;
				out << YAML::Key << "Shape"           << YAML::Value << (int)shape.shape;
				out << YAML::Key << "Angle"           << YAML::Value << shape.angle;
				out << YAML::Key << "Radius"          << YAML::Value << shape.radius;
				out << YAML::Key << "RadiusThickness" << YAML::Value << shape.radiusThickness;
				out << YAML::Key << "Arc"             << YAML::Value << shape.arc;
				out << YAML::Key << "Length"          << YAML::Value << shape.length;
				out << YAML::Key << "EmitFrom"        << YAML::Value << shape.emitFrom;
				if(shape.texture != nullptr)
					out << YAML::Key << "Texture" << YAML::Value << shape.texture->GetPath();
				else
					out << YAML::Key << "Texture" << YAML::Value << std::string();
				out << YAML::Key << "Position"        << YAML::Value << shape.position;
				out << YAML::Key << "Rotation"        << YAML::Value << shape.rotation;
				out << YAML::Key << "Scale"           << YAML::Value << shape.scale;
				out << YAML::EndMap; // Shape
			}

			auto& renderer = particleSystem->renderer;

			out << YAML::Key << "Renderer";
			out << YAML::BeginMap; // Renderer
			{
				out << YAML::Key << "Enabled"         << YAML::Value << renderer.enabled;
				out << YAML::Key << "RenderMode"      << YAML::Value << (int)renderer.renderMode;
				out << YAML::Key << "NormalDirection" << YAML::Value << renderer.normalDirection;
				if (renderer.material != nullptr)
					out << YAML::Key << "Material" << YAML::Value << renderer.material->GetPath();
				else
					out << YAML::Key << "Material" << YAML::Value << std::string();
				out << YAML::Key << "Meshes";
				out << YAML::BeginSeq; // Meshes
				{
					for (int i = 0; i < renderer.meshes.size(); i++)
					{
						out << YAML::BeginMap;
						out << YAML::Key << "MeshType" << YAML::Value << (int)renderer.meshes[i].first;
						out << YAML::EndMap;
					}
					out << YAML::EndSeq; // Meshes
				}

				out << YAML::EndMap; // Renderer
			}

			out << YAML::EndMap; // ParticleSystemComponent
		}


		if (!entity.GetEntityChildren().empty())
		{
			out << YAML::Key << "Entities" << YAML::Value << YAML::BeginSeq;// 开始序列化
			for (auto& entity : entity.GetEntityChildren())
			{
				if (uuid != nullptr)
				{
					UUID id = UUID();
					SerializeEntity(out, *entity.get(), &id);
				}
				else
					SerializeEntity(out, *entity.get());
			}
			out << YAML::EndSeq; // 结束序列化
		}

		out << YAML::EndMap;//Entity
	}

	void SceneSerializer::SerializePrefab(const std::string& filepath, Entity& entity, UUID* uuid)
	{
		YAML::Emitter out;
		out << YAML::BeginMap;
		{
			out << YAML::Key << "Entities" << YAML::Value << YAML::BeginSeq;// 开始序列化
				Volcano::SerializeEntity(out, entity, uuid);
			out << YAML::EndSeq; // 结束序列化

			out << YAML::EndMap;
		}
		std::ofstream fout(filepath);
		fout << out.c_str();
	}

	void SceneSerializer::Serialize(const std::string& filepath)
	{
		YAML::Emitter out;
		out << YAML::BeginMap;
		{
			out << YAML::Key << "Scene" << YAML::Value << m_Scene->GetName();
			if (!m_Scene->GetEntityList().empty())
			{
				out << YAML::Key << "Entities" << YAML::Value << YAML::BeginSeq;// 开始序列化
				{
					for (auto& entity : m_Scene->GetEntityList())
						Volcano::SerializeEntity(out, *entity.get());
					out << YAML::EndSeq; // 结束序列化
				}
			}

			out << YAML::Key << "Meshes" << YAML::BeginSeq;
			{
				for (auto& [name, mesh] : Mesh::GetMeshLibrary()->GetMeshes())
				{
					out << YAML::BeginMap;
					out << YAML::Key << "Path" << YAML::Value << name;
					out << YAML::EndMap;
				}
				out << YAML::EndSeq;
			}

			out << YAML::Key << "Animations" << YAML::BeginSeq;
			{
				for (auto& [name, animation] : Animation::GetAnimationLibrary()->GetAnimations())
				{
					out << YAML::BeginMap;
					out << YAML::Key << "Path" << YAML::Value << animation->GetName();
					out << YAML::EndMap;
				}
				out << YAML::EndSeq;
			}

			// 将已载入的Prefab的路径保存
			out << YAML::Key << "Prefab";
			out << YAML::BeginMap; // Prefab
			{
				out << YAML::Key << "PrefabPaths" << YAML::BeginSeq; // PrefabPaths
				{
					for (auto& [prefabPath, entity] : Prefab::GetEntityPathMap())
					{
						out << YAML::BeginMap;
						out << YAML::Key << "PrefabPath" << YAML::Value << prefabPath;
						out << YAML::EndMap;
					}
					out << YAML::EndSeq; // PrefabPaths
				}

				out << YAML::Key << "EntityPrefabMap" << YAML::BeginSeq; // EntityPrefabMap
				{
					for (auto& [id, prefabPath] : Prefab::GetEntityPrefabMap())
					{
						out << YAML::BeginMap;
						out << YAML::Key << "ID"         << YAML::Value << id;
						out << YAML::Key << "PrefabPath" << YAML::Value << prefabPath;
						out << YAML::EndMap;
					}
					out << YAML::EndSeq; // EntityPrefabMap
				}

				out << YAML::EndMap;// Prefab
			}
			out << YAML::EndMap;
		}
		std::ofstream fout(filepath);
		fout << out.c_str();
	}

	void SceneSerializer::SerializeRuntime(const std::string& filepath)
	{
		// Not implemented
		VOL_CORE_ASSERT(false);
	}

	void LoadAnimation(YAML::Node& yamlNode, AssimpNodeData& node, Animation& animation)
	{
		node.name = yamlNode["Name"].as<std::string>();
		node.transformation = yamlNode["NodeTransform"].as<glm::mat4>();
		int id = yamlNode["ID"].as<int>();
		animation.GetBoneIDMap()[node.name] = { id, yamlNode["Offset"].as<glm::mat4>() };
		auto yamlBone = yamlNode["Bone"];
		if (yamlBone.IsDefined())
		{
			Bone bone(node.name, id);
			bone.RemoveKeyPosition(0);
			bone.RemoveKeyRotation(0);
			bone.RemoveKeyScale(0);
			for (auto boneData : yamlBone)
			{
				bone.AddKeyPosition(boneData["KeyPosition"].as<glm::vec3>(), boneData["PositionTimeStamp"].as<float>());
				bone.AddKeyRotation(boneData["KeyRotation"].as<glm::quat>(), boneData["RotationTimeStamp"].as<float>());
				bone.AddKeyScale(boneData["KeyScale"].as<glm::vec3>(), boneData["ScaleTimeStamp"].as<float>());
			}
			animation.GetBones().push_back(bone);
		}
		auto yamlChildren = yamlNode["Children"];
		for (auto yamlChild : yamlChildren)
		{
			AssimpNodeData child;
			child.parent = &node;
			LoadAnimation(yamlChild, child, animation);
			node.childrenCount++;
			node.children.push_back(child);
		}
	}

	// entityParent为null时在scene目录下读取Entity
	Ref<Entity> DeserializeLoadEntity(YAML::Node& entity, Scene* scene, Ref<Entity> entityParent = nullptr)
	{
		uint64_t uuid = entity["EntityID"].as<uint64_t>();

		std::string name;
		auto tagComponent = entity["TagComponent"];
		if (tagComponent)
			name = tagComponent["Tag"].as<std::string>();

		VOL_CORE_TRACE("Deserialized entity with ID = {0}, name = {1}", uuid, name);

		Ref<Entity> deserializedEntity = scene->CreateEntityWithUUID(uuid, name, entityParent.get());

		deserializedEntity->SetActive(entity["Active"].as<bool>());

		auto transformComponent = entity["TransformComponent"];
		if (transformComponent)
		{
			//Entitys always have transforms
			auto& tc = deserializedEntity->GetComponent<TransformComponent>();
			tc.Translation = transformComponent["Translation"].as<glm::vec3>();
			tc.Rotation = transformComponent["Rotation"].as<glm::vec3>();
			tc.Scale = transformComponent["Scale"].as<glm::vec3>();
		}

		auto lightComponent = entity["LightComponent"];
		if (lightComponent)
		{
			auto& lc = deserializedEntity->AddComponent<LightComponent>();
			lc.enabled = lightComponent["Enabled"].as<bool>();
			lc.Type = (LightComponent::LightType)lightComponent["Type"].as<int>();
			lc.Ambient = lightComponent["Ambient"].as<glm::vec3>();
			lc.Diffuse = lightComponent["Diffuse"].as<glm::vec3>();
			lc.Specular = lightComponent["Specular"].as<glm::vec3>();
			lc.Constant = lightComponent["Constant"].as<float>();
			lc.Linear = lightComponent["Linear"].as<float>();
			lc.Quadratic = lightComponent["Quadratic"].as<float>();
			lc.CutOff = lightComponent["CutOff"].as<float>();
			lc.OuterCutOff = lightComponent["OuterCutOff"].as<float>();
		}

		auto cameraComponent = entity["CameraComponent"];
		if (cameraComponent)
		{
			auto& cc = deserializedEntity->AddComponent<CameraComponent>();
			cc.enabled = cameraComponent["Enabled"].as<bool>();
			auto cameraProps = cameraComponent["Camera"];
			cc.Camera.SetProjectionType((SceneCamera::ProjectionType)cameraProps["ProjectionType"].as<int>());

			cc.Camera.SetPerspectiveVerticalFOV(cameraProps["PerspectiveFOV"].as<float>());
			cc.Camera.SetPerspectiveNearClip(cameraProps["PerspectiveNear"].as<float>());
			cc.Camera.SetPerspectiveFarClip(cameraProps["PerspectiveFar"].as<float>());
			cc.Camera.SetOrthographicSize(cameraProps["OrthographicSize"].as<float>());
			cc.Camera.SetOrthographicNearClip(cameraProps["OrthographicNear"].as<float>());
			cc.Camera.SetOrthographicFarClip(cameraProps["OrthographicFar"].as<float>());

			cc.Primary = cameraComponent["Primary"].as<bool>();
			cc.FixedAspectRatio = cameraComponent["FixedAspectRatio"].as<bool>();
		}

		auto scriptComponent = entity["ScriptComponent"];
		if (scriptComponent)
		{
			auto& sc = deserializedEntity->AddComponent<ScriptComponent>();
			sc.enabled = scriptComponent["Enabled"].as<bool>();
			sc.ClassName = scriptComponent["ClassName"].as<std::string>();

			// 获取脚本字段数据
			auto scriptFields = scriptComponent["ScriptFields"];
			if (scriptFields)
			{
				// 获取mono类
				Ref<ScriptClass> entityClass = ScriptEngine::GetClass(sc.ClassName);

				if (entityClass)
				{
					const auto& fields = entityClass->GetFields();
					auto& entityFields = ScriptEngine::GetScriptFieldMap(*deserializedEntity.get());

					for (auto scriptField : scriptFields)
					{
						std::string name = scriptField["Name"].as<std::string>();
						std::string typeString = scriptField["Type"].as<std::string>();
						ScriptFieldType type = Utils::ScriptFieldTypeFromString(typeString);

						//entityFields[name] = ScriptFieldInstance(); // 初始化实体对应字段
						ScriptFieldInstance& fieldInstance = entityFields[name];

						// TODO(Yan): turn this assert into volcano log warning
						VOL_CORE_ASSERT(fields.find(name) != fields.end(), "Deserialize.ScriptComponent");

						if (fields.find(name) == fields.end())
							continue;

						fieldInstance.Field = fields.at(name);

						switch (type)
						{
							READ_SCRIPT_FIELD(Float,         float);
							READ_SCRIPT_FIELD(Double,        double);
							READ_SCRIPT_FIELD(Bool,          bool);
							READ_SCRIPT_FIELD(Char,          char);
							READ_SCRIPT_FIELD(Byte,          int8_t);
							READ_SCRIPT_FIELD(Short,         int16_t);
							READ_SCRIPT_FIELD(Int,           int32_t);
							READ_SCRIPT_FIELD(Long,          int64_t);
							READ_SCRIPT_FIELD(UByte,         uint8_t);
							READ_SCRIPT_FIELD(UShort,        uint16_t);
							READ_SCRIPT_FIELD(UInt,          uint32_t);
							READ_SCRIPT_FIELD(ULong,         uint64_t);
							READ_SCRIPT_FIELD(String,        std::string);
							READ_SCRIPT_FIELD(Vector2,       glm::vec2);
							READ_SCRIPT_FIELD(Vector3,       glm::vec3);
							READ_SCRIPT_FIELD(Vector4,       glm::vec4);
							READ_SCRIPT_FIELD(Quaternion,    glm::quat);
							READ_SCRIPT_FIELD(Matrix4x4,     glm::mat4);
							READ_SCRIPT_FIELD(Entity,        UUID);
							READ_SCRIPT_FIELD(GameObject,    UUID);
							READ_SCRIPT_FIELD(Component,     UUID);
							READ_SCRIPT_FIELD(Transform,     UUID);
							READ_SCRIPT_FIELD(Behaviour,     UUID);
							READ_SCRIPT_FIELD(MonoBehaviour, UUID);
							READ_SCRIPT_FIELD(Collider,      UUID);
							READ_SCRIPT_FIELD(Rigidbody,     UUID);
						}
					}
				}
			}
		}

		auto spriteRendererComponent = entity["SpriteRendererComponent"];
		if (spriteRendererComponent)
		{
			auto& src = deserializedEntity->AddComponent<SpriteRendererComponent>();
			src.Color = spriteRendererComponent["Color"].as<glm::vec4>();
			if (spriteRendererComponent["TexturePath"])
			{
				std::string texturePath = spriteRendererComponent["TexturePath"].as<std::string>();
				auto path = Project::GetAssetFileSystemPath(texturePath);
				src.Texture = Texture2D::Create(path.string());
			}

			if (spriteRendererComponent["TilingFactor"])
				src.TilingFactor = spriteRendererComponent["TilingFactor"].as<float>();
		}

		auto meshComponent = entity["MeshComponent"];
		if (meshComponent)
		{
			auto& mc = deserializedEntity->AddComponent<MeshComponent>();
			mc.meshType = (MeshType)meshComponent["MeshType"].as<int>();
			if (mc.meshType == MeshType::Model && meshComponent["ModelPath"].IsDefined())
			{
				mc.modelPath = meshComponent["ModelPath"].as<std::string>();
				if (!Mesh::GetMeshLibrary()->Exists(mc.modelPath))
					Mesh::GetMeshLibrary()->Load(mc.modelPath);
				auto meshNode = Mesh::GetMeshLibrary()->Get(mc.modelPath);
				mc.SetMesh(MeshType::Model, deserializedEntity.get(), meshNode->mesh);
			}
			else
				mc.SetMesh(mc.meshType, deserializedEntity.get());

			auto vertexBones = meshComponent["VertexBone"];
			if (!vertexBones.IsNull())
			{
				for (auto vertexBone : vertexBones)
				{
					MeshComponent::VertexBone vertexBoneTemp;
					vertexBoneTemp.vertexIndex1 = vertexBone["VertexIndex1"].as<int>();
					vertexBoneTemp.vertexIndex2 = vertexBone["VertexIndex2"].as<int>();
					vertexBoneTemp.boneIndex = vertexBone["BoneIndex"].as<int>();
					vertexBoneTemp.weight = vertexBone["Weight"].as<float>();
					mc.vertexBone.push_back(vertexBoneTemp);
				}
			}
		}

		auto meshRendererComponent = entity["MeshRendererComponent"];
		if (meshRendererComponent)
		{
			auto& mrc = deserializedEntity->AddComponent<MeshRendererComponent>();
			mrc.enabled = meshRendererComponent["Enabled"].as<bool>();
			auto textures = meshRendererComponent["Textures"];
			if (!textures.IsNull())
			{
				for (auto texture : textures)
				{
					ImageType type = (ImageType)texture["ImageType"].as<int>();
					std::string path = texture["Path"].as<std::string>();
					if (!path.empty())
					{
						auto filePath = Project::GetAssetFileSystemPath(path);
						mrc.AddTexture(type, Texture2D::Create(filePath.string(), texture["Filp"].as<bool>()));
					}
					else
						mrc.AddTexture(type);
				}
			}
		}

		auto circleRendererComponent = entity["CircleRendererComponent"];
		if (circleRendererComponent)
		{
			auto& crc = deserializedEntity->AddComponent<CircleRendererComponent>();
			crc.enabled = circleRendererComponent["Enabled"].as<bool>();
			crc.Color = circleRendererComponent["Color"].as<glm::vec4>();
			if (circleRendererComponent["TexturePath"])
				crc.Texture = Texture2D::Create(circleRendererComponent["TexturePath"].as<std::string>());
			crc.Thickness = circleRendererComponent["Thickness"].as<float>();
			crc.Fade = circleRendererComponent["Fade"].as<float>();
		}

		auto animatorComponent = entity["AnimatorComponent"];
		if (animatorComponent)
		{
			auto& ac = deserializedEntity->AddComponent<AnimatorComponent>();
			ac.enabled = animatorComponent["Enabled"].as<bool>();
		}

		auto animationComponent = entity["AnimationComponent"];
		if (animationComponent)
		{
			auto& ac = deserializedEntity->AddComponent<AnimationComponent>();
			ac.enabled = animationComponent["Enabled"].as<bool>();
			std::string path = animationComponent["Path"].as<std::string>();
			if (!path.empty())
			{
				if (!Animation::GetAnimationLibrary()->Exists(path))
					Animation::GetAnimationLibrary()->LoadAnm(path);
				ac.animation = Animation::GetAnimationLibrary()->Get(path);
			}
			else
				ac.animation = std::make_shared<Animation>();
		}

		auto rigidbody2DComponent = entity["Rigidbody2DComponent"];
		if (rigidbody2DComponent)
		{
			auto& rb2d = deserializedEntity->AddComponent<Rigidbody2DComponent>();
			rb2d.Type = RigidBody2DBodyTypeFromString(rigidbody2DComponent["BodyType"].as<std::string>());
			rb2d.FixedRotation = rigidbody2DComponent["FixedRotation"].as<bool>();
		}

		auto boxCollider2DComponent = entity["BoxCollider2DComponent"];
		if (boxCollider2DComponent)
		{
			auto& bc2d = deserializedEntity->AddComponent<BoxCollider2DComponent>();
			bc2d.Offset = boxCollider2DComponent["Offset"].as<glm::vec2>();
			bc2d.Size = boxCollider2DComponent["Size"].as<glm::vec2>();
			bc2d.Density = boxCollider2DComponent["Density"].as<float>();
			bc2d.Friction = boxCollider2DComponent["Friction"].as<float>();
			bc2d.Restitution = boxCollider2DComponent["Restitution"].as<float>();
			bc2d.RestitutionThreshold = boxCollider2DComponent["RestitutionThreshold"].as<float>();
		}

		auto circleCollider2DComponent = entity["CircleCollider2DComponent"];
		if (circleCollider2DComponent)
		{
			auto& cc2d = deserializedEntity->AddComponent<CircleCollider2DComponent>();
			cc2d.Offset = circleCollider2DComponent["Offset"].as<glm::vec2>();
			cc2d.Radius = circleCollider2DComponent["Radius"].as<float>();
			cc2d.Density = circleCollider2DComponent["Density"].as<float>();
			cc2d.Friction = circleCollider2DComponent["Friction"].as<float>();
			cc2d.Restitution = circleCollider2DComponent["Restitution"].as<float>();
			cc2d.RestitutionThreshold = circleCollider2DComponent["RestitutionThreshold"].as<float>();
		}

		auto rigidbodyComponent = entity["RigidbodyComponent"];
		if (rigidbodyComponent)
		{
			auto& rb = deserializedEntity->AddComponent<RigidbodyComponent>();
			rb.Type = (RigidbodyComponent::BodyType)rigidbodyComponent["Type"].as<int>();
			rb.FixedRotation = rigidbodyComponent["FixedRotation"].as<bool>();
		}

		auto boxColliderComponent = entity["BoxColliderComponent"];
		if (boxColliderComponent)
		{
			auto& bc = deserializedEntity->AddComponent<BoxColliderComponent>();
			bc.enabled = boxColliderComponent["Enabled"].as<bool>();
			bc.isTrigger = boxColliderComponent["IsTrigger"].as<bool>();
			bc.center = boxColliderComponent["Center"].as<glm::vec3>();
			bc.material.density = boxColliderComponent["Density"].as<float>();
			bc.material.staticFriction = boxColliderComponent["StaticFriction"].as<float>();
			bc.material.dynamicFriction = boxColliderComponent["DynamicFriction"].as<float>();
			bc.material.bounciness = boxColliderComponent["Bounciness"].as<float>();
			bc.material.restitutionThreshold = boxColliderComponent["RestitutionThreshold"].as<float>();
			bc.size = boxColliderComponent["Size"].as<glm::vec3>();
		}

		auto sphereColliderComponent = entity["SphereColliderComponent"];
		if (sphereColliderComponent)
		{
			auto& sc = deserializedEntity->AddComponent<SphereColliderComponent>();
			sc.enabled = sphereColliderComponent["Enabled"].as<bool>();
			sc.isTrigger = sphereColliderComponent["IsTrigger"].as<bool>();
			sc.center = sphereColliderComponent["Center"].as<glm::vec3>();
			sc.material.density = sphereColliderComponent["Density"].as<float>();
			sc.material.staticFriction = sphereColliderComponent["StaticFriction"].as<float>();
			sc.material.dynamicFriction = sphereColliderComponent["DynamicFriction"].as<float>();
			sc.material.bounciness = sphereColliderComponent["Bounciness"].as<float>();
			sc.material.restitutionThreshold = sphereColliderComponent["RestitutionThreshold"].as<float>();
			sc.radius = sphereColliderComponent["Radius"].as<float>();
		}

		auto skyboxComponent = entity["SkyboxComponent"];
		if (skyboxComponent)
		{
			auto& sc = deserializedEntity->AddComponent<SkyboxComponent>();
			sc.enabled = skyboxComponent["Enabled"].as<bool>();
			sc.Primary = skyboxComponent["Primary"].as<bool>();
			std::string path  = skyboxComponent["Path"].as<std::string>();
			if (!path.empty())
			{
				sc.texture = TextureCube::Create(Project::GetAssetFileSystemPath(path).string());
			}
		}

		auto particleSystemComponent = entity["ParticleSystemComponent"];
		if (particleSystemComponent)
		{
			auto& psc = deserializedEntity->AddComponent<ParticleSystemComponent>();
			psc.enabled = particleSystemComponent["Enabled"].as<bool>();

			auto particleSystemYAML = particleSystemComponent["ParticleSystem"];
			auto& particleSystem = psc.particleSystem;
			particleSystem->playbackSpeed        = particleSystemYAML["PlaybackSpeed"].as<float>();       
			particleSystem->duration             = particleSystemYAML["Duration"].as<float>();
			particleSystem->looping              = particleSystemYAML["Looping"].as<bool>();
			particleSystem->prewarm              = particleSystemYAML["Prewarm"].as<bool>();
			particleSystem->startDelay1          = particleSystemYAML["StartDelay1"].as<float>();
			particleSystem->startDelay2          = particleSystemYAML["StartDelay2"].as<float>();
			particleSystem->startDelayType       = particleSystemYAML["StartDelayType"].as<int>();      
			particleSystem->startLifetime1       = particleSystemYAML["StartLifetime1"].as<float>();      
			particleSystem->startLifetime2       = particleSystemYAML["StartLifetime2"].as<float>();      
			particleSystem->startLifetimeType    = particleSystemYAML["StartLifetimeType"].as<int>();
			particleSystem->startSpeed1          = particleSystemYAML["StartSpeed1"].as<float>();         
			particleSystem->startSpeed2          = particleSystemYAML["StartSpeed2"].as<float>();         
			particleSystem->startSpeedType       = particleSystemYAML["StartSpeedType"].as<int>();
			particleSystem->threeDStartSize      = particleSystemYAML["ThreeDStartSize"].as<bool>();
			particleSystem->startSize1           = particleSystemYAML["StartSize1"].as<float>();
			particleSystem->startSize2           = particleSystemYAML["StartSize2"].as<float>();
			particleSystem->threeDStartSize1     = particleSystemYAML["ThreeDStartSize1"].as<glm::vec3>();
			particleSystem->threeDStartSize2     = particleSystemYAML["ThreeDStartSize2"].as<glm::vec3>();
			particleSystem->startSizeType        = particleSystemYAML["StartSizeType"].as<int>();
			particleSystem->threeDStartRotation  = particleSystemYAML["ThreeDStartRotation"].as<bool>();
			particleSystem->startRotation1       = particleSystemYAML["StartRotation1"].as<float>();      
			particleSystem->startRotation2       = particleSystemYAML["StartRotation2"].as<float>();      
			particleSystem->threeDStartRotation1 = particleSystemYAML["ThreeDStartRotation1"].as<glm::vec3>();
			particleSystem->threeDStartRotation2 = particleSystemYAML["ThreeDStartRotation2"].as<glm::vec3>();
			particleSystem->startRotationType    = particleSystemYAML["StartRotationType"].as<int>();
			particleSystem->flipRotation         = particleSystemYAML["FlipRotation"].as<float>();
			particleSystem->startColor1          = particleSystemYAML["StartColor1"].as<glm::vec4>();
			particleSystem->startColor2          = particleSystemYAML["StartColor2"].as<glm::vec4>();
			particleSystem->startColorType       = particleSystemYAML["StartColorType"].as<int>();
			particleSystem->simulationSpace      = particleSystemYAML["SimulationSpace"].as<int>();
			particleSystem->simulationSpeed      = particleSystemYAML["SimulationSpeed"].as<float>();
			particleSystem->playOnAwake          = particleSystemYAML["PlayOnAwake"].as<bool>();
			particleSystem->maxParticles         = particleSystemYAML["MaxParticles"].as<int>();
			particleSystem->playOnAwake          = particleSystemYAML["PlayOnAwake"].as<bool>();

			auto emissionYAML = particleSystemComponent["Emission"];
			auto& emission = particleSystem->emission;

			emission.enabled              = emissionYAML["Enabled"].as<bool>();             
			emission.rateOverTime1        = emissionYAML["RateOverTime1"].as<float>();       
			emission.rateOverTime2        = emissionYAML["RateOverTime2"].as<float>();       
			emission.rateOverTimeType     = emissionYAML["RateOverTimeType"].as<int>();
			emission.rateOverDistance1    = emissionYAML["RateOverDistance1"].as<float>();   
			emission.rateOverDistance2    = emissionYAML["RateOverDistance2"].as<float>();   
			emission.rateOverDistanceType = emissionYAML["RateOverDistanceType"].as<int>();
			auto burstsYAML = emissionYAML["Bursts"];
			for (auto burstYAML : burstsYAML)
			{
				ParticleSystem_Emission::Burst burst;
				burst.time        = burstYAML["Time"].as<float>();       
				burst.count1      = burstYAML["Count1"].as<float>();     
				burst.count2      = burstYAML["Count2"].as<float>();     
				burst.countType   = burstYAML["CountType"].as<int>();  
				burst.cycles      = burstYAML["Cycles"].as<uint32_t>();     
				burst.cyclesType  = burstYAML["CyclesType"].as<int>();
				burst.interval    = burstYAML["Interval"].as<float>();   
				burst.probability = burstYAML["Probability"].as<float>();
				emission.bursts.push_back(burst);
			}

			auto shapeYAML = particleSystemComponent["Shape"];
			auto& shape = particleSystem->shape;

			shape.enabled         = shapeYAML["Enabled"].as<bool>();        
			shape.shape           = (ParticleSystem_Shape::Shape)shapeYAML["Shape"].as<int>();
			shape.angle           = shapeYAML["Angle"].as<float>();          
			shape.radius          = shapeYAML["Radius"].as<float>();         
			shape.radiusThickness = shapeYAML["RadiusThickness"].as<float>();
			shape.arc             = shapeYAML["Arc"].as<float>();            
			shape.length          = shapeYAML["Length"].as<float>();         
			shape.emitFrom        = shapeYAML["EmitFrom"].as<bool>();
			std::string shapeTexturePath = shapeYAML["Texture"].as<std::string>();
			if (shapeTexturePath != std::string())
				shape.texture = Texture2D::Create(Project::GetAssetFileSystemPath(shapeTexturePath).string());
			else
				shape.texture = nullptr;
			shape.position        = shapeYAML["Position"].as<glm::vec3>();       
			shape.rotation        = shapeYAML["Rotation"].as<glm::vec3>();
			shape.scale           = shapeYAML["Scale"].as<glm::vec3>();


			auto rendererYAML = particleSystemComponent["Renderer"];
			auto& renderer = particleSystem->renderer;
			renderer.enabled         = rendererYAML["Enabled"].as<bool>();        
			renderer.renderMode      = (ParticleSystem_Renderer::RenderMode)rendererYAML["RenderMode"].as<int>();
			renderer.normalDirection = rendererYAML["NormalDirection"].as<float>();
			std::string rendererMaterialPath = rendererYAML["Material"].as<std::string>();
			if (rendererMaterialPath != std::string())
				renderer.material = Texture2D::Create(Project::GetAssetFileSystemPath(rendererMaterialPath).string());
			else
				renderer.material = nullptr;
			auto meshesYAML = rendererYAML["Meshes"];
			for (auto meshYAML : meshesYAML)
			{
				Ref<Mesh> mesh;
				ParticleSystem_Renderer::MeshType type = (ParticleSystem_Renderer::MeshType)meshYAML["MeshType"].as<int>();
				switch (type)
				{
				case ParticleSystem_Renderer::MeshType::None:
					mesh = nullptr;
					break;

				case ParticleSystem_Renderer::MeshType::Cube:
					mesh = CubeMesh::CloneRef();
					break;
					
				case ParticleSystem_Renderer::MeshType::Capsule:
					mesh = CapsuleMesh::CloneRef();
					break;
					
				case ParticleSystem_Renderer::MeshType::Cylinder:
					mesh = CylinderMesh::CloneRef();
					break;
					
				case ParticleSystem_Renderer::MeshType::Plane:
					mesh = PlaneMesh::CloneRef();
					break;
					
				case ParticleSystem_Renderer::MeshType::Sphere:
					mesh = SphereMesh::CloneRef();
					break;
					
				case ParticleSystem_Renderer::MeshType::Quad:
					mesh = QuadMesh::CloneRef();
					break;

				default:
					VOL_ASSERT(false, "Deserialize.ParticleSystemComponent.Renderer.MeshType");
					break;
				}
				if(mesh != nullptr)
					mesh->ResetMaxMeshes(particleSystem->maxParticles);
				renderer.meshes.push_back({ type, mesh });
			}
		}

		auto entities = entity["Entities"];
		if (entities)
			for (auto entity : entities)
				DeserializeLoadEntity(entity, scene, deserializedEntity);

		return deserializedEntity;
	}

	Ref<Entity> SceneSerializer::DeserializePrefab(const std::string& filepath, const std::string& fileName)
	{
		YAML::Node data;
		try
		{
			data = YAML::LoadFile(filepath);
		}
		catch (YAML::ParserException e)
		{
			VOL_CORE_ERROR("Failed to load .volcano file '{0}'\n     {1}", filepath, e.what());
			return nullptr;
		}

		if (!data["Entities"])
			return nullptr;
		VOL_CORE_TRACE("Deserializing Prefab '{0}'", fileName);


		auto prefab = data["Entities"];
		if (prefab.size())
		{
			auto entity = prefab[0];
			return DeserializeLoadEntity(entity, m_Scene.get(), nullptr);
		}
		return nullptr;
	}

	bool SceneSerializer::Deserialize(const std::string& filepath)
	{
		YAML::Node data;
		try
		{
			data = YAML::LoadFile(filepath);
		}
		catch (YAML::ParserException e)
		{
			VOL_CORE_ERROR("Failed to load .volcano file '{0}'\n     {1}", filepath, e.what());
			return false;
		}

		if (!data["Scene"])
			return false;
		std::string sceneName = data["Scene"].as<std::string>();
		VOL_CORE_TRACE("Deserializing scene '{0}'", sceneName);
		m_Scene->SetName(sceneName);
		m_Scene->SetFilePath(filepath);
		m_Scene->SetPath(std::filesystem::path(filepath).parent_path().append(sceneName).string());

		auto meshes = data["Meshes"];
		if (meshes)
			for (auto mesh : meshes)
				Mesh::GetMeshLibrary()->Load(mesh["Path"].as<std::string>());

		auto animations = data["Animations"];
		if (animations)
			for (auto animation : animations)
				Animation::GetAnimationLibrary()->LoadAnm(animation["Path"].as<std::string>());

		auto prefab = data["Prefab"];
		if (prefab)
		{
			for (auto prefabPath : prefab["PrefabPaths"])
			{
				std::string prefabPathTemp = prefabPath["PrefabPath"].as<std::string>();
				if (Prefab::Get(prefabPathTemp) == nullptr)
					Prefab::Load(Project::GetAssetFileSystemPath(prefabPathTemp));
			}

			for (auto entityPrefab : prefab["EntityPrefabMap"])
			{
				Prefab::GetEntityPrefabMap()[entityPrefab["ID"].as<UUID>()] = entityPrefab["PrefabPath"].as<std::string>();
			}
		}

		auto entities = data["Entities"];
		if (entities)
			for (auto entity : entities)
				DeserializeLoadEntity(entity, m_Scene.get());
		return true;
	}

	bool SceneSerializer::DeserializeRuntime(const std::string& filepath)
	{
		// Not implemented
		VOL_CORE_ASSERT(false);
		return false;
	}


	MeshSerializer::MeshSerializer(Ref<MeshNode> meshNode)
		: m_MeshNode(meshNode)
	{
	}

	// 序列化Mesh的顶点、索引、纹理、骨骼信息
	// filepath: 目标mms的绝对路径
	bool MeshSerializer::Serialize(const std::filesystem::path filepath)
	{
		std::string name = FileUtils::GetFileNameFromPath(filepath.string());

		if (m_MeshNode == nullptr)
			return false;

		YAML::Emitter out;
		{
			out << YAML::BeginMap;
			{
				out << YAML::Key << "Mesh";
				out << YAML::BeginMap;// Mesh
				{
					out << YAML::Key << "Name" << YAML::Value << name;
					out << YAML::Key << "NumVertices" << YAML::Value << m_MeshNode->mesh->GetVertexSize();
					out << YAML::Key << "Vertices" << YAML::BeginSeq; // Vertices
					{
						auto& vertices = m_MeshNode->mesh->GetVertices();
						for (uint32_t i = 0; i < vertices.size(); i++)
						{
							out << YAML::BeginMap;
							{
								out << YAML::Key << "Position" << YAML::Value << vertices[i].Position;
								out << YAML::Key << "TexCoords" << YAML::Value << vertices[i].TexCoords;
								out << YAML::Key << "Normal" << YAML::Value << vertices[i].Normal;
								out << YAML::Key << "Tangent" << YAML::Value << vertices[i].Tangent;
								out << YAML::Key << "Bitangent" << YAML::Value << vertices[i].Bitangent;
								out << YAML::Key << "BoneIDs" << YAML::Value << glm::ivec4(vertices[i].BoneIDs[0], vertices[i].BoneIDs[1], vertices[i].BoneIDs[2], vertices[i].BoneIDs[3]);
								out << YAML::Key << "Weights" << YAML::Value << glm::vec4(vertices[i].Weights[0], vertices[i].Weights[1], vertices[i].Weights[2], vertices[i].Weights[3]);
								out << YAML::EndMap;
							}
						}
						out << YAML::EndSeq; // Vertices
					}
					auto& indices = m_MeshNode->mesh->GetIndices();
					out << YAML::Key << "NumIndices" << YAML::Value << m_MeshNode->mesh->GetIndexSize();
					out << YAML::Key << "Indices" << YAML::BeginSeq; // Indices
					{
						for (uint32_t i = 0; i < indices.size(); i++)
							out << indices[i];
						out << YAML::EndSeq; // Indices
					}


					auto& textures = m_MeshNode->textures;
					if (!textures.empty())
					{
						out << YAML::Key << "Textures" << YAML::Value << YAML::BeginSeq; // Textures
						for (uint32_t i = 0; i < textures.size(); i++)
						{
							out << YAML::BeginMap;
							out << YAML::Key << "ImageType" << YAML::Value << (int)textures[i].first;
							// 保存纹理相对路径
							out << YAML::Key << "Path" << YAML::Value << textures[i].second->GetPath();
							out << YAML::Key << "Filp" << YAML::Value << textures[i].second->GetFilp();
							out << YAML::EndMap;
						}
						out << YAML::EndSeq;// Textures
					}
					/*
						struct BoneData
						{
							int id;
							std::string name;
							glm::mat4 offset;
						};

						Entity* entityTemp = m_MeshNode->mesh->GetEntity();
						do
						{
							if (entityTemp->HasComponent<AnimationComponent>())
							{
								auto& animation = entityTemp->GetComponent<AnimationComponent>().animation;

								out << YAML::Key << "Bones" << YAML::BeginSeq; // Bones
								{
									std::unordered_map<int, BoneData> boneMap;
									for (auto& vertex : m_MeshNode->mesh->GetVertices())
									{
										for (uint32_t boneIndex = 0; boneIndex < MAX_BONE_INFLUENCE; boneIndex++)
										{
											int id = vertex.BoneIDs[boneIndex];
											if (id == -1)
												continue;
											if (boneMap.find(id) == boneMap.end())
											{
												Bone* bone = animation->FindBone(id);
												if (bone != nullptr)
												{
													boneMap[id] = BoneData(bone->GetBoneID(), bone->GetBoneName(), animation->GetBoneIDMap()[bone->GetBoneName()].offset);
												}
											}
										}
									}
									for (auto& [id, bone] : boneMap)
									{
										out << YAML::BeginMap;
										{
											out << YAML::Key << "ID" << YAML::Value << bone.id;
											out << YAML::Key << "Name" << YAML::Value << bone.name;
											out << YAML::Key << "Offset" << YAML::Value << bone.offset;
											out << YAML::EndMap;
										}
									}
									out << YAML::EndSeq;// Bones
								}
								break;
							}
							else
								entityTemp = entityTemp->GetEntityParent();
						} while (entityTemp != nullptr);
						*/
					out << YAML::EndMap; // Mesh
				}
				out << YAML::EndMap;
			}
		}

		std::ofstream fout(filepath);
		fout << out.c_str();

		return true;
	}

	// 从目标mms文件中读取mesh和textures
	// filepath: mms文件绝对路径
	bool MeshSerializer::Deserialize(const std::filesystem::path filepath)
	{

		YAML::Node data;
		try
		{
			data = YAML::LoadFile(filepath.string());
		}
		catch (YAML::ParserException e)
		{
			//VOL_CORE_ERROR("Failed to load project file '{0}'\n     {1}", filepath, e.what());
			return false;
		}

		auto meshNode = data["Mesh"];
		if (!meshNode)
			return false;

		std::vector<MeshVertex> vertices;
		std::vector<uint32_t> indices;
		std::vector<std::pair<ImageType, Ref<Texture>>> textures;

		for (auto vertex : meshNode["Vertices"])
		{
			MeshVertex mv;
			mv.Position        = vertex["Position"].as<glm::vec3>();
			mv.TexCoords       = vertex["TexCoords"].as<glm::vec2>();
			mv.Normal          = vertex["Normal"].as<glm::vec3>();
			mv.Tangent         = vertex["Tangent"].as<glm::vec3>();
			mv.Bitangent       = vertex["Bitangent"].as<glm::vec3>();
			glm::ivec4 boneIDs = vertex["BoneIDs"].as<glm::ivec4>();
			glm::vec4 weights  = vertex["Weights"].as<glm::vec4>();
			for (uint32_t i = 0; i < 4; i++)
			{
				if (boneIDs[i] == -1)
					continue;
				mv.BoneIDs[i] = boneIDs[i];
				mv.Weights[i] = weights[i];
			}
			vertices.push_back(mv);
		}
		for (auto index : meshNode["Indices"])
			indices.push_back(index.as<int>());

		m_MeshNode->mesh = std::make_shared<ModelMesh>(vertices, indices);

		auto texturesNode = meshNode["Textures"];
		if (texturesNode.size())
		{
			m_MeshNode->textures.clear();
			for (auto texture : texturesNode)
			{
				std::string texturePath = Project::GetAssetFileSystemPath(texture["Path"].as<std::string>()).string();
				m_MeshNode->textures.push_back({ (ImageType)texture["ImageType"].as<int>(), Texture2D::Create(texturePath, texture["Filp"].as<bool>())});
			}
		}


		return true;
	}

	AnimationSerializer::AnimationSerializer(Ref<Animation> animation)
		: m_Animation(animation)
	{
	}

	bool AnimationSerializer::Serialize(const std::filesystem::path filepath)
	{

		std::string name = FileUtils::GetFileNameFromPath(filepath.string());

		YAML::Emitter out;
		{
			out << YAML::BeginMap; // Animation
			{
				out << YAML::Key << "Animation";
				out << YAML::BeginMap; // Animation
				{
					out << YAML::Key << "Name" << YAML::Value << name;
					out << YAML::Key << "Duration" << YAML::Value << m_Animation->GetDuration();
					out << YAML::Key << "TicksPerSecond" << YAML::Value << m_Animation->GetTicksPerSecond();

					out << YAML::Key << "BoneNode";
					SerializeBoneNode(out, m_Animation->GetRootNode(), *m_Animation.get());
					out << YAML::EndMap;
				}
				out << YAML::EndMap;
			}
		}

		std::ofstream fout(filepath);
		fout << out.c_str();

		return true;
	}

	bool AnimationSerializer::Deserialize(const std::filesystem::path filepath)
	{
		if (m_Animation == nullptr)
			return false;

		YAML::Node data;
		try
		{
			data = YAML::LoadFile(filepath.string());
		}
		catch (YAML::ParserException e)
		{
			//VOL_CORE_ERROR("Failed to load project file '{0}'\n     {1}", filepath, e.what());
			return false;
		}

		auto animationNode = data["Animation"];
		if (!animationNode)
			return false;

		m_Animation->SetDuration(animationNode["Duration"].as<float>());
		m_Animation->SetTicksPerSecond(animationNode["TicksPerSecond"].as<float>());
		auto& rootNode = m_Animation->GetRootNode();
		auto yamlBoneNode = animationNode["BoneNode"];
		LoadAnimation(yamlBoneNode, rootNode, *m_Animation.get());

		return true;
	}
}