#include "volpch.h"
#include "SceneSerializer.h"

#include "Volcano/Project/Project.h"
#include "Volcano/Scene/Entity.h"
#include "Volcano/Scripting/ScriptEngine.h"
#include "Volcano/Utils/YAMLUtils.h"

namespace Volcano
{
	namespace Utils
	{

#define READ_SCRIPT_FIELD(FieldType, Type)             \
			case ScriptFieldType::FieldType:                   \
			{                                                  \
				Type data = scriptField["data"].as<Type>();    \
				scriptFieldInstance.SetValue(data);            \
				break;                                         \
			}

		static Rigidbody2DComponent::BodyType RigidBody2DBodyTypeFromString(const std::string& bodyTypeString)
		{
			if (bodyTypeString == "Static")    return Rigidbody2DComponent::BodyType::Static;
			if (bodyTypeString == "Dynamic")   return Rigidbody2DComponent::BodyType::Dynamic;
			if (bodyTypeString == "Kinematic") return Rigidbody2DComponent::BodyType::Kinematic;

			VOL_CORE_ASSERT(false, "Unknown body type");
			return Rigidbody2DComponent::BodyType::Static;
		}

		// entityParent为null时在scene目录下读取Entity
		Ref<Entity> DeserializeEntity(YAML::Node& entity, Scene* scene, Ref<Entity> entityParent = nullptr)
		{
			uint64_t uuid = entity["EntityID"].as<uint64_t>();

			std::string name;
			auto tagComponent = entity["TagComponent"];
			if (tagComponent)
				name = tagComponent["tag"].as<std::string>();

			VOL_CORE_TRACE("Deserialized entity with ID = {0}, name = {1}", uuid, name);

			Ref<Entity> deserializedEntity = scene->CreateEntityWithUUID(uuid, name, entityParent.get());

			deserializedEntity->SetActive(entity["Active"].as<bool>());

			auto transformComponent = entity["TransformComponent"];
			if (transformComponent)
			{
				//Entitys always have transforms
				auto& tc = deserializedEntity->GetComponent<TransformComponent>();
				tc.translation = transformComponent["translation"].as<glm::vec3>();
				tc.SetRotation(transformComponent["rotation"].as<glm::quat>());
				tc.scale = transformComponent["scale"].as<glm::vec3>();
			}

			auto cameraComponent = entity["CameraComponent"];
			if (cameraComponent)
			{
				auto& cc = deserializedEntity->AddComponent<CameraComponent>();
				cc.enabled = cameraComponent["enabled"].as<bool>();
				auto cameraProps = cameraComponent["Camera"];
				cc.Camera.SetProjectionType((SceneCamera::ProjectionType)cameraProps["projectionType"].as<int>());
				cc.Camera.SetPerspectiveVerticalFOV(cameraProps["perspectiveFOV"].as<float>());
				cc.Camera.SetPerspectiveNearClip(cameraProps["perspectiveNear"].as<float>());
				cc.Camera.SetPerspectiveFarClip(cameraProps["perspectiveFar"].as<float>());
				cc.Camera.SetOrthographicSize(cameraProps["orthographicSize"].as<float>());
				cc.Camera.SetOrthographicNearClip(cameraProps["orthographicNear"].as<float>());
				cc.Camera.SetOrthographicFarClip(cameraProps["orthographicFar"].as<float>());

				cc.primary = cameraComponent["primary"].as<bool>();
				cc.fixedAspectRatio = cameraComponent["fixedAspectRatio"].as<bool>();
			}

			auto circleRendererComponent = entity["CircleRendererComponent"];
			if (circleRendererComponent)
			{
				auto& crc = deserializedEntity->AddComponent<CircleRendererComponent>();
				crc.enabled = circleRendererComponent["enabled"].as<bool>();
				crc.color = circleRendererComponent["color"].as<glm::vec4>();
				crc.thickness = circleRendererComponent["thickness"].as<float>();
				crc.fade = circleRendererComponent["fade"].as<float>();
			}

			auto meshComponent = entity["MeshComponent"];
			if (meshComponent)
			{
				auto& mc = deserializedEntity->AddComponent<MeshComponent>();
				mc.meshType = (MeshType)meshComponent["meshType"].as<int>();
				mc.modelPath = meshComponent["modelPath"].as<std::string>();
			}

			auto meshRendererComponent = entity["MeshRendererComponent"];
			if (meshRendererComponent)
			{
				auto& mrc = deserializedEntity->AddComponent<MeshRendererComponent>();
				mrc.enabled = meshRendererComponent["enabled"].as<bool>();
               
				mrc.materialLibraryKey.key     = meshRendererComponent["materialLibraryKey"].as<std::string>();
				mrc.materialLibraryKey.keyHash = std::hash<std::string>{}(mrc.materialLibraryKey.key);
				mrc.color                      = meshRendererComponent["color"].as<glm::vec4>();
				mrc.uvRect                     = meshRendererComponent["uvRect"].as<glm::vec4>();
				mrc.parallaxScale              = meshRendererComponent["parallaxScale"].as<float>();
				mrc.tilingFactor               = meshRendererComponent["tilingFactor"].as<float>();
				mrc.thickness                  = meshRendererComponent["thickness"].as<float>();
				mrc.fade                       = meshRendererComponent["fade"].as<float>();
				mrc.flags                      = meshRendererComponent["flags"].as<uint32_t>();
				mrc.outlineColor               = meshRendererComponent["outlineColor"].as<glm::vec4>();
				mrc.outlineScale               = meshRendererComponent["outlineScale"].as<glm::vec3>();
				mrc.explosionOffset            = meshRendererComponent["explosionOffset"].as<float>();
				mrc.normalVisualizationColor   = meshRendererComponent["normalVisualizationColor"].as<glm::vec4>();
				mrc.normalVisualizationLength  = meshRendererComponent["normalVisualizationLength"].as<float>();
				mrc.normalVisualizationIndex1  = meshRendererComponent["normalVisualizationIndex1"].as<int>();
				mrc.normalVisualizationIndex2  = meshRendererComponent["normalVisualizationIndex2"] .as<int>();
			}

			auto lightComponent = entity["LightComponent"];
			if (lightComponent)
			{
				auto& lc = deserializedEntity->AddComponent<LightComponent>();
				lc.enabled       = lightComponent["enabled"].as<bool>();
				lc.type          = (LightComponent::LightType)lightComponent["type"].as<uint32_t>();
				lc.ambient       = lightComponent["ambient"].as<glm::vec3>();
				lc.diffuse       = lightComponent["diffuse"].as<glm::vec3>();
				lc.specular      = lightComponent["specular"].as<glm::vec3>();
				lc.constant      = lightComponent["constant"].as<float>();
				lc.linear        = lightComponent["linear"].as<float>();
				lc.quadratic     = lightComponent["quadratic"].as<float>();
				lc.cutoff        = lightComponent["cutoff"].as<float>();
				lc.outerCutoff   = lightComponent["outerCutoff"] .as<float>();
				lc.radius        = lightComponent["radius"].as<float>();
				lc.shadowEnabled = lightComponent["shadowEnabled"].as<int>();
			}

			auto skyboxComponent = entity["SkyboxComponent"];
			if (skyboxComponent)
			{
				auto& sc = deserializedEntity->AddComponent<SkyboxComponent>();
				sc.enabled = skyboxComponent["enabled"].as<bool>();
				sc.primary = skyboxComponent["primary"].as<bool>();
				sc.textureType = skyboxComponent["textureType"].as<int>();

				const auto& texturesNode = skyboxComponent["textures"];
				for (uint32_t i = 0; i < texturesNode.size() && i < 6; i++)
				{
					const auto& texNode = texturesNode[i];
					sc.textures[i].key = texNode["key"].as<std::string>();
					sc.textures[i].keyHash = texNode["keyHash"].as<uint64_t>();
					sc.textures[i].flip = texNode["flip"].as<bool>();
					sc.textures[i].internalFormat = (TextureInternalFormat)texNode["internalFormat"].as<int>();
				}
				sc.ResetTextureCubeSixSided();

				std::string textureCubeMap = skyboxComponent["textureCubeMap"].as<std::string>();
				if (!textureCubeMap.empty())
					sc.textureCubeMap = TextureCube::Create(textureCubeMap);
			}

			auto hdrComponent = entity["HDRComponent"];
			if (hdrComponent)
			{
				auto& hc = deserializedEntity->AddComponent<HDRComponent>();
				hc.enabled = hdrComponent["enabled"].as<bool>();
				hc.primary = hdrComponent["primary"].as<bool>();

				const auto& equirectangularMapKeyNode = hdrComponent["equirectangularMapKey"];
				hc.equirectangularMapKey.key     = equirectangularMapKeyNode["key"].as<std::string>();
				hc.equirectangularMapKey.keyHash = equirectangularMapKeyNode["keyHash"].as<uint64_t>();
				hc.equirectangularMapKey.flip    = equirectangularMapKeyNode["flip"].as<bool>();
				hc.equirectangularMapKey.internalFormat = (TextureInternalFormat)equirectangularMapKeyNode["internalFormat"].as<int>();

				hc.UpdateEnvCubeMap();
			}

			auto particleSystemComponent = entity["ParticleSystemComponent"];
			if (particleSystemComponent)
			{
				auto& pc = deserializedEntity->AddComponent<ParticleSystemComponent>();
				pc.enabled = particleSystemComponent["enabled"].as<bool>();
				auto& particleSystem = pc.particleSystem;

				particleSystem->duration               = particleSystemComponent["duration"].as<float>();
				particleSystem->looping                = particleSystemComponent["looping"].as<bool>();
				particleSystem->prewarm                = particleSystemComponent["prewarm"].as<bool>();
				particleSystem->startDelay1            = particleSystemComponent["startDelay1"].as<float>();
				particleSystem->startDelay2            = particleSystemComponent["startDelay2"].as<float>();
				particleSystem->startDelayType         = particleSystemComponent["startDelayType"].as<int>();
				particleSystem->startLifetime1         = particleSystemComponent["startLifetime1"].as<float>();
				particleSystem->startLifetime2         = particleSystemComponent["startLifetime2"].as<float>();
				particleSystem->startLifetimeType      = particleSystemComponent["startLifetimeType"].as<int>();
				particleSystem->startSpeed1            = particleSystemComponent["startSpeed1"].as<float>();
				particleSystem->startSpeed2            = particleSystemComponent["startSpeed2"].as<float>();
				particleSystem->startSpeedType         = particleSystemComponent["startSpeedType"].as<int>();
				particleSystem->useThreeDStartSize     = particleSystemComponent["useThreeDStartSize"].as<bool>();
				particleSystem->threeDStartSize1       = particleSystemComponent["threeDStartSize1"].as<glm::vec3>();
				particleSystem->threeDStartSize2       = particleSystemComponent["threeDStartSize2"].as<glm::vec3>();
				particleSystem->startSize1             = particleSystemComponent["startSize1"].as<float>();
				particleSystem->startSize2             = particleSystemComponent["startSize2"].as<float>();
				particleSystem->startSizeType          = particleSystemComponent["startSizeType"].as<int>();
				particleSystem->useThreeDStartRotation = particleSystemComponent["useThreeDStartRotation"].as<bool>();
				particleSystem->threeDStartRotation1   = particleSystemComponent["threeDStartRotation1"].as<glm::vec3>();
				particleSystem->threeDStartRotation2   = particleSystemComponent["threeDStartRotation2"].as<glm::vec3>();
				particleSystem->startRotation1         = particleSystemComponent["startRotation1"].as<float>();
				particleSystem->startRotation2         = particleSystemComponent["startRotation2"].as<float>();
				particleSystem->startRotationType      = particleSystemComponent["startRotationType"].as<int>();
				particleSystem->flipRotation           = particleSystemComponent["flipRotation"].as<float>();
				particleSystem->startColor1            = particleSystemComponent["startColor1"].as<glm::vec4>();
				particleSystem->startColor2            = particleSystemComponent["startColor2"].as<glm::vec4>();
				particleSystem->startColorType         = particleSystemComponent["startColorType"].as<int>();
				particleSystem->simulationSpace        = particleSystemComponent["simulationSpace"].as<int>();
				particleSystem->simulationSpeed        = particleSystemComponent["simulationSpeed"].as<float>();
				particleSystem->isDeltaTimeScaled      = particleSystemComponent["isDeltaTimeScaled"].as<bool>();
				particleSystem->playOnAwake            = particleSystemComponent["playOnAwake"].as<bool>();
				particleSystem->maxParticles           = particleSystemComponent["maxParticles"].as<int>();

				auto& emission = particleSystem->emission;
				const auto& emissionNode = particleSystemComponent["emission"];

				emission.enabled                = emissionNode["enabled"].as<bool>();             
				emission.rateOverTime1			= emissionNode["rateOverTime1"].as<float>();       
				emission.rateOverTime2			= emissionNode["rateOverTime2"].as<float>();       
				emission.rateOverTimeType		= emissionNode["rateOverTimeType"].as<int>();
				emission.rateOverDistance1		= emissionNode["rateOverDistance1"].as<float>();   
				emission.rateOverDistance2		= emissionNode["rateOverDistance2"].as<float>();   
				emission.rateOverDistanceType	= emissionNode["rateOverDistanceType"].as<int>();

				const auto& burstsNode = emissionNode["bursts"];
				for (const auto& burstNode : burstsNode)
				{
					ParticleSystem_Emission::Burst burst = {
						burstNode["time"].as<float>(),
						burstNode["count1"].as<float>(),
						burstNode["count2"].as<float>(),
						burstNode["countType"].as<int>(),
						burstNode["cycles"].as<uint32_t>(),
						burstNode["cyclesType"].as<int>(),
						burstNode["interval"].as<float>(),
						burstNode["probability"].as<float>()
					};

					emission.bursts.push_back(burst);
				}


				auto& shape = particleSystem->shape;
				const auto& shapeNode = particleSystemComponent["shape"];

				shape.enabled         = shapeNode["enabled"].as<bool>();
				shape.shape           = (ParticleSystem_Shape::Shape)shapeNode["shape"].as<int>();
				shape.angle           = shapeNode["angle"].as<float>();
				shape.radius          = shapeNode["radius"].as<float>();
				shape.radiusThickness = shapeNode["radiusThickness"].as<float>();
				shape.arc             = shapeNode["arc"].as<float>();
				shape.boxThickness    = shapeNode["boxThickness"].as<glm::vec3>();
				shape.mode            = shapeNode["mode"].as<int>();
				shape.length          = shapeNode["length"].as<float>();
				shape.emitFrom        = (ParticleSystem_Shape::EmitFrom)shapeNode["emitFrom"].as<int>();
				shape.texture         = shapeNode["texture"].as<std::string>();
				shape.position        = shapeNode["position"].as<glm::vec3>();
				shape.rotation        = shapeNode["rotation"].as<glm::quat>();
				shape.scale           = shapeNode["scale"].as<glm::vec3>();


				auto& renderer = particleSystem->renderer;
				const auto& rendererNode = particleSystemComponent["renderer"];

				renderer.enabled = rendererNode["enabled"].as<bool>();
				renderer.renderMode = (ParticleSystem_Renderer::RenderMode)rendererNode["renderMode"].as<int>();
				const auto& meshesNode = rendererNode["meshes"];
				if (meshesNode && meshesNode.IsSequence())
				{
					renderer.meshes.clear();
					for (const auto& meshNode : meshesNode)
					{
						renderer.meshes.push_back(meshNode.as<int>());
					}
				}
				renderer.materialLibraryKey.key = rendererNode["materialLibraryKey"].as<std::string>();
				renderer.materialLibraryKey.keyHash = std::hash<std::string>{}(renderer.materialLibraryKey.key);

				auto& rotationOverLifetime = particleSystem->rotationOverLifetime;
				const auto& rotationOverLifetimeNode = particleSystemComponent["rotationOverLifetime"];

				rotationOverLifetime.enabled             = rotationOverLifetimeNode["enabled"].as<bool>();
				rotationOverLifetime.separateAxes        = rotationOverLifetimeNode["separateAxes"].as<bool>();
				rotationOverLifetime.angularVelocityType = rotationOverLifetimeNode["angularVelocityType"].as<int>();
				rotationOverLifetime.angularVelocity1    = rotationOverLifetimeNode["angularVelocity1"].as<float>();
				rotationOverLifetime.angularVelocity2    = rotationOverLifetimeNode["angularVelocity2"].as<float>();
				rotationOverLifetime.angularVelocity3D1  = rotationOverLifetimeNode["angularVelocity3D1"].as<glm::vec3>();
				rotationOverLifetime.angularVelocity3D2  = rotationOverLifetimeNode["angularVelocity3D2"].as<glm::vec3>();

			}

			auto scriptComponent = entity["ScriptComponent"];
			if (scriptComponent)
			{
				auto& sc = deserializedEntity->AddComponent<ScriptComponent>();
				sc.enabled = scriptComponent["enabled"].as<bool>();
				sc.ClassName = scriptComponent["className"].as<std::string>();
				Ref<ScriptClass> scriptClass = ScriptEngine::GetScriptClass(sc.ClassName, false);
				if (scriptClass != nullptr)
				{
					ScriptEngine::CreateMonoBehaviourScriptInstanceByEntity(deserializedEntity, false);

				}

				auto scriptFields = scriptComponent["scriptFields"];
				if (!scriptFields.IsNull() && scriptClass != nullptr)
				{
					auto& entityScriptFieldMap = ScriptEngine::GetEntityScriptFieldMap();

					for (auto scriptField : scriptFields)
					{
						std::string fieldName       = scriptField["name"].as<std::string>();
						uint64_t fieldNameHash      = scriptField["nameHash"].as<uint64_t>();
						std::string fieldTypeString = scriptField["type"].as<std::string>();
						ScriptFieldType fieldType   = Utils::ScriptFieldTypeFromString(fieldTypeString);

						ScriptFieldInstance* scriptFieldInstancePtr = entityScriptFieldMap.TryGetField(deserializedEntity->GetUUID(), scriptClass->GetFullNameHash(), fieldNameHash);
						if (scriptFieldInstancePtr == nullptr)
							continue;
						ScriptFieldInstance& scriptFieldInstance = *scriptFieldInstancePtr;

						switch (fieldType)
						{
							READ_SCRIPT_FIELD(Float, float);
							READ_SCRIPT_FIELD(Double, double);
							READ_SCRIPT_FIELD(Bool, bool);
							READ_SCRIPT_FIELD(Char, char);
							READ_SCRIPT_FIELD(Byte, int8_t);
							READ_SCRIPT_FIELD(Short, int16_t);
							READ_SCRIPT_FIELD(Int, int32_t);
							READ_SCRIPT_FIELD(Long, int64_t);
							READ_SCRIPT_FIELD(UByte, int);
							READ_SCRIPT_FIELD(UShort, uint16_t);
							READ_SCRIPT_FIELD(UInt, uint32_t);
							READ_SCRIPT_FIELD(ULong, uint64_t);
							READ_SCRIPT_FIELD(String, std::string);
							READ_SCRIPT_FIELD(Vector2, glm::vec2);
							READ_SCRIPT_FIELD(Vector3, glm::vec3);
							READ_SCRIPT_FIELD(Vector4, glm::vec4);
							READ_SCRIPT_FIELD(Quaternion, glm::quat);
							READ_SCRIPT_FIELD(Matrix4x4, glm::mat4);
							READ_SCRIPT_FIELD(Object, UUID);
							READ_SCRIPT_FIELD(GameObject, UUID);
							READ_SCRIPT_FIELD(Component, UUID);
							READ_SCRIPT_FIELD(Transform, UUID);
							READ_SCRIPT_FIELD(Behaviour, UUID);
							READ_SCRIPT_FIELD(MonoBehaviour, UUID);
						}
					}
				}
			}

			auto rigidbody2DComponent = entity["Rigidbody2DComponent"];
			if (rigidbody2DComponent)
			{
				auto& rb2d = deserializedEntity->AddComponent<Rigidbody2DComponent>();
				rb2d.type = RigidBody2DBodyTypeFromString(rigidbody2DComponent["bodyType"].as<std::string>());
				rb2d.fixedRotation = rigidbody2DComponent["fixedRotation"].as<bool>();
			}

			auto boxCollider2DComponent = entity["BoxCollider2DComponent"];
			if (boxCollider2DComponent)
			{
				auto& bc2d = deserializedEntity->AddComponent<BoxCollider2DComponent>();
				bc2d.enabled = boxCollider2DComponent["enabled"].as<bool>();
				bc2d.offset = boxCollider2DComponent["offset"].as<glm::vec2>();
				bc2d.size = boxCollider2DComponent["size"].as<glm::vec2>();
				bc2d.density = boxCollider2DComponent["density"].as<float>();
				bc2d.friction = boxCollider2DComponent["friction"].as<float>();
				bc2d.restitution = boxCollider2DComponent["restitution"].as<float>();
				bc2d.restitutionThreshold = boxCollider2DComponent["restitutionThreshold"].as<float>();
			}

			auto circleCollider2DComponent = entity["CircleCollider2DComponent"];
			if (circleCollider2DComponent)
			{
				auto& cc2d = deserializedEntity->AddComponent<CircleCollider2DComponent>();
				cc2d.enabled = circleCollider2DComponent["enabled"].as<bool>();
				cc2d.offset = circleCollider2DComponent["offset"].as<glm::vec2>();
				cc2d.radius = circleCollider2DComponent["radius"].as<float>();
				cc2d.density = circleCollider2DComponent["density"].as<float>();
				cc2d.friction = circleCollider2DComponent["friction"].as<float>();
				cc2d.restitution = circleCollider2DComponent["restitution"].as<float>();
				cc2d.restitutionThreshold = circleCollider2DComponent["restitutionThreshold"].as<float>();
			}

			auto entities = entity["Entities"];
			if (entities)
				for (auto entity : entities)
					DeserializeEntity(entity, scene, deserializedEntity);

			return deserializedEntity;
		}

	}
}