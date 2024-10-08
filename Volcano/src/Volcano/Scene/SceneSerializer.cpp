#include "volpch.h"
#include "SceneSerializer.h"

#include "Entity.h"
#include "Components.h"
#include "Volcano/Scripting/ScriptEngine.h"
#include "Volcano/Core/UUID.h"
#include "Volcano/Project/Project.h"

#include <fstream>
#include <yaml-cpp/yaml.h>

namespace YAML {

	template<>
	struct convert<glm::vec2>
	{
		static Node encode(const glm::vec2& rhs)
		{
			Node node;
			node.push_back(rhs.x);
			node.push_back(rhs.y);
			node.SetStyle(EmitterStyle::Flow);
			return node;
		}

		static bool decode(const Node& node, glm::vec2& rhs)
		{
			if (!node.IsSequence() || node.size() != 2)
				return false;

			rhs.x = node[0].as<float>();
			rhs.y = node[1].as<float>();
			return true;
		}
	};


	template<>
	struct convert<glm::vec3>
	{
		static Node encode(const glm::vec3& rhs)
		{
			Node node;
			node.push_back(rhs.x);
			node.push_back(rhs.y);
			node.push_back(rhs.z);
		}
		static bool decode(const Node& node, glm::vec3& rhs)
		{
			if (!node.IsSequence() || node.size() != 3)
				return false;

			rhs.x = node[0].as<float>();
			rhs.y = node[1].as<float>();
			rhs.z = node[2].as<float>();
			return true;
		}
	};
	template<>
	struct convert<glm::vec4>
	{
		static Node encode(const glm::vec4& rhs)
		{
			Node node;
			node.push_back(rhs.x);
			node.push_back(rhs.y);
			node.push_back(rhs.z);
			node.push_back(rhs.w);
		}
		static bool decode(const Node& node, glm::vec4& rhs)
		{
			if (!node.IsSequence() || node.size() != 4)
				return false;

			rhs.x = node[0].as<float>();
			rhs.y = node[1].as<float>();
			rhs.z = node[2].as<float>();
			rhs.w = node[3].as<float>();
			return true;
		}
	};

	template<>
	struct convert<glm::quat>
	{
		static Node encode(const glm::quat& rhs)
		{
			Node node;
			node.push_back(rhs.x);
			node.push_back(rhs.y);
			node.push_back(rhs.z);
			node.push_back(rhs.w);
		}
		static bool decode(const Node& node, glm::quat& rhs)
		{
			if (!node.IsSequence() || node.size() != 4)
				return false;

			rhs.x = node[0].as<float>();
			rhs.y = node[1].as<float>();
			rhs.z = node[2].as<float>();
			rhs.w = node[3].as<float>();
			return true;
		}
	};

	template<>
	struct convert<glm::mat4>
	{
		static Node encode(const glm::mat4& rhs)
		{
			Node node;
			for(uint32_t i = 0; i < 4; i++)
				for(uint32_t j = 0; j < 4; j++)
					node.push_back(rhs[i][j]);
		}
		static bool decode(const Node& node, glm::mat4& rhs)
		{
			if (!node.IsSequence() || node.size() != 4)
				return false;

			for (uint32_t i = 0; i < 4; i++)
				for (uint32_t j = 0; j < 4; j++)
					rhs[i][j] = node[i * 4 + j].as<float>();
			return true;
		}
	};

	template<>
	struct convert<Volcano::UUID>
	{
		static Node encode(const Volcano::UUID& uuid)
		{
			Node node;
			node.push_back((uint64_t)uuid);
			return node;
		}

		static bool decode(const Node& node, Volcano::UUID& uuid)
		{
			uuid = node.as<uint64_t>();
			return true;
		}
	};

}

namespace Volcano {

#define WRITE_SCRIPT_FIELD(FieldType, Type)           \
			case ScriptFieldType::FieldType:          \
				out << scriptField.GetValue<Type>();  \
				break

#define READ_SCRIPT_FIELD(FieldType, Type)             \
			case ScriptFieldType::FieldType:                   \
			{                                                  \
				Type data = scriptField["Data"].as<Type>();    \
				fieldInstance.SetValue(data);                  \
				break;                                         \
			}

	YAML::Emitter& operator<<(YAML::Emitter& out, const glm::vec2& v)
	{
		out << YAML::Flow;
		out << YAML::BeginSeq << v.x << v.y << YAML::EndSeq;
		return out;
	}


	YAML::Emitter& operator<<(YAML::Emitter& out, const glm::vec3& v)
	{
		out << YAML::Flow;
		out << YAML::BeginSeq << v.x << v.y << v.z << YAML::EndSeq;
		return out;
	}

	YAML::Emitter& operator<<(YAML::Emitter& out, const glm::vec4& v)
	{
		out << YAML::Flow;
		out << YAML::BeginSeq << v.x << v.y << v.z  << v.w << YAML::EndSeq;
		return out;
	}

	YAML::Emitter& operator<<(YAML::Emitter& out, const glm::quat& q)
	{
		out << YAML::Flow;
		out << YAML::BeginSeq << q.x << q.y << q.z << q.w << YAML::EndSeq;
		return out;
	}

	YAML::Emitter& operator<<(YAML::Emitter& out, const glm::mat4& m)
	{
		out << YAML::Flow;
		out << YAML::BeginSeq << 
			m[0][0] << m[0][1] << m[0][2] << m[0][3] <<
			m[1][0] << m[1][1] << m[1][2] << m[1][3] <<
			m[2][0] << m[2][1] << m[2][2] << m[2][3] <<
			m[3][0] << m[3][1] << m[3][2] << m[3][3] << YAML::EndSeq;
		return out;
	}

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

	static void SerializeEntity(YAML::Emitter& out, Entity& entity)
	{
		VOL_CORE_ASSERT(entity.HasComponent<IDComponent>());

		out << YAML::BeginMap;//Entity
		out << YAML::Key << "EntityID" << YAML::Value << entity.GetUUID();
		
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

			out << YAML::Key << "Type"        << YAML::Value << (int)lightComponent.Type;
			out << YAML::Key << "Ambient"     << YAML::Value << lightComponent.Ambient;
			out << YAML::Key << "Diffuse"     << YAML::Value << lightComponent.Diffuse;
			out << YAML::Key << "Specular"    << YAML::Value << lightComponent.Specular;
			out << YAML::Key << "Constant"    << YAML::Value << lightComponent.Constant;
			out << YAML::Key << "Linear"      << YAML::Value << lightComponent.Linear;
			out << YAML::Key << "Quadratic"   << YAML::Value << lightComponent.Quadratic;
			out << YAML::Key << "CutOff"      << YAML::Value << lightComponent.CutOff;
			out << YAML::Key << "OuterCutOff" << YAML::Value << lightComponent.OuterCutOff;

			out << YAML::EndMap;//LightComponent
		}

		if (entity.HasComponent<CameraComponent>())
		{
			out << YAML::Key << "CameraComponent";
			out << YAML::BeginMap;//CameraComponent

			auto& cameraComponent = entity.GetComponent<CameraComponent>();
			auto& camera = cameraComponent.Camera;

			out << YAML::Key << "Camera" << YAML::Value; 
			out << YAML::BeginMap;//Camera
			out << YAML::Key << "ProjectionType"   << YAML::Value << (int)camera.GetProjectionType();
			out << YAML::Key << "PerspectiveFOV"   << YAML::Value << camera.GetPerspectiveVerticalFOV();
			out << YAML::Key << "PerspectiveNear"  << YAML::Value << camera.GetPerspectiveNearClip();
			out << YAML::Key << "PerspectiveFar"   << YAML::Value << camera.GetPerspectiveFarClip();
			out << YAML::Key << "OrthographicSize" << YAML::Value << camera.GetOrthographicSize();
			out << YAML::Key << "OrthographicNear" << YAML::Value << camera.GetOrthographicNearClip();
			out << YAML::Key << "OrthographicFar"  << YAML::Value << camera.GetOrthographicFarClip();
			out << YAML::EndMap;//Camera

			out << YAML::Key << "Primary" << YAML::Value << cameraComponent.Primary;
			out << YAML::Key << "FixedAspectRatio" << YAML::Value << cameraComponent.FixedAspectRatio;

			out << YAML::EndMap;//CameraComponent
		}

		if (entity.HasComponent<ScriptComponent>())
		{
			auto& scriptComponent = entity.GetComponent<ScriptComponent>();

			out << YAML::Key << "ScriptComponent";
			out << YAML::BeginMap; // ScriptComponent
			out << YAML::Key << "ClassName" << YAML::Value << scriptComponent.ClassName;

			// 保存字段Fields
			Ref<ScriptClass> entityClass = ScriptEngine::GetEntityClass(scriptComponent.ClassName);
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
					ScriptFieldInstance& scriptField = entityFields.at(name);

					switch (field.Type)
					{
						WRITE_SCRIPT_FIELD(Float,      float    );
						WRITE_SCRIPT_FIELD(Double,     double   );
						WRITE_SCRIPT_FIELD(Bool,       bool     );
						WRITE_SCRIPT_FIELD(Char,       char     );
						WRITE_SCRIPT_FIELD(Byte,       int8_t   );
						WRITE_SCRIPT_FIELD(Short,      int16_t  );
						WRITE_SCRIPT_FIELD(Int,        int32_t  );
						WRITE_SCRIPT_FIELD(Long,       int64_t  );
						WRITE_SCRIPT_FIELD(UByte,      uint8_t  );
						WRITE_SCRIPT_FIELD(UShort,     uint16_t );
						WRITE_SCRIPT_FIELD(UInt,       uint32_t );
						WRITE_SCRIPT_FIELD(ULong,      uint64_t );
						WRITE_SCRIPT_FIELD(Vector2,    glm::vec2);
						WRITE_SCRIPT_FIELD(Vector3,    glm::vec3);
						WRITE_SCRIPT_FIELD(Vector4,    glm::vec4);
						WRITE_SCRIPT_FIELD(Quaternion, glm::quat);
						WRITE_SCRIPT_FIELD(Matrix4x4,  glm::mat4);
						WRITE_SCRIPT_FIELD(Entity,     UUID     );
					}
					out << YAML::EndMap; // ScriptFields
				}
				out << YAML::EndSeq;
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
				out << YAML::EndMap; // CircleRendererComponent
			}
		}

		if (entity.HasComponent<MeshRendererComponent>())
		{
			out << YAML::Key << "MeshRendererComponent";
			out << YAML::BeginMap; // MeshRendererComponent
			{
				auto& meshRendererComponent = entity.GetComponent<MeshRendererComponent>();
				auto& textures = meshRendererComponent.Textures;
				if (!textures.empty())
				{
					out << YAML::Key << "Textures" << YAML::Value << YAML::BeginSeq;
					for (uint32_t i = 0; i < textures.size(); i++)
					{
						out << YAML::BeginMap;
						out << YAML::Key << "ImageType" << YAML::Value << (int)textures[i].first;
						out << YAML::Key << "Path"      << YAML::Value << textures[i].second->GetPath();
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
			out << YAML::Key << "Color" << YAML::Value << circleRendererComponent.Color;
			if (circleRendererComponent.Texture)
				out << YAML::Key << "TexturePath" << YAML::Value << circleRendererComponent.Texture->GetPath();
			out << YAML::Key << "Thickness" << YAML::Value << circleRendererComponent.Thickness;
			out << YAML::Key << "Fade" << YAML::Value << circleRendererComponent.Fade;

			out << YAML::EndMap; // CircleRendererComponent
		}


		if (entity.HasComponent<SphereRendererComponent>())
		{
			out << YAML::Key << "SphereRendererComponent";
			out << YAML::BeginMap;//SphereRendererComponent
			auto& sphereRendererComponent = entity.GetComponent<SphereRendererComponent>();
			out << YAML::Key << "Color" << YAML::Value << sphereRendererComponent.Color;
			if (sphereRendererComponent.Albedo)
				out << YAML::Key << "AlbedoPath"    << YAML::Value << sphereRendererComponent.Albedo->GetPath();
			if (sphereRendererComponent.Normal)	    
				out << YAML::Key << "NormalPath"    << YAML::Value << sphereRendererComponent.Normal->GetPath();
			if (sphereRendererComponent.Metallic)   
				out << YAML::Key << "MetallicPath"  << YAML::Value << sphereRendererComponent.Metallic->GetPath();
			if (sphereRendererComponent.Roughness)
				out << YAML::Key << "RoughnessPath" << YAML::Value << sphereRendererComponent.Roughness->GetPath();
			if (sphereRendererComponent.AO)
				out << YAML::Key << "AOPath" << YAML::Value << sphereRendererComponent.AO->GetPath();
			out << YAML::EndMap;//sphereRendererComponent
		}

		if (entity.HasComponent<ModelRendererComponent>())
		{
			out << YAML::Key << "ModelRendererComponent";
			out << YAML::BeginMap;//ModelRendererComponent
			auto& modelRendererComponent = entity.GetComponent<ModelRendererComponent>();
			if (!modelRendererComponent.ModelPath.empty())
				out << YAML::Key << "ModelPath" << YAML::Value << modelRendererComponent.ModelPath;
			out << YAML::EndMap;//ModelRendererComponent
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

		if (!entity.GetEntityChildren().empty())
		{
		    out << YAML::Key << "Entities" << YAML::Value << YAML::BeginSeq;// 开始序列化
			for (auto& [name, entity] : entity.GetEntityChildren())
				SerializeEntity(out, *entity.get());
		    out << YAML::EndSeq; // 结束序列化
		}

		out << YAML::EndMap;//Entity
	}
	
	void SceneSerializer::Serialize(const std::string& filepath)
	{
		std::string sceneName = "Untitled";// TODO：获取filepath的文件名
		YAML::Emitter out;
		out << YAML::BeginMap;
		out << YAML::Key << "Scene" << YAML::Value << sceneName;
		if (!m_Scene->GetEntityNameMap().empty())
		{
			out << YAML::Key << "Entities" << YAML::Value << YAML::BeginSeq;// 开始序列化
			for (auto& [name, entity] : m_Scene->GetEntityNameMap())
				SerializeEntity(out, *entity.get());
			out << YAML::EndSeq; // 结束序列化
		}

		out << YAML::EndMap;
		std::ofstream fout(filepath);
		fout << out.c_str();
	}

	void SceneSerializer::SerializeRuntime(const std::string& filepath)
	{
		// Not implemented
		VOL_CORE_ASSERT(false);
	}

	bool DeserializeLoadEntity(YAML::Node& entity, Scene* scene, Ref<Entity> entityParent = nullptr)
	{

		uint64_t uuid = entity["EntityID"].as<uint64_t>();
		std::string name;
		auto tagComponent = entity["TagComponent"];
		if (tagComponent)
			name = tagComponent["Tag"].as<std::string>();

		VOL_CORE_TRACE("Deserialized entity with ID = {0}, name = {1}", uuid, name);

		Ref<Entity> deserializedEntity = scene->CreateEntityWithUUID(uuid, name, entityParent);

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
			sc.ClassName = scriptComponent["ClassName"].as<std::string>();

			// 获取脚本字段数据
			auto scriptFields = scriptComponent["ScriptFields"];
			if (scriptFields)
			{
				Ref<ScriptClass> entityClass = ScriptEngine::GetEntityClass(sc.ClassName);

				if (entityClass)
				{
					const auto& fields = entityClass->GetFields();
					auto& entityFields = ScriptEngine::GetScriptFieldMap(*deserializedEntity.get());

					for (auto scriptField : scriptFields)
					{
						std::string name = scriptField["Name"].as<std::string>();
						std::string typeString = scriptField["Type"].as<std::string>();
						ScriptFieldType type = Utils::ScriptFieldTypeFromString(typeString);

						ScriptFieldInstance& fieldInstance = entityFields[name];

						// TODO(Yan): turn this assert into Hazelnut log warning
						VOL_CORE_ASSERT(fields.find(name) != fields.end(), "Deserialize.ScriptComponent");

						if (fields.find(name) == fields.end())
							continue;

						fieldInstance.Field = fields.at(name);

						switch (type)
						{
							READ_SCRIPT_FIELD(Float, float);
							READ_SCRIPT_FIELD(Double, double);
							READ_SCRIPT_FIELD(Bool, bool);
							READ_SCRIPT_FIELD(Char, char);
							READ_SCRIPT_FIELD(Byte, int8_t);
							READ_SCRIPT_FIELD(Short, int16_t);
							READ_SCRIPT_FIELD(Int, int32_t);
							READ_SCRIPT_FIELD(Long, int64_t);
							READ_SCRIPT_FIELD(UByte, uint8_t);
							READ_SCRIPT_FIELD(UShort, uint16_t);
							READ_SCRIPT_FIELD(UInt, uint32_t);
							READ_SCRIPT_FIELD(ULong, uint64_t);
							READ_SCRIPT_FIELD(Vector2, glm::vec2);
							READ_SCRIPT_FIELD(Vector3, glm::vec3);
							READ_SCRIPT_FIELD(Vector4, glm::vec4);
							READ_SCRIPT_FIELD(Quaternion, glm::quat);
							READ_SCRIPT_FIELD(Matrix4x4, glm::mat4);
							READ_SCRIPT_FIELD(Entity, UUID);
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
			mc.SetMeshType(mc.meshType, deserializedEntity.get());
		}

		auto meshRendererComponent = entity["MeshRendererComponent"];
		if (meshRendererComponent)
		{
			auto& mrc = deserializedEntity->AddComponent<MeshRendererComponent>();
			auto textures = meshRendererComponent["Textures"];
			if (!textures.IsNull())
			{
				for (auto texture : textures)
				{
					ImageType type = (ImageType)texture["ImageType"].as<int>();
					std::string path = texture["Path"].as<std::string>();
					auto filePath = Project::GetAssetFileSystemPath(path);
					mrc.SetTexture(type, Texture2D::Create(filePath.string()));
				}
			}
		}

		auto circleRendererComponent = entity["CircleRendererComponent"];
		if (circleRendererComponent)
		{
			auto& crc = deserializedEntity->AddComponent<CircleRendererComponent>();
			crc.Color = circleRendererComponent["Color"].as<glm::vec4>();
			if (circleRendererComponent["TexturePath"])
				crc.Texture = Texture2D::Create(circleRendererComponent["TexturePath"].as<std::string>());
			crc.Thickness = circleRendererComponent["Thickness"].as<float>();
			crc.Fade = circleRendererComponent["Fade"].as<float>();
		}


		auto sphereRendererComponent = entity["SphereRendererComponent"];
		if (sphereRendererComponent)
		{
			auto& src = deserializedEntity->AddComponent<SphereRendererComponent>();
			src.Color = sphereRendererComponent["Color"].as<glm::vec4>();
			if (sphereRendererComponent["AlbedoPath"])
			{
				std::string albedoPath = sphereRendererComponent["AlbedoPath"].as<std::string>();
				auto path = Project::GetAssetFileSystemPath(albedoPath);
				src.Albedo = Texture2D::Create(path.string());
			}
			if (sphereRendererComponent["NormalPath"])
			{
				std::string normalPath = sphereRendererComponent["NormalPath"].as<std::string>();
				auto path = Project::GetAssetFileSystemPath(normalPath);
				src.Normal = Texture2D::Create(path.string());
			}
			if (sphereRendererComponent["MetallicPath"])
			{
				std::string metallicPath = sphereRendererComponent["MetallicPath"].as<std::string>();
				auto path = Project::GetAssetFileSystemPath(metallicPath);
				src.Metallic = Texture2D::Create(path.string());
			}
			if (sphereRendererComponent["RoughnessPath"])
			{
				std::string roughnessPath = sphereRendererComponent["RoughnessPath"].as<std::string>();
				auto path = Project::GetAssetFileSystemPath(roughnessPath);
				src.Roughness = Texture2D::Create(path.string());
			}
			if (sphereRendererComponent["AOPath"])
			{
				std::string AOPath = sphereRendererComponent["AOPath"].as<std::string>();
				auto path = Project::GetAssetFileSystemPath(AOPath);
				src.AO = Texture2D::Create(path.string());
			}
		}

		auto modelRendererComponent = entity["ModelRendererComponent"];
		if (modelRendererComponent)
		{
			auto& mrc = deserializedEntity->AddComponent<ModelRendererComponent>();
			std::string specularPath = modelRendererComponent["ModelPath"].as<std::string>();
			mrc.ModelPath = specularPath;
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

		auto entities = entity["Entities"];
		if (entities)
			for (auto entity : entities)
				DeserializeLoadEntity(entity, scene, deserializedEntity);

		return false;
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
}