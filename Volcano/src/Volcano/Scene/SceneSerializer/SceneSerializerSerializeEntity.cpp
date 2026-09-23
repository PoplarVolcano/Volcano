#include "volpch.h"
#include "SceneSerializer.h"

#include "Volcano/Scene/Entity.h"
#include "Volcano/Scene/Components.h"
#include "Volcano/Scripting/ScriptEngine.h"
#include "Volcano/Utils/YAMLUtils.h"

namespace Volcano
{
	namespace Utils
	{
#define WRITE_SCRIPT_FIELD(FieldType, Type)           \
			case ScriptFieldType::FieldType:          \
				out << scriptFieldInstance.GetValue<Type>();  \
				break

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

		void SerializeParticleSystemComponent(YAML::Emitter& out, Entity& entity);

		void SerializeEntity(YAML::Emitter& out, Entity& entity)
		{
			VOL_CORE_ASSERT(entity.HasComponent<IDComponent>());

			out << YAML::BeginMap;//Entity
			out << YAML::Key << "EntityID" << YAML::Value << entity.GetUUID();
			out << YAML::Key << "Active" << YAML::Value << entity.GetActive();

			if (entity.HasComponent<TagComponent>())
			{
				out << YAML::Key << "TagComponent";
				out << YAML::BeginMap;//TagComponent
				auto& tag = entity.GetComponent<TagComponent>().tag;
				out << YAML::Key << "tag" << YAML::Value << tag;
				out << YAML::EndMap;//TagComponent
			}

			if (entity.HasComponent<TransformComponent>())
			{
				out << YAML::Key << "TransformComponent";
				out << YAML::BeginMap;//TransformComponent
				auto& tc = entity.GetComponent<TransformComponent>();
				out << YAML::Key << "translation" << YAML::Value << tc.translation;
				out << YAML::Key << "rotation" << YAML::Value << tc.rotation;
				out << YAML::Key << "scale" << YAML::Value << tc.scale;
				out << YAML::EndMap;//TransformComponent
			}

			if (entity.HasComponent<CameraComponent>())
			{
				out << YAML::Key << "CameraComponent";
				out << YAML::BeginMap;//CameraComponent

				auto& cameraComponent = entity.GetComponent<CameraComponent>();
				auto& camera = cameraComponent.Camera;

				out << YAML::Key << "enabled" << YAML::Value << cameraComponent.enabled;

				out << YAML::Key << "Camera" << YAML::Value;
				out << YAML::BeginMap;//Camera
				out << YAML::Key << "projectionType"   << YAML::Value << (int)camera.GetProjectionType();
				out << YAML::Key << "perspectiveFOV"   << YAML::Value << camera.GetPerspectiveVerticalFOV();
				out << YAML::Key << "perspectiveNear"  << YAML::Value << camera.GetPerspectiveNearClip();
				out << YAML::Key << "perspectiveFar"   << YAML::Value << camera.GetPerspectiveFarClip();
				out << YAML::Key << "orthographicSize" << YAML::Value << camera.GetOrthographicSize();
				out << YAML::Key << "orthographicNear" << YAML::Value << camera.GetOrthographicNearClip();
				out << YAML::Key << "orthographicFar"  << YAML::Value << camera.GetOrthographicFarClip();
				out << YAML::EndMap;//Camera

				out << YAML::Key << "primary" << YAML::Value << cameraComponent.primary;
				out << YAML::Key << "fixedAspectRatio" << YAML::Value << cameraComponent.fixedAspectRatio;

				out << YAML::EndMap;//CameraComponent
			}

			if (entity.HasComponent<CircleRendererComponent>())
			{
				out << YAML::Key << "CircleRendererComponent";
				out << YAML::BeginMap; // CircleRendererComponent

				auto& circleRendererComponent = entity.GetComponent<CircleRendererComponent>();

				out << YAML::Key << "enabled" << YAML::Value << circleRendererComponent.enabled;
				out << YAML::Key << "color" << YAML::Value << circleRendererComponent.color;
				out << YAML::Key << "thickness" << YAML::Value << circleRendererComponent.thickness;
				out << YAML::Key << "fade" << YAML::Value << circleRendererComponent.fade;

				out << YAML::EndMap; // CircleRendererComponent
			}

			if (entity.HasComponent<MeshComponent>())
			{
				out << YAML::Key << "MeshComponent";
				out << YAML::BeginMap; // MeshComponent
				{
					auto& meshComponent = entity.GetComponent<MeshComponent>();
					out << YAML::Key << "meshType" << YAML::Value << (int)meshComponent.meshType;
					if (meshComponent.modelPath.empty())
						out << YAML::Key << "modelPath" << YAML::Value << meshComponent.modelPath;

					out << YAML::EndMap; // MeshComponent
				}
			}

			if (entity.HasComponent<MeshRendererComponent>())
			{
				out << YAML::Key << "MeshRendererComponent";
				out << YAML::BeginMap; // MeshRendererComponent
				{
					auto& meshRendererComponent = entity.GetComponent<MeshRendererComponent>();

					out << YAML::Key << "enabled"                   << YAML::Value << meshRendererComponent.enabled;
					out << YAML::Key << "materialLibraryKey"        << YAML::Value << meshRendererComponent.materialLibraryKey.key;
					out << YAML::Key << "color"                     << YAML::Value << meshRendererComponent.color;
					out << YAML::Key << "uvRect"                    << YAML::Value << meshRendererComponent.uvRect;
					out << YAML::Key << "parallaxScale"             << YAML::Value << meshRendererComponent.parallaxScale;
					out << YAML::Key << "tilingFactor"              << YAML::Value << meshRendererComponent.tilingFactor;
					out << YAML::Key << "thickness"                 << YAML::Value << meshRendererComponent.thickness;
					out << YAML::Key << "fade"                      << YAML::Value << meshRendererComponent.fade;
					out << YAML::Key << "flags"                     << YAML::Value << meshRendererComponent.flags;
					out << YAML::Key << "outlineColor"              << YAML::Value << meshRendererComponent.outlineColor;
					out << YAML::Key << "outlineScale"              << YAML::Value << meshRendererComponent.outlineScale;
					out << YAML::Key << "explosionOffset"           << YAML::Value << meshRendererComponent.explosionOffset;
					out << YAML::Key << "normalVisualizationColor"  << YAML::Value << meshRendererComponent.normalVisualizationColor;
					out << YAML::Key << "normalVisualizationLength" << YAML::Value << meshRendererComponent.normalVisualizationLength;
					out << YAML::Key << "normalVisualizationIndex1" << YAML::Value << meshRendererComponent.normalVisualizationIndex1;
					out << YAML::Key << "normalVisualizationIndex2" << YAML::Value << meshRendererComponent.normalVisualizationIndex2;
					out << YAML::EndMap; // MeshRendererComponent
				}
			}

			if (entity.HasComponent<LightComponent>())
			{
				out << YAML::Key << "LightComponent";
				out << YAML::BeginMap; // LightComponent
				{
					auto& lightComponent = entity.GetComponent<LightComponent>();

					out << YAML::Key << "enabled"       << YAML::Value << lightComponent.enabled;
					out << YAML::Key << "type"          << YAML::Value << (int)lightComponent.type;
					out << YAML::Key << "ambient"       << YAML::Value << lightComponent.ambient;
					out << YAML::Key << "diffuse"       << YAML::Value << lightComponent.diffuse;
					out << YAML::Key << "specular"      << YAML::Value << lightComponent.specular;
					out << YAML::Key << "constant"      << YAML::Value << lightComponent.constant;
					out << YAML::Key << "linear"        << YAML::Value << lightComponent.linear;
					out << YAML::Key << "quadratic"     << YAML::Value << lightComponent.quadratic;
					out << YAML::Key << "cutoff"        << YAML::Value << lightComponent.cutoff;
					out << YAML::Key << "outerCutoff"   << YAML::Value << lightComponent.outerCutoff;
					out << YAML::Key << "radius"        << YAML::Value << lightComponent.radius;
					out << YAML::Key << "shadowEnabled" << YAML::Value << lightComponent.shadowEnabled;


					out << YAML::EndMap; // LightComponent
				}
			}

			if (entity.HasComponent<SkyboxComponent>())
			{
				out << YAML::Key << "SkyboxComponent";
				out << YAML::BeginMap; // SkyboxComponent
				{
					auto& skyboxComponent = entity.GetComponent<SkyboxComponent>();

					out << YAML::Key << "enabled"     << YAML::Value << skyboxComponent.enabled;
					out << YAML::Key << "primary"     << YAML::Value << skyboxComponent.primary;
					out << YAML::Key << "textureType" << YAML::Value << skyboxComponent.textureType;
					out << YAML::Key << "textures"    << YAML::Value;
					out << YAML::BeginSeq; // textures
					for (int i = 0; i != 6; i++)
					{
						out << YAML::BeginMap;
						out << YAML::Key << "key"            << YAML::Value << skyboxComponent.textures[i].key;
						out << YAML::Key << "keyHash"        << YAML::Value << skyboxComponent.textures[i].keyHash;
						out << YAML::Key << "flip"           << YAML::Value << skyboxComponent.textures[i].flip;
						out << YAML::Key << "internalFormat" << YAML::Value << (int)skyboxComponent.textures[i].internalFormat;
						out << YAML::EndMap;
					}
					out << YAML::EndSeq; // textures
					out << YAML::Key << "textureCubeMap" << YAML::Value << skyboxComponent.textureCubeMap->GetRelativePath();
					
					out << YAML::EndMap; // SkyboxComponent
				}
			}

			if (entity.HasComponent<HDRComponent>())
			{
				out << YAML::Key << "HDRComponent";
				out << YAML::BeginMap; // HDRComponent
				{
					auto& hdrComponent = entity.GetComponent<HDRComponent>();

					out << YAML::Key << "enabled" << YAML::Value << hdrComponent.enabled;
					out << YAML::Key << "primary" << YAML::Value << hdrComponent.primary;

					out << YAML::Key << "equirectangularMapKey" << YAML::Value;
					out << YAML::BeginMap; // equirectangularMapKey
					{
						out << YAML::Key << "key"            << YAML::Value << hdrComponent.equirectangularMapKey.key;
						out << YAML::Key << "keyHash"        << YAML::Value << hdrComponent.equirectangularMapKey.keyHash;
						out << YAML::Key << "flip"           << YAML::Value << hdrComponent.equirectangularMapKey.flip;
						out << YAML::Key << "internalFormat" << YAML::Value << (int)hdrComponent.equirectangularMapKey.internalFormat;
						out << YAML::EndMap; // equirectangularMapKey
					}
					out << YAML::EndMap; // HDRComponent
				}
			}

			if (entity.HasComponent<ParticleSystemComponent>())
			{
				SerializeParticleSystemComponent(out, entity);
			}

			if (entity.HasComponent<ScriptComponent>())
			{
				out << YAML::Key << "ScriptComponent";
				out << YAML::BeginMap; // ScriptComponent

				auto& scriptComponent = entity.GetComponent<ScriptComponent>();

				out << YAML::Key << "enabled" << YAML::Value << scriptComponent.enabled;
				out << YAML::Key << "className" << YAML::Value << scriptComponent.ClassName;

				// 保存字段Fields
				Ref<ScriptClass> scriptClass = ScriptEngine::GetScriptClass(scriptComponent.ClassName, false);

				if (scriptClass != nullptr)
				{
					const auto& fields = scriptClass->GetFields();
					if (fields.size() > 0)
					{
						out << YAML::Key << "scriptFields" << YAML::Value;
						auto& entityScriptFieldMap = ScriptEngine::GetEntityScriptFieldMap();
						out << YAML::BeginSeq;
						for (const auto& [fieldNameHash, scriptField] : fields)
						{
							ScriptFieldInstance* scriptFieldInstancePtr = entityScriptFieldMap.TryGetField(entity.GetUUID(), scriptClass->GetFullNameHash(), fieldNameHash);
							if (scriptFieldInstancePtr == nullptr)
								continue;

							out << YAML::BeginMap; // ScriptField
							out << YAML::Key << "name" << YAML::Value << scriptField.name;
							out << YAML::Key << "nameHash" << YAML::Value << scriptField.nameHash;
							out << YAML::Key << "type" << YAML::Value << Utils::ScriptFieldTypeToString(scriptField.type);

							out << YAML::Key << "data" << YAML::Value;

							ScriptFieldInstance& scriptFieldInstance = *scriptFieldInstancePtr;
							switch (scriptField.type)
							{
								WRITE_SCRIPT_FIELD(Float, float);
								WRITE_SCRIPT_FIELD(Double, double);
								WRITE_SCRIPT_FIELD(Bool, bool);
								WRITE_SCRIPT_FIELD(Char, char);
								WRITE_SCRIPT_FIELD(Byte, int8_t);
								WRITE_SCRIPT_FIELD(Short, int16_t);
								WRITE_SCRIPT_FIELD(Int, int32_t);
								WRITE_SCRIPT_FIELD(Long, int64_t);
								WRITE_SCRIPT_FIELD(UByte, uint8_t);
								WRITE_SCRIPT_FIELD(UShort, uint16_t);
								WRITE_SCRIPT_FIELD(UInt, uint32_t);
								WRITE_SCRIPT_FIELD(ULong, uint64_t);
								WRITE_SCRIPT_FIELD(String, std::string);
								WRITE_SCRIPT_FIELD(Vector2, glm::vec2);
								WRITE_SCRIPT_FIELD(Vector3, glm::vec3);
								WRITE_SCRIPT_FIELD(Vector4, glm::vec4);
								WRITE_SCRIPT_FIELD(Quaternion, glm::quat);
								WRITE_SCRIPT_FIELD(Matrix4x4, glm::mat4);
								WRITE_SCRIPT_FIELD(Object, UUID);
								WRITE_SCRIPT_FIELD(GameObject, UUID);
								WRITE_SCRIPT_FIELD(Component, UUID);
								WRITE_SCRIPT_FIELD(Transform, UUID);
								WRITE_SCRIPT_FIELD(Behaviour, UUID);
								WRITE_SCRIPT_FIELD(MonoBehaviour, UUID);
							}
							out << YAML::EndMap; // ScriptFields
						}
						out << YAML::EndSeq;
					}
				}

				out << YAML::EndMap; // ScriptComponent
			}

			if (entity.HasComponent<Rigidbody2DComponent>())
			{
				out << YAML::Key << "Rigidbody2DComponent";
				out << YAML::BeginMap; // Rigidbody2DComponent

				auto& rb2dComponent = entity.GetComponent<Rigidbody2DComponent>();
				out << YAML::Key << "bodyType" << YAML::Value << RigidBody2DBodyTypeToString(rb2dComponent.type);
				out << YAML::Key << "fixedRotation" << YAML::Value << rb2dComponent.fixedRotation;

				out << YAML::EndMap; // Rigidbody2DComponent
			}

			if (entity.HasComponent<BoxCollider2DComponent>())
			{
				out << YAML::Key << "BoxCollider2DComponent";
				out << YAML::BeginMap; // BoxCollider2DComponent

				auto& bc2dComponent = entity.GetComponent<BoxCollider2DComponent>();

				out << YAML::Key << "enabled"              << YAML::Value << bc2dComponent.enabled;
				out << YAML::Key << "offset"               << YAML::Value << bc2dComponent.offset;
				out << YAML::Key << "size"                 << YAML::Value << bc2dComponent.size;
				out << YAML::Key << "density"              << YAML::Value << bc2dComponent.density;
				out << YAML::Key << "friction"             << YAML::Value << bc2dComponent.friction;
				out << YAML::Key << "restitution"          << YAML::Value << bc2dComponent.restitution;
				out << YAML::Key << "restitutionThreshold" << YAML::Value << bc2dComponent.restitutionThreshold;

				out << YAML::EndMap; // BoxCollider2DComponent
			}

			if (entity.HasComponent<CircleCollider2DComponent>())
			{
				out << YAML::Key << "CircleCollider2DComponent";
				out << YAML::BeginMap; // CircleCollider2DComponent

				auto& cc2dComponent = entity.GetComponent<CircleCollider2DComponent>();

				out << YAML::Key << "enabled"              << YAML::Value << cc2dComponent.enabled;
				out << YAML::Key << "offset"               << YAML::Value << cc2dComponent.offset;
				out << YAML::Key << "radius"               << YAML::Value << cc2dComponent.radius;
				out << YAML::Key << "density"              << YAML::Value << cc2dComponent.density;
				out << YAML::Key << "friction"             << YAML::Value << cc2dComponent.friction;
				out << YAML::Key << "restitution"          << YAML::Value << cc2dComponent.restitution;
				out << YAML::Key << "restitutionThreshold" << YAML::Value << cc2dComponent.restitutionThreshold;

				out << YAML::EndMap; // CircleCollider2DComponent
			}

			if (!entity.GetEntityChildrenList().empty())
			{
				out << YAML::Key << "Entities" << YAML::Value << YAML::BeginSeq;// 开始序列化
				for (auto& entity : entity.GetEntityChildrenList())
					SerializeEntity(out, *entity.get());
				out << YAML::EndSeq; // 结束序列化
			}

			out << YAML::EndMap;//Entity
		}
	}
}