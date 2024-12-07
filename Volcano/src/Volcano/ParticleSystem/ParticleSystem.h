#pragma once

#include "Queue.h"

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#define GLM_ENABLE_EXPERIMENTAL
#include "glm/gtx/quaternion.hpp"
#include "Volcano/Renderer/Texture.h"
#include "Volcano/Renderer/RendererItem/Mesh.h"
#include "Volcano/ParticleSystem/ThreadPool.h"
#include "Volcano/Renderer/Camera.h"

#include "Particle.h"

namespace Volcano {

	struct ParticleSystem_Emission
	{
		struct Burst
		{
			float time;
			float count1, count2;
			uint8_t countType;     // 0: Constant 1: Curve 2: Random between two constants 3: Random between two Curves
			uint32_t cycles;       // 循环，执行一次burst循环几轮
			uint8_t cyclesType;    // 0: Infinite 1: Count
			float interval;        // 间隔
			float probability;     // 概率
		};

		bool enabled;
		float rateOverTime1, rateOverTime2;
		uint8_t rateOverTimeType;     // 0: Constant 1: Curve 2: Random between two constants 3: Random between two Curves
		float rateOverDistance1, rateOverDistance2;
		uint8_t rateOverDistanceType; // 0: Constant 1: Curve 2: Random between two constants 3: Random between two Curves
		std::vector<Burst> bursts;
	};

	struct ParticleSystem_Shape
	{
		enum class Shape { Sphere, Hemisphere, Cone, Donut, Box, Mesh, MeshRenderer, SkinnedMeshRenderer, Sprite, SpriteRenderer, Circle, Edge, Rectangle };

		bool enabled;

		Shape shape;
		float angle;           // 角度，[0.0f, 90.0f]
		float radius;          // 半径，[0.0001f, ]
		float radiusThickness; // 半径厚度，[0.0f, 1.0f];
		float arc;             // 多少角度内释放粒子，[0.0f, 360.0f]
		float length;          // 长度，[0.0f, ]
		bool emitFrom;         // true: Base false: Volume  从底部发射还是shape体内发射
		Ref<Texture2D> texture;

		glm::vec3 position;
		glm::vec3 rotation;
		glm::vec3 scale;

		glm::mat4 GetTransform() const
		{
			glm::mat4 rotation = glm::toMat4(glm::quat(rotation));

			return glm::translate(glm::mat4(1.0f), position)
				* rotation
				* glm::scale(glm::mat4(1.0f), scale);
		}
		glm::mat3 GetNormalTransform() const
		{
			return  glm::mat3(transpose(inverse(GetTransform())));
		}
	};

	struct ParticleSystem_VelocityOverLifetime
	{
		bool enabled;
	};

	struct ParticleSystem_LimitVelocityOverLifetime
	{
		bool enabled;
	};

	struct ParticleSystem_InheritVelocity
	{
		bool enabled;
	};

	struct ParticleSystem_LifetimebyEmitterSpeed
	{
		bool enabled;
	};

	struct ParticleSystem_ForceOverLifetime
	{
		bool enabled;
	};

	struct ParticleSystem_ColorOverLifetime
	{
		bool enabled;
	};

	struct ParticleSystem_ColorBySpeed
	{
		bool enabled;
	};

	struct ParticleSystem_SizeOverLifetime
	{
		bool enabled;
	};

	struct ParticleSystem_SizeBySpeed
	{
		bool enabled;
	};

	struct ParticleSystem_RotationOverLifetime
	{
		bool enabled;
	};

	struct ParticleSystem_RotationBySpeed
	{
		bool enabled;
	};

	struct ParticleSystem_ExternalForces
	{
		bool enabled;
	};

	struct ParticleSystem_Noise
	{
		bool enabled;
	};

	struct ParticleSystem_Collision
	{
		bool enabled;
	};

	struct ParticleSystem_Triggers
	{
		bool enabled;
	};

	struct ParticleSystem_SubEmitters
	{
		bool enabled;
	};

	struct ParticleSystem_TextureSheetAnimation
	{
		bool enabled;
	};

	struct ParticleSystem_Lights
	{
		bool enabled;
	};

	struct ParticleSystem_Trails
	{
		bool enabled;
	};

	struct ParticleSystem_CustomData
	{
		bool enabled;
	};

	struct ParticleSystem_Renderer
	{
		enum class RenderMode { Billboard, StrectchedBillboard, HorizontalBillboard, VerticalBillboard, Mesh, None };
		enum class MeshType { None, Cube, Capsule, Cylinder, Plane, Sphere, Quad };
		bool enabled;

		RenderMode renderMode;

		float normalDirection;
		Ref<Texture2D> material;

		std::vector<std::pair<MeshType, Ref<Mesh>>> meshes;
		std::mutex mutex;
	};

	class ParticleSystem;

	class Entity;

	class ParticleSystem
	{
	public:
		ParticleSystem();
		~ParticleSystem();

		ParticleSystem(ParticleSystem& other);

		void Update(float ts);
		void Render(const Camera& camera, const glm::mat4& transform, const glm::vec3& position, const glm::vec3& rotation);

		void Play();
		void Pause();
		void Restart();
		void Stop();

		void UpdateEntityOnMesh();
	private:
		Ref<ThreadPool<Particle>> particlePool;
		//Queue<Particle> particlePool;

		// 等待所有粒子进入待机
		void NotifyParticles(int i);


	public:
		Entity*   entity;

		glm::vec3 cameraPosition;
		glm::vec3 cameraRotation;

		bool      isPlaying;
		float     playbackSpeed;
		float     playbackTime;
		float     timeStep;     // 每次更新时的时间步
		float     deltaTime;    // 不足一次粒子生成的时间
		int       particleCount;

		float     duration;  // 一个循环的时长，[0.05f, 100000.0f]
		bool      looping;
		bool      prewarm;
		float     startDelay1, startDelay2; // [0.0f,]
		uint8_t   startDelayType;    // 0: Constant 1: Random between two constants
		float     startLifetime1, startLifetime2; // 粒子生成时的剩余寿命，[0.0001, ]
		uint8_t   startLifetimeType; // 0: Constant 1: Curve 2: Random between two constants 3: Random between two Curves
		float     startSpeed1, startSpeed2;       // 粒子生成时的速度
		uint8_t   startSpeedType;    // 0: Constant 1: Curve 2: Random between two constants 3: Random between two Curves
		bool      threeDStartSize;
		float     startSize1, startSize2;         // 粒子生成时的大小，[0.0f,]
		glm::vec3 threeDStartSize1, threeDStartSize2;
		uint8_t   startSizeType;     // 0: Constant 1: Curve 2: Random between two constants 3: Random between two Curves
		bool      threeDStartRotation;
		float     startRotation1, startRotation2;
		glm::vec3 threeDStartRotation1, threeDStartRotation2;
		uint8_t   startRotationType; // 0: Constant 1: Curve 2: Random between two constants 3: Random between two Curves
		float     flipRotation;      // 翻转旋转，[0.0f, 1.0f]
		glm::vec4 startColor1, startColor2;        // 粒子生成时的颜色，[0.0f,]
		uint8_t   startColorType;    // 0: Color    1: Gradient 2: Random between two Colors 3: Random between two Gradients 4: Rnadom Color
		uint8_t   simulationSpace;   // 0: local    1: world    2: Custom
		float     simulationSpeed;   // 模拟时间流逝速度, [0.0, 100.0f]
		bool      playOnAwake;
		int       maxParticles;

		ParticleSystem_Emission                  emission;
		ParticleSystem_Shape                     shape;
		ParticleSystem_VelocityOverLifetime      velocityOverLifetime;
		ParticleSystem_LimitVelocityOverLifetime limitVelocityOverLifetime;
		ParticleSystem_InheritVelocity           inheritVelocity;
		ParticleSystem_LifetimebyEmitterSpeed    lifetimebyEmitterSpeed;
		ParticleSystem_ForceOverLifetime         forceOverLifetime;
		ParticleSystem_ColorOverLifetime         colorOverLifetime;
		ParticleSystem_ColorBySpeed              colorBySpeed;
		ParticleSystem_SizeOverLifetime          sizeOverLifetime;
		ParticleSystem_SizeBySpeed               sizeBySpeed;
		ParticleSystem_RotationOverLifetime      rotationOverLifetime;
		ParticleSystem_RotationBySpeed           rotationBySpeed;
		ParticleSystem_ExternalForces            externalForces;
		ParticleSystem_Noise                     noise;
		ParticleSystem_Collision                 collision;
		ParticleSystem_Triggers                  triggers;
		ParticleSystem_SubEmitters               subEmitters;
		ParticleSystem_TextureSheetAnimation     textureSheetAnimation;
		ParticleSystem_Lights                    lights;
		ParticleSystem_Trails                    trails;
		ParticleSystem_CustomData                customData;
		ParticleSystem_Renderer                  renderer;

		// -1: none 0: update 1: render 2:destroy
		int state;
		Ref<std::mutex> particleMutex;
		int waitingCount;
		int finishCount;
		std::condition_variable updateCondition;
		std::condition_variable mainCondition;
		
	};

}