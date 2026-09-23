#pragma once

#include "Volcano/ParticleSystem/Particle.h"
#include "Volcano/ParticleSystem/ParticleModules.h"
#include "Volcano/Renderer/Camera.h"

namespace Volcano
{
	class Entity;

	class VOL_API ParticleSystem
	{
	public:
		ParticleSystem();
		~ParticleSystem();

		ParticleSystem(ParticleSystem& other);

		void Play();
		void Pause();
		void Restart();
		void Stop();

		void Update(const glm::vec3& position, const glm::vec3& rotation);

		void Render();


		void EmissionParticles();
		uint32_t CreateParticle();
		void UpdateParticles();
		void RenderParticles();
		uint64_t GetParticleCount() { return m_Particles.size(); }
	public:
		Entity* entity;

		glm::vec3 cameraPosition;                     // 摄像头世界坐标
		glm::vec3 cameraRotation;                     // 摄像头世界欧拉角

		bool      isPlaying;                          // 当前是否正在播放
		float     lastEmissionTime;                   // 上一次生成粒子的时间
		float     playbackSpeed;                      // 粒子播放速度
		float     playbackTime;                       // 当前系统运行到了第几秒（用于计算生命周期进度）

		float     duration;                           // 一个完整循环的时长（秒）
		bool      looping;                            // 是否循环播放
		bool      prewarm;                            // 预热，系统启动时假装已经运行了很久
		float     startDelay1, startDelay2;           // 启动延迟。点击 Play 后，等几秒才开始喷粒子，区间
		int       startDelayType;                     // 0: 常量 1: 两个常数之间的随机值
		float     startLifetime1, startLifetime2;     // 粒子生成时的剩余寿命（秒），区间
		int       startLifetimeType;                  // 0: 常量 1: 曲线 2: 两个常数之间的随机值 3: 两条曲线间的随机
		float     startSpeed1, startSpeed2;           // 粒子生成时的初速度（米/秒）
		int       startSpeedType;                     // 0: 常量 1: 曲线 2: 两个常数之间的随机值 3: 两条曲线间的随机
		bool      useThreeDStartSize;                 // 是否允许3D控制缩放，true用threeDStartSize，false用startSize，二者选其一，二者公用startSizeType
		glm::vec3 threeDStartSize1, threeDStartSize2; // 3D 缩放时的 X、Y、Z 随机范围
		float     startSize1, startSize2;             // 粒子生成时的大小，区间
		int       startSizeType;                      // 0: 常量 1: 曲线 2: 两个常数之间的随机值 3: 两条曲线间的随机
		bool      useThreeDStartRotation;             // 是否允许3D控制旋转，true用threeDStartRotation，false用startRotation，二者选其一，二者公用startRotationType
		glm::vec3 threeDStartRotation1, threeDStartRotation2;
		float     startRotation1, startRotation2;     // 初始旋转角度（弧度）
		int       startRotationType;                  // 0: 常量 1: 曲线 2: 两个常数之间的随机值 3: 两条曲线间的随机
		float     flipRotation;                       // 翻转旋转，0 不翻转，1 随机反向
		glm::vec4 startColor1, startColor2;           // 粒子生成时的颜色RGBA
		int       startColorType;                     // 0: 颜色 1: 渐变 2: 两个常数之间的随机值 3: 两条渐变之间的随机 4、随机颜色
		int       simulationSpace;                    // 模拟空间，0: local（粒子随粒子发射器移动）   1: world  （粒子发射后定在世界坐标不动，如烟雾）  2: Custom
		float     simulationSpeed;                    // 模拟时间流逝速度, [0.0, 100.0f]
		bool      isDeltaTimeScaled;                  // 决定时间差是否缩放 true: Scaled，false: Unscaled
		bool      playOnAwake;		                  // 唤醒时播放
		int       maxParticles;                       // 最大粒子数

		ParticleSystem_Emission emission;
		ParticleSystem_Shape    shape;
		ParticleSystem_Renderer renderer;
		ParticleSystem_RotationOverLifetime rotationOverLifetime;

	private:
		std::random_device m_RandomDevice;  // 用于获取一个随机的种子值
		std::mt19937 m_mt19937;             // 使用随机设备作为种子初始化梅森旋转算法引擎 

		std::vector<Particle> m_Particles;     // 粒子内存池
		std::vector<uint32_t> m_ActiveIndices; // 当前存活的粒子索引
	};

}