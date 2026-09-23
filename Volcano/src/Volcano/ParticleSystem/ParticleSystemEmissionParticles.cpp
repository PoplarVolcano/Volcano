#include "volpch.h"
#include "ParticleSystem.h"

#include "Volcano/Core/Time.h"
#include "Volcano/Scene/Entity.h"

namespace Volcano
{

	void ParticleSystem::EmissionParticles()
	{
		// 随机数distribution分配
		std::uniform_real_distribution<float> distUniform(0.0f, 1.0f);
		std::uniform_int_distribution<>       distMeshIndex(0, (int)(renderer.meshes.size() - 1));
		std::uniform_real_distribution<float> distStartLifetime(startLifetime1, startLifetime2);
		std::uniform_real_distribution<float> distStartSpeed(startSpeed1, startSpeed2);
		std::uniform_real_distribution<float> distThreeDStartSizeX(threeDStartSize1.x, threeDStartSize2.x);
		std::uniform_real_distribution<float> distThreeDStartSizeY(threeDStartSize1.y, threeDStartSize2.y);
		std::uniform_real_distribution<float> distThreeDStartSizeZ(threeDStartSize1.z, threeDStartSize2.z);
		std::uniform_real_distribution<float> distStartSize(startSize1, startSize2);
		std::uniform_real_distribution<float> distThreeDStartRotationX(threeDStartRotation1.x, threeDStartRotation2.x);
		std::uniform_real_distribution<float> distThreeDStartRotationY(threeDStartRotation1.y, threeDStartRotation2.y);
		std::uniform_real_distribution<float> distThreeDStartRotationZ(threeDStartRotation1.z, threeDStartRotation2.z);
		std::uniform_real_distribution<float> distStartRotation(startRotation1, startRotation2);
		std::uniform_real_distribution<float> distStartColorX(startColor1.x, startColor2.x);
		std::uniform_real_distribution<float> distStartColorY(startColor1.y, startColor2.y);
		std::uniform_real_distribution<float> distStartColorZ(startColor1.z, startColor2.z);
		std::uniform_real_distribution<float> distStartColorW(startColor1.w, startColor2.w);
		std::uniform_real_distribution<float> distAngle(0.0f, shape.angle);
		std::uniform_real_distribution<float> distArc(0.0f, shape.arc);
		std::uniform_real_distribution<float> distRadius(0.0001f, shape.radius);
		std::uniform_real_distribution<float> distRadiusThickness(0.0001f, shape.radiusThickness);
		std::uniform_real_distribution<float> distBoxThicknessX(0.0f, shape.boxThickness.x);
		std::uniform_real_distribution<float> distBoxThicknessY(0.0f, shape.boxThickness.y);
		std::uniform_real_distribution<float> distBoxThicknessZ(0.0f, shape.boxThickness.z);
		std::uniform_real_distribution<float> distAngularVelocity(rotationOverLifetime.angularVelocity1, rotationOverLifetime.angularVelocity2);
		std::uniform_real_distribution<float> distAngularVelocityX(rotationOverLifetime.angularVelocity3D1.x, rotationOverLifetime.angularVelocity3D2.x);
		std::uniform_real_distribution<float> distAngularVelocityY(rotationOverLifetime.angularVelocity3D1.y, rotationOverLifetime.angularVelocity3D2.y);
		std::uniform_real_distribution<float> distAngularVelocityZ(rotationOverLifetime.angularVelocity3D1.z, rotationOverLifetime.angularVelocity3D2.z);


		// 获取帧时间与累积时间
		float deltaTime = Time::GetDeltaTime();
		playbackTime += deltaTime;

		// 计算发射间隔（emissionInterval）及本次应发射数量
		float emissionInterval;
		switch (emission.rateOverTimeType)
		{
		case 0:
			emissionInterval = 1.0f / emission.rateOverTime1;
			break;
		case 2:
			emissionInterval = 1.0f / std::uniform_real_distribution<float>(emission.rateOverTime1, emission.rateOverTime2)(m_mt19937);
			break;
		default:
			emissionInterval = 1.0f;
			break;
		}

		uint32_t count = (uint32_t)((playbackTime - lastEmissionTime) / emissionInterval);

		TransformComponent& transformComponent = entity->GetComponent<TransformComponent>();
		glm::mat4 entityTransform = entity->GetTransform();
		glm::mat4 shapeTransform = shape.GetTransform();
		glm::vec3& shapeScale = shape.scale;

		// 循环生成 count 个粒子
		for (uint32_t i = 0; i < count && m_Particles.size() < maxParticles; i++)
		{
			uint32_t index = CreateParticle();
			Particle& particle = m_Particles[index];

			// 设置剩余生命周期
			switch (startLifetimeType)
			{
			case 0:
				particle.lifetime = startLifetime1;
				particle.startLifetime = particle.lifetime;
				break;
			case 2:
				particle.lifetime = distStartLifetime(m_mt19937);
				particle.startLifetime = particle.lifetime;
				break;
			default:
				break;
			}

			// 设置大小
			if (useThreeDStartSize)
			{
				switch (startSizeType)
				{
				case 0:
					particle.startSize = threeDStartSize1;
					break;
				case 2:
					particle.startSize.x = distThreeDStartSizeX(m_mt19937);
					particle.startSize.y = distThreeDStartSizeY(m_mt19937);
					particle.startSize.z = distThreeDStartSizeZ(m_mt19937);
					break;
				default:
					break;
				}
			}
			else
			{
				switch (startSizeType)
				{
				case 0:
					particle.startSize = glm::vec3(startSize1);
					break;
				case 2:
					particle.startSize = glm::vec3(distStartSize(m_mt19937));
					break;
				default:
					break;
				}
			}
			//particle.startSize *= shapeScale;

			// 设置旋转
			if (useThreeDStartRotation)
			{
				switch (startRotationType)
				{
				case 0:
					particle.rotation = glm::quat(threeDStartRotation1);
					break;
				case 2:
					particle.rotation = glm::quat(glm::vec3(
						distThreeDStartRotationX(m_mt19937),
						distThreeDStartRotationY(m_mt19937),
						distThreeDStartRotationZ(m_mt19937)
					));
					break;
				default:
					break;
				}
			}
			else
			{
				switch (startRotationType)
				{
				case 0:
					particle.rotation = glm::quat(glm::vec3(0.0f, 0.0f, startRotation1));
					break;
				case 2:
					particle.rotation = glm::quat(glm::vec3(0.0f, 0.0f, distStartRotation(m_mt19937)));
					break;
				default:
					break;
				}
			}

			// 设置颜色
			switch (startColorType)
			{
			case 0:
				particle.startColor = startColor1;
				break;
			case 2:
				particle.startColor.x = distStartColorX(m_mt19937);
				particle.startColor.y = distStartColorY(m_mt19937);
				particle.startColor.z = distStartColorZ(m_mt19937);
				particle.startColor.w = distStartColorW(m_mt19937);
				break;
			default:
				break;
			}

			switch (rotationOverLifetime.angularVelocityType)
			{
			case 0:
			{
				if (!rotationOverLifetime.separateAxes)
				{
					particle.angularVelocity = glm::radians(rotationOverLifetime.angularVelocity1);
					particle.axisOfRotation = glm::vec3(0.0f, 0.0f, -1.0f);
				}
				else
				{
					particle.angularVelocity3D = glm::radians(rotationOverLifetime.angularVelocity3D1);
					float speed = glm::length(particle.angularVelocity3D);
					if (speed > 1e-8f) // 防止除以零
					{
						// 局部空间中的旋转轴
						particle.axisOfRotation = particle.angularVelocity3D / speed;
					}
					else
					{
						particle.angularVelocity3D = glm::vec3(0.0f);
						particle.axisOfRotation = glm::vec3(0.0f);
					}
				}
				break;
			}
			case 2:
			{
				if (!rotationOverLifetime.separateAxes)
				{
					particle.angularVelocity = glm::radians(distAngularVelocity(m_mt19937));
					particle.axisOfRotation = glm::vec3(0.0f, 0.0f, -1.0f);
				}
				else
				{
					particle.angularVelocity3D = glm::radians(glm::vec3(
						distAngularVelocityX(m_mt19937),
						distAngularVelocityY(m_mt19937),
						distAngularVelocityZ(m_mt19937)
					));
					particle.angularVelocity = glm::length(particle.angularVelocity3D);
					if (particle.angularVelocity > 1e-8f) // 防止除以零
					{
						// 局部空间中的旋转轴
						particle.axisOfRotation = particle.angularVelocity3D / particle.angularVelocity;
					}
					else
					{
						particle.angularVelocity3D = glm::vec3(0.0f);
						particle.axisOfRotation = glm::vec3(0.0f);
					}
				}
				break;
			}
			default:
				break;
			}


			if (shape.enabled)
			{
				float speed = 0.0f;;
				switch (startSpeedType)
				{
				case 0: speed = startSpeed1; break;
				case 2: speed = distStartSpeed(m_mt19937); break;
				default: break;
				}

				switch (shape.shape)
				{
				case Volcano::ParticleSystem_Shape::Shape::Sphere:
				{
					// 球面均匀分布补偿
					// 
					// 球面上，极角 phi 对应的是一个“纬线环”，环的半径是 sin(phi)。
					// 当 phi 接近 0（北极）时，环的面积趋近于 0；当 phi 接近 90°（赤道）时，环的面积最大。
					// 如果 phi 是线性均匀的，那么落在北极小面积区域内的粒子数和落在赤道大面积区域内的粒子数一样多，
					// 结果就是粒子在南北极挤成一团，赤道稀疏。
					// 
					// 球面上，从最顶部（0°）到某个角度 phi 所覆盖的那一块“球皮”的面积 2 * PI * (1 - cos(phi))
					// 让单位随机数 u 控制“面积占比”，所以 u 应该等于“当前面积”除以“最大角度对应的总面积”
					// u = 1 - cos(phi)  /  1 - cos(angle)
					// 解得 phi = arccos(1 - u * (1 - cos(angle));
					// 
					// u 是线性均匀的，但它代表的不是“角度”，而是 “1 - cos φ 这个值”
					// 因为球面面积跟 (1 - cos φ) 成正比，所以当 u 均匀时，(1 - cos φ) 就均匀，面积就均匀。
					// 而 acos 的作用，就是把这个均匀的(1 - cos φ) 翻译回对应的角度 φ。
					// 
					// 极角（纬度）：使用 acos 补偿，确保在 [0, shape.angle] 范围内立体角均匀
					float phi = std::acos(1.0f - distUniform(m_mt19937) * (1.0f - std::cos(glm::radians(shape.angle))));

					// 方位角（经度）
					float theta = glm::radians(distArc(m_mt19937));

					// 半径
					float radius = distRadius(m_mt19937);

					// 速度方向：径向向外，方位角（经度）theta以z轴为轴
					glm::vec3 velocityDirection =
						glm::vec3(std::sin(phi) * std::cos(theta), std::sin(phi) * std::sin(theta), std::cos(phi));
					// 计算局部坐标
					glm::vec3 localPosition = velocityDirection * radius * shapeScale;

					if (!(shapeScale.x == shapeScale.y && shapeScale.y == shapeScale.z))
					{
						velocityDirection = glm::normalize(localPosition);
					}

					// 如果半径极小导致方向丢失，退化为完全随机方向
					// glm::length2(v) 等价于 glm::dot(v, v)
					if (glm::length2(localPosition) < 1e-8f) {
						// 直接再调用一次随机分布生成备用方向
						float fallbackPhi = std::acos(1.0f - distUniform(m_mt19937) * (1.0f - std::cos(glm::radians(shape.angle))));
						float fallbackTheta = glm::radians(distArc(m_mt19937));
						velocityDirection = glm::vec3(
							std::sin(fallbackPhi) * std::cos(fallbackTheta),
							std::sin(fallbackPhi) * std::sin(fallbackTheta),
							std::cos(fallbackPhi)
						);
					}

					particle.velocity =
						glm::toMat4(shape.rotation)
						* glm::vec4(velocityDirection * speed, 1.0f);
					particle.position = shape.GetTransform() * glm::vec4(localPosition, 1.0f);
					
					break;
				}
				case Volcano::ParticleSystem_Shape::Shape::Hemisphere:
				{
					// 极角（纬度）：使用 acos 补偿，确保在 [shape.angle, 180.0f] 范围内立体角均匀
					float phi = std::acos(1.0f - distUniform(m_mt19937) * (1.0f - std::cos(glm::radians(shape.angle)))) + glm::radians(90.0f);

					// 方位角（经度）
					float theta = glm::radians(distArc(m_mt19937));

					// 半径
					float radius = distRadius(m_mt19937);

					// 速度方向：径向向外，方位角（经度）theta以z轴为轴
					glm::vec3 velocityDirection =
						glm::vec3(std::sin(phi) * std::cos(theta), std::sin(phi) * std::sin(theta), std::cos(phi));
					// 计算局部坐标
					glm::vec3 localPosition = velocityDirection * radius * shapeScale;

					if (!(shapeScale.x == shapeScale.y && shapeScale.y == shapeScale.z))
					{
						velocityDirection = glm::normalize(localPosition);
					}

					// 如果半径极小导致方向丢失，退化为完全随机方向
					// glm::length2(v) 等价于 glm::dot(v, v)
					if (glm::length2(localPosition) < 1e-8f) {
						// 直接再调用一次随机分布生成备用方向
						float fallbackPhi = std::acos(1.0f - distUniform(m_mt19937) * (1.0f - std::cos(glm::radians(shape.angle))));
						float fallbackTheta = glm::radians(distArc(m_mt19937));
						velocityDirection = glm::vec3(
							std::sin(fallbackPhi) * std::cos(fallbackTheta),
							std::sin(fallbackPhi) * std::sin(fallbackTheta),
							std::cos(fallbackPhi)
						);
					}

					particle.velocity =
						glm::toMat4(shape.rotation)
						* glm::vec4(velocityDirection * speed, 1.0f);
					particle.position = shape.GetTransform() * glm::vec4(localPosition, 1.0f);
					break;
				}
				case Volcano::ParticleSystem_Shape::Shape::Cone:
				{
					/*
				    计算圆锥顶点位置（基于 shape.angle、shape.radius 和缩放）。
				    							.(sin(angle), cos(angle), 0)
				    (0,0,0)                 |  /
				    \  |  /                 | /
				     \ | /                  |/∠-angle
				      \|/           ¯¯¯¯¯¯¯¯|¯¯¯¯¯¯¯¯¯¯¯¯
				    (0,0,distance)          |
				    
				    在底面圆内随机取一点（角度 distArc，半径 distRadius）。
				    速度方向为顶点指向点位置，速度大小从 startSpeed 获取。
				    若速度为负，位置取反（使粒子从锥顶反向射出）。
				    最终乘以 Entity 的旋转四元数，将位置和速度转换到世界空间。
				    */
					float angle = glm::radians(distAngle(m_mt19937));
					float angleArc = glm::radians(distArc(m_mt19937));
					float radius = distRadius(m_mt19937);

					particle.position = glm::vec3(std::sin(angleArc), std::cos(angleArc), 0.0f) * radius * shapeScale;

					glm::vec3 forwardDir(0.0f, 0.0f, -1.0f);
					glm::vec3 outwardDir;
					float r2 = particle.position.x * particle.position.x + particle.position.y * particle.position.y;
					if (r2 < 1e-8f)
					{
						outwardDir = glm::vec3(1.0f, 0.0f, 0.0f);
					}
					else
					{
						// glm::normalize(v) 的原始定义是：v / sqrt(dot(v, v))
						// 转换为 v * (1 / sqrt(dot(v, v)))
						float reciprocalR = 1.0f / glm::sqrt(r2);
						outwardDir = particle.position * reciprocalR;
					}

					float blendFactor = tan(angle);
					// 混合方向：向前 + 向外 * tan(Angle)
					glm::vec3 mixedDir = glm::normalize(forwardDir + outwardDir * blendFactor);

					particle.velocity = mixedDir * speed;

					particle.velocity = shape.rotation * glm::vec4(particle.velocity, 1.0f);
					particle.position = shape.GetTransform() * glm::vec4(particle.position * blendFactor, 1.0f);
					break;
				}
				case Volcano::ParticleSystem_Shape::Shape::Donut:
				case Volcano::ParticleSystem_Shape::Shape::Box:
				{
					glm::vec3 localPosition(0.0f);
					switch (shape.emitFrom)
					{
					case ParticleSystem_Shape::EmitFrom::Valum:
					{
						localPosition = glm::vec3(distUniform(m_mt19937), distUniform(m_mt19937), distUniform(m_mt19937));
						localPosition -= 0.5f;
						break;
					}
					case ParticleSystem_Shape::EmitFrom::Shell:
					{
						int shellIndex = (int)(distUniform(m_mt19937) * 6.0f);

						float randomFloat1 = distUniform(m_mt19937) - 0.5f;
						float randomFloat2 = distUniform(m_mt19937) - 0.5f;
						switch (shellIndex)
						{
						case 0:  localPosition = glm::vec3( 0.5f - distBoxThicknessX(m_mt19937), randomFloat1, randomFloat2); break;
						case 1:  localPosition = glm::vec3(-0.5f + distBoxThicknessX(m_mt19937), randomFloat1, randomFloat2); break;
						case 2:  localPosition = glm::vec3(randomFloat1,  0.5f - distBoxThicknessY(m_mt19937), randomFloat2); break;
						case 3:  localPosition = glm::vec3(randomFloat1, -0.5f + distBoxThicknessY(m_mt19937), randomFloat2); break;
						case 4:  localPosition = glm::vec3(randomFloat1, randomFloat2,  0.5f - distBoxThicknessZ(m_mt19937)); break;
						case 5:  localPosition = glm::vec3(randomFloat1, randomFloat2, -0.5f + distBoxThicknessZ(m_mt19937)); break;
						default: localPosition = glm::vec3(0.5f); break;
						}
						break;
					}
					case ParticleSystem_Shape::EmitFrom::Edge:
					{
						glm::vec3 boxThickness(distBoxThicknessX(m_mt19937), distBoxThicknessY(m_mt19937), distBoxThicknessZ(m_mt19937));
						int edgeIndex = (int)(distUniform(m_mt19937) * 12.0f);

						float randomFloat1 = distUniform(m_mt19937) - 0.5f;
						float randomFloat2 = (distUniform(m_mt19937) - 0.5f) * 2.0f;

						switch (edgeIndex)
						{
						// 垂直于x轴的棱
						case 0:  localPosition = glm::vec3(randomFloat1,  0.5f + randomFloat2 * shape.boxThickness.y, -0.5f + randomFloat2 * shape.boxThickness.z); break;
						case 1:  localPosition = glm::vec3(randomFloat1, -0.5f + randomFloat2 * shape.boxThickness.y, -0.5f + randomFloat2 * shape.boxThickness.z); break;
						case 2:  localPosition = glm::vec3(randomFloat1, -0.5f + randomFloat2 * shape.boxThickness.y,  0.5f + randomFloat2 * shape.boxThickness.z); break;
						case 3:  localPosition = glm::vec3(randomFloat1,  0.5f + randomFloat2 * shape.boxThickness.y,  0.5f + randomFloat2 * shape.boxThickness.z); break;
						// 垂直于y轴的棱
						case 4:  localPosition = glm::vec3(-0.5f + randomFloat2 * shape.boxThickness.x, randomFloat1, -0.5f + randomFloat2 * shape.boxThickness.z); break;
						case 5:  localPosition = glm::vec3( 0.5f + randomFloat2 * shape.boxThickness.x, randomFloat1, -0.5f + randomFloat2 * shape.boxThickness.z); break;
						case 6:  localPosition = glm::vec3( 0.5f + randomFloat2 * shape.boxThickness.x, randomFloat1,  0.5f + randomFloat2 * shape.boxThickness.z); break;
						case 7:  localPosition = glm::vec3(-0.5f + randomFloat2 * shape.boxThickness.x, randomFloat1,  0.5f + randomFloat2 * shape.boxThickness.z); break;
						// 垂直于z轴的棱
						case 8:  localPosition = glm::vec3(-0.5f + randomFloat2 * shape.boxThickness.x, -0.5f + randomFloat2 * shape.boxThickness.y, randomFloat1); break;
						case 9:  localPosition = glm::vec3(-0.5f + randomFloat2 * shape.boxThickness.x,  0.5f + randomFloat2 * shape.boxThickness.y, randomFloat1); break;
						case 10: localPosition = glm::vec3( 0.5f + randomFloat2 * shape.boxThickness.x,  0.5f + randomFloat2 * shape.boxThickness.y, randomFloat1); break;
						case 11: localPosition = glm::vec3( 0.5f + randomFloat2 * shape.boxThickness.x, -0.5f + randomFloat2 * shape.boxThickness.y, randomFloat1); break;
						default: localPosition = glm::vec3(0.5f); break;
						}

						break;
					}
					default:
						break;
					}

					particle.velocity = shape.rotation * glm::vec4(glm::vec3(0.0f, 0.0f, -1.0f) * speed, 1.0f);
					particle.position = shape.GetTransform() * glm::vec4(localPosition, 1.0f);

					break;
				}
				case Volcano::ParticleSystem_Shape::Shape::Mesh:
				case Volcano::ParticleSystem_Shape::Shape::MeshRenderer:
				case Volcano::ParticleSystem_Shape::Shape::SkinnedMeshRenderer:
				case Volcano::ParticleSystem_Shape::Shape::Sprite:
				case Volcano::ParticleSystem_Shape::Shape::SpriteRenderer:
				case Volcano::ParticleSystem_Shape::Shape::Circle:
				case Volcano::ParticleSystem_Shape::Shape::Edge:
				case Volcano::ParticleSystem_Shape::Shape::Rectangle:

				default:
					particle.position = { 0.0f, 0.0f, 0.0f };
					break;
				}
				switch (renderer.renderMode)
				{
				case ParticleSystem_Renderer::RenderMode::Billboard:
				{
					particle.meshType = (uint8_t)MeshType::Quad;
					break;
				}
				case ParticleSystem_Renderer::RenderMode::Mesh:
				{
					particle.meshType = renderer.meshes[distMeshIndex(m_mt19937)];
					break;
				}
				default:
					particle.meshType = (uint8_t)MeshType::Quad;
					break;
				}
			}
			else
			{

			}

			if (simulationSpace == 1)
			{
				// 世界坐标状态下生成的粒子
				particle.position = entityTransform * glm::vec4(particle.position, 1.0f);
				glm::quat entityRotation = glm::quat_cast(entityTransform);
				particle.velocity = entityRotation * glm::vec4(particle.velocity, 1.0f);
				particle.rotation = entityRotation * particle.rotation;

				if (!rotationOverLifetime.separateAxes)
				{

					if (particle.angularVelocity > 1e-8f) // 防止除以零
					{
						particle.axisOfRotation = entityRotation * particle.axisOfRotation;
					}
					else
					{
						particle.angularVelocity = 0.0f;
						particle.axisOfRotation = glm::vec3(0.0f);
					}
				}
				else
				{
					if (glm::dot(particle.angularVelocity3D, particle.angularVelocity3D) > 1e-8f) // 防止除以零
					{
						particle.axisOfRotation = entityRotation * particle.axisOfRotation;
						particle.angularVelocity3D = particle.axisOfRotation * particle.angularVelocity;
					}
					else
					{
						particle.angularVelocity3D = glm::vec3(0.0f);
						particle.axisOfRotation = glm::vec3(0.0f);
					}
				}
			}

			lastEmissionTime += emissionInterval;
		}
	}

}