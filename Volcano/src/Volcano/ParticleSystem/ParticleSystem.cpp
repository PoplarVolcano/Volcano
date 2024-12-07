#include "volpch.h"

#include "ParticleSystem.h"
#include "Volcano/Renderer/RendererItem/QuadMesh.h"
#include "Volcano/Scene/Entity.h"

#include <random>

namespace Volcano {

	ParticleSystem::ParticleSystem()
	{
		isPlaying = false;
		playbackSpeed = 1.0f;
		playbackTime  = 0.0f;
		timeStep = 0.0f;
		deltaTime = 0.0f;
		particleCount = 0;
		waitingCount = 0;
		finishCount = 0;
		state = -1;
		particleMutex = std::make_shared<std::mutex>();

		duration = 5.0f;
		looping = true;
		prewarm = false;
		startDelay1 = 0.0f, startDelay2 = 0.0f;
		startDelayType = 0;
		startLifetime1 = 5.0f, startLifetime2 = 5.0f;
		startLifetimeType = 0;
		startSpeed1 = 5.0f, startSpeed2 = 5.0f;
		startSpeedType = 0;
		threeDStartSize = false;
		startSize1 = 1.0f, startSize2 = 1.0f;
		threeDStartSize1 = glm::vec3(1.0f), threeDStartSize2 = glm::vec3(1.0f);
		startSizeType = 0;
		threeDStartRotation = false;
		startRotation1 = 0.0f, startRotation2 = 0.0f;
		threeDStartRotation1 = glm::vec3(0.0f), threeDStartRotation2 = glm::vec3(0.0f);
		startRotationType = 0;
		flipRotation = 0.0f;
		startColor1 = glm::vec4(1.0f), startColor2 = glm::vec4(1.0f);
		startColorType = 0;
		simulationSpace = 0;
		simulationSpeed = 1.0f;
		playOnAwake = true;
		maxParticles = 1000;
		particlePool = std::make_shared<ThreadPool<Particle>>(maxParticles);
		//particlePool = std::make_shared<ThreadPool<Particle>>(1);

		emission.enabled = true;
		emission.rateOverTime1 = 10;
		emission.rateOverTime2 = 10;
		emission.rateOverTimeType = 0;
		emission.rateOverDistance1 = 0;
		emission.rateOverDistance2 = 0;
		emission.rateOverDistanceType = 0;

		shape.enabled = true;
		shape.shape = ParticleSystem_Shape::Shape::Cone;
		shape.angle = 25.0f;
		shape.radius = 1.0f;
		shape.radiusThickness = 1.0f;
		shape.arc = 360.0f;
		shape.length = 5.0f;
		shape.emitFrom = true;
		shape.texture = nullptr;
		shape.position = { 0.0f, 0.0f, 0.0f };
		shape.rotation = { 0.0f, 0.0f, 0.0f };
		shape.scale    = { 0.0f, 0.0f, 0.0f };

		renderer.enabled = true;
		renderer.renderMode = ParticleSystem_Renderer::RenderMode::Billboard;
		renderer.normalDirection = 1.0f;
		renderer.material = nullptr;
		Ref<Mesh> mesh = QuadMesh::CloneRef();
		mesh->ResetMaxMeshes(maxParticles);
		renderer.meshes.push_back({ ParticleSystem_Renderer::MeshType::Quad, mesh });
	}

	ParticleSystem::~ParticleSystem()
	{
		Stop();
	}

	ParticleSystem::ParticleSystem(ParticleSystem& other)
	{
		if (other.isPlaying)
			other.Stop();
		isPlaying     = other.isPlaying;
		playbackSpeed = other.playbackSpeed;
		playbackTime  = other.playbackTime;
		timeStep      = other.timeStep;
		deltaTime     = other.deltaTime;
		particleCount = other.particleCount;
		waitingCount  = other.waitingCount;
		finishCount   = other.finishCount;
		state         = other.state;
		particleMutex = std::make_shared<std::mutex>();

		duration             = other.duration;
		looping              = other.looping;
		prewarm              = other.prewarm;
		startDelay1          = other.startDelay1;
		startDelay2          = other.startDelay2;
		startDelayType       = other.startDelayType;
		startLifetime1       = other.startLifetime1;
		startLifetime2       = other.startLifetime2;
		startLifetimeType    = other.startLifetimeType;
		startSpeed1          = other.startSpeed1;
		startSpeed2          = other.startSpeed2;
		startSpeedType       = other.startSpeedType;
		threeDStartSize      = other.threeDStartSize;
		startSize1           = other.startSize1;
		startSize2           = other.startSize2;
		threeDStartSize1     = other.threeDStartSize1;
		threeDStartSize2     = other.threeDStartSize2;
		startSizeType        = other.startSizeType;
		threeDStartRotation  = other.threeDStartRotation;
		startRotation1       = other.startRotation1;
		startRotation2       = other.startRotation2;
		threeDStartRotation1 = other.threeDStartRotation1;
		threeDStartRotation2 = other.threeDStartRotation2;
		startRotationType    = other.startRotationType;
		flipRotation         = other.flipRotation;
		startColor1          = other.startColor1;
		startColor2          = other.startColor2;
		startColorType       = other.startColorType;
		simulationSpace      = other.simulationSpace;
		simulationSpeed      = other.simulationSpeed;
		playOnAwake          = other.playOnAwake;
		maxParticles         = other.maxParticles;
		particlePool         = other.particlePool;

		emission.enabled              = other.emission.enabled;
		emission.rateOverTime1        = other.emission.rateOverTime1;
		emission.rateOverTime2        = other.emission.rateOverTime2;
		emission.rateOverTimeType     = other.emission.rateOverTimeType;
		emission.rateOverDistance1    = other.emission.rateOverDistance1;
		emission.rateOverDistance2    = other.emission.rateOverDistance2;
		emission.rateOverDistanceType = other.emission.rateOverDistanceType;

		shape.enabled         = other.shape.enabled;
		shape.shape           = other.shape.shape;
		shape.angle           = other.shape.angle;
		shape.radius          = other.shape.radius;
		shape.radiusThickness = other.shape.radiusThickness;
		shape.arc             = other.shape.arc;
		shape.length          = other.shape.length;
		shape.emitFrom        = other.shape.emitFrom;
		shape.texture         = other.shape.texture;
		shape.position        = other.shape.position;
		shape.rotation        = other.shape.rotation;
		shape.scale           = other.shape.scale;

		renderer.enabled         = other.renderer.enabled;
		renderer.renderMode      = other.renderer.renderMode;
		renderer.normalDirection = other.renderer.normalDirection;
		renderer.material        = other.renderer.material;
		for (int i = 0; i < other.renderer.meshes.size(); i++)
		{
			auto type = other.renderer.meshes[i].first;
			auto mesh = std::make_shared<Mesh>(*other.renderer.meshes[i].second.get());
			renderer.meshes.push_back({ type, mesh });
		}
	}

	void ParticleSystem::Update(float ts)
	{
		// 如果不循环且时间大于循环时长，则不再产生粒子，等待粒子全部消失后停止播放
		if (!looping && playbackTime > duration)
		{
			if (particleCount > 0)
			{
				playbackTime += ts;
				timeStep = ts;

				NotifyParticles(0);
			}
			else
			{
				isPlaying = false;
				playbackTime = 0.0f;
			}
			return;
		}

		if (isPlaying == true && emission.enabled)
		{

			// 初始化随机数引擎和分布
			std::random_device rd;  // 用于获取一个随机的种子值
			std::mt19937 mt(rd()); // 使用随机设备作为种子初始化梅森旋转算法引擎 

			std::uniform_int_distribution<>       distMeshIndex(0, renderer.meshes.size() - 1);
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
			std::uniform_real_distribution<float> distAngle(0.0f, 360.0f);
			//if(shape.angle < 0.0f)
			//	distAngle = std::uniform_real_distribution<float>(glm::radians(shape.angle), -glm::radians(shape.angle));
			//else
			//	distAngle = std::uniform_real_distribution<float>(-glm::radians(shape.angle), glm::radians(shape.angle));
			std::uniform_real_distribution<float> distRadius(0.0001f, shape.radius);

			playbackTime += ts;

			timeStep = ts;
			deltaTime += ts;
			float generateInterval;
			if (emission.rateOverTimeType == 0)
				generateInterval = 1.0f / emission.rateOverTime1;
			else if (emission.rateOverTimeType == 2)
			{
				generateInterval = 1.0f / std::uniform_real_distribution<float>(emission.rateOverTime1, emission.rateOverTime2)(mt);
			}
			else
				generateInterval = 1.0f;
			int count = deltaTime / generateInterval;
			deltaTime -= (float)count * generateInterval; // fmod返回deltaTime / generateInterval的余数

			auto& tc = entity->GetComponent<TransformComponent>();
			glm::vec3 position = tc.Translation;
			glm::quat rotation = glm::quat(tc.Rotation);
			glm::vec3 scale = tc.Scale;

			for (int i = 0; i < count && particleCount < maxParticles; i++)
			{
				Ref<Particle> particle = std::make_shared<Particle>(this);

				// 设置剩余生命周期
				if (startLifetimeType == 0)
				{
					particle->lifetime = startLifetime1;
					particle->startLifetime = particle->lifetime;
				}
				else
				{
					particle->lifetime = distStartLifetime(mt);
					particle->startLifetime = particle->lifetime;
				}

				// 设置大小
				if (threeDStartSize)
				{
					if (startSizeType == 0)
						particle->startSize = threeDStartSize1;
					else if (startSizeType == 2)
					{
						particle->startSize.x = distThreeDStartSizeX(mt);
						particle->startSize.y = distThreeDStartSizeY(mt);
						particle->startSize.z = distThreeDStartSizeZ(mt);
					}
				}
				else
				{
					if (startSizeType == 0)
						particle->startSize = glm::vec3(startSize1);
					else if (startSizeType == 2)
						particle->startSize = glm::vec3(distStartSize(mt));
				}

				// 设置旋转
				if (threeDStartRotation)
				{
					if (startRotationType == 0)
						particle->rotation = threeDStartRotation1;
					else if (startRotationType == 2)
					{
						particle->rotation.x = distThreeDStartRotationX(mt);
						particle->rotation.y = distThreeDStartRotationY(mt);
						particle->rotation.z = distThreeDStartRotationZ(mt);
					}
				}
				else
				{
					if (startRotationType == 0)
						particle->rotation.z = startRotation1;
					else if (startRotationType == 2)
						particle->rotation.z = distStartRotation(mt);
				}

				// 设置颜色
				if (startColorType == 0)
					particle->startColor = startColor1;
				else if (startColorType == 2)
				{
					particle->startColor.x = distStartColorX(mt);
					particle->startColor.y = distStartColorY(mt);
					particle->startColor.z = distStartColorZ(mt);
					particle->startColor.w = distStartColorW(mt);
				}


				if (shape.enabled)
				{
					switch (shape.shape)
					{
					case Volcano::ParticleSystem_Shape::Shape::Cone:
					{

						particle->startSize *= scale;


						float distance = shape.radius / tan(glm::radians(shape.angle)) * scale.z;
						glm::vec3 apex = glm::vec3(0.0f, 0.0f, distance); // 圆锥顶点
						float angle = distAngle(mt);
						particle->position = glm::vec3(sin(angle), cos(angle), 0.0f) * distRadius(mt) * scale;

						float speed;
						if (startSpeedType == 0)
							speed = startSpeed1;
						else if (startSpeedType == 2)
							speed = distStartSpeed(mt);
						else
							speed = 0.0f;

						particle->velocity = glm::normalize(particle->position - apex) * speed * scale.z;

						// 速度反向的时候得到的position是反的，相对圆锥底面圆心取对称
						if (speed < 0.0f)
							particle->position = -particle->position;

						particle->position = rotation * particle->position;
						particle->velocity = rotation * particle->velocity;
					}
					break;

					default:
					{
						particle->position = { 0.0f, 0.0f, 0.0f };
					}
						break;
					}
				}


				if (renderer.renderMode == ParticleSystem_Renderer::RenderMode::Billboard)
				{
					particle->meshIndex = 0;
				}
				else if (renderer.renderMode == ParticleSystem_Renderer::RenderMode::Mesh)
				{
					int meshIndex = distMeshIndex(mt);
					while (renderer.meshes[meshIndex].first != ParticleSystem_Renderer::MeshType::None)
						meshIndex = distMeshIndex(mt);
					particle->meshIndex = meshIndex;
				}

				particleMutex->lock();
				particleCount++;
				particleMutex->unlock();

				particlePool->append(particle);
				//VOL_TRACE(std::to_string(particlePool->tasks_queue.size()));

			}

			// 粒子加入线程池后不是立刻进入update待机，要先进行等待
			NotifyParticles(-1);

			// 设置update状态，唤醒所有待机粒子
			NotifyParticles(0);
		}



	}

	void ParticleSystem::UpdateEntityOnMesh()
	{
		for (int i = 0; i < renderer.meshes.size(); i++)
		{
			if (renderer.meshes[i].second != nullptr)
				renderer.meshes[i].second->SetEntity(entity);
		}
	}

	void ParticleSystem::NotifyParticles(int i)
	{
		state = i;
		waitingCount = particleCount;
		finishCount = 0;
		updateCondition.notify_all();

		while (true)
		{
			std::unique_lock<std::mutex> lock(*particleMutex.get());
			if (finishCount == particleCount)
				break;
			mainCondition.wait(lock);
		}

	}

	void ParticleSystem::Render(const Camera& camera, const glm::mat4& transform, const glm::vec3& position, const glm::vec3& rotation)
	{
		this->cameraPosition = position;
		this->cameraRotation = rotation;

		if (renderer.enabled)
		{
			if (renderer.meshes.size() == 0)
				VOL_ASSERT(false, "renderer.meshes.size() == 0");

			for (int i = 0; i < renderer.meshes.size(); i++)
				if (renderer.meshes[i].first != ParticleSystem_Renderer::MeshType::None)
				{
					renderer.meshes[i].second->BeginScene(camera, transform, position);
					renderer.meshes[i].second->StartBatch();
				}

			renderer.meshes[0].second->BindTextures({ { ImageType::Albedo, renderer.material } });
			renderer.meshes[0].second->BindShader(RenderType::PARTICLE);

			NotifyParticles(1);

			//VOL_TRACE("IndexCount: " + std::to_string(renderer.meshes[0]->GetIndexCount()));
			for (int i = 0; i < renderer.meshes.size(); i++)
				if (renderer.meshes[i].first != ParticleSystem_Renderer::MeshType::None)
				{
					renderer.meshes[i].second->EndScene();
				}

		}
		
	}

	void ParticleSystem::Play()
	{
		isPlaying = true;

		playbackTime = 0.0f;
		deltaTime = 0.0f;
		timeStep = 0.0f;
	}

	void ParticleSystem::Pause()
	{
		isPlaying = false;
	}

	void ParticleSystem::Restart()
	{
		Stop();
		Play();
	}

	void ParticleSystem::Stop()
	{
		isPlaying = false;

		playbackTime = 0.0f;
		deltaTime = 0.0f;
		timeStep = 0.0f;

		// 销毁所有粒子
		NotifyParticles(2);
	}

}