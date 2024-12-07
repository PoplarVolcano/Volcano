#include "volpch.h"

#include "Particle.h"
#include "ParticleSystem.h"
#include "Volcano/Scene/Entity.h"

namespace Volcano {

	void Particle::process()
	{

		while (true)
		{
			// 每一次循环都睡眠，等待particleSystem->condition的notify_all
			std::unique_lock<std::mutex> lock(*particleSystem->particleMutex.get());

			particleSystem->finishCount++;
			particleSystem->mainCondition.notify_one();

			particleSystem->updateCondition.wait(lock);

			particleSystem->waitingCount--;

			if (particleSystem->state == 0 && particleSystem->isPlaying)
			{
				// update state

				lifetime -= particleSystem->timeStep;
				if (lifetime < 0.0f)
				{
					// 生命周期结束，销毁粒子
					particleSystem->particleCount--;
					particleSystem->mainCondition.notify_one();
					return;
				}

				position += velocity * particleSystem->timeStep;
				rotation += angularVelocity * particleSystem->timeStep;

			}
			else if(particleSystem->state == 1 && particleSystem->renderer.enabled)
			{
				// render state

				std::vector<glm::mat4> finalBoneMatrices;

				glm::vec3 up = glm::quat(particleSystem->cameraRotation) * glm::vec3(0.0f, 1.0f, 0.0f);//glm::normalize(glm::cross(glm::quat(particleSystem->cameraRotation) * glm::vec3(-1.0f, 0.0f, 0.0f), particleSystem->cameraPosition - position));
				glm::mat4 lookAt = glm::lookAt(glm::vec3(0.0f), position - particleSystem->cameraPosition, up);

				glm::mat4 transform =
					glm::translate(glm::mat4(1.0f), position)
					* glm::transpose(lookAt)
					* glm::toMat4(glm::quat(rotation))
					* glm::scale(glm::mat4(1.0f), startSize);


				glm::mat4 transformTemp = transpose(inverse(transform));
				glm::mat3 normalTransform;
				normalTransform[0] = transformTemp[0];
				normalTransform[1] = transformTemp[1];
				normalTransform[2] = transformTemp[2];
				//VOL_TRACE(std::to_string(position.x) + ", " + std::to_string(position.y) + ", " + std::to_string(position.z));
				particleSystem->renderer.meshes[meshIndex].second->DrawMesh((int)particleSystem->entity->GetEntityHandle(), finalBoneMatrices, &transform, &normalTransform);

			}
			else if (particleSystem->state == 2)
			{
				particleSystem->particleCount--;
				particleSystem->mainCondition.notify_one();
				return;
			}

		}

	}
}