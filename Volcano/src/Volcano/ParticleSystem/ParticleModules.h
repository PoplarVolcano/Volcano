#pragma once

#include "Volcano/Renderer/Texture.h"
#include "Volcano/Renderer/Material.h"

namespace Volcano
{
	// 粒子发射（Emission）
	struct ParticleSystem_Emission
	{
		// 爆发发射粒子
		struct Burst
		{
			float    time;            // 爆发时间点
			float    count1, count2;  // 爆发数
			int      countType;       // 0: 常量 1: 曲线 2: 两个常数之间的随机值 3: 两条曲线间的随机
			uint32_t cycles;          // 循环，执行一次burst循环几轮
			int      cyclesType;      // 0: 常量 1: 曲线
			float    interval;        // 爆发间隔
			float    probability;     // 爆发概率
		};

		bool    enabled;                      // 粒子发射器是否激活
		float   rateOverTime1, rateOverTime2;// 每秒发射粒子数
		int     rateOverTimeType;          // 0: 常量 1: 曲线 2: 两个常数之间的随机值 3: 两条曲线间的随机
		float   rateOverDistance1, rateOverDistance2; // 每移动 1 米发射多少粒子（用于随物体移动产生拖尾/灰尘，比如角色跑步扬尘）
		int     rateOverDistanceType;      // 0: 常量 1: 曲线 2: 两个常数之间的随机值 3: 两条曲线间的随机
		std::vector<Burst> bursts;
	};

	// 粒子发射器形状（Shape）
	struct ParticleSystem_Shape
	{
		// 球体、半球体、圆锥体、圆环、立方体、网格、网格渲染器、蒙皮网格渲染器、精灵、精灵渲染器、圆形、边、矩形
		enum class Shape { Sphere, Hemisphere, Cone, Donut, Box, Mesh, MeshRenderer, SkinnedMeshRenderer, Sprite, SpriteRenderer, Circle, Edge, Rectangle };
		enum class EmitFrom { Base, Valum, Shell, Edge };

		bool enabled;         // 粒子发射器形状是否激活，未激活时朝z轴正方向发射粒子

		Shape       shape;
		float       angle;           // 角度
		float       radius;          // 半径
		float       radiusThickness; // 半径厚度，[0.0f, 1.0f];
		float       arc;             // 多少角度内释放粒子，[0.0f, 360.0f]
		glm::vec3   boxThickness;    // Box发射器，混合模式参数;
		int         mode;            // 生成粒子模式，0：Random 1：Loop
		float       length;          // 长度
		EmitFrom    emitFrom;        // Base、Volume、Shell、Edge  从底部发射、从shape体内发射、从shape表面发射、从shape边上
		std::string texture;         // 纹理的key
							    
		glm::vec3 position;        // 粒子发射器位置
		glm::quat rotation;        // 粒子发射器欧拉角
		glm::vec3 scale;           // 粒子发射器缩放

		glm::mat4 GetTransform() const
		{
			return glm::translate(glm::mat4(1.0f), position)
				* glm::toMat4(rotation)
				* glm::scale(glm::mat4(1.0f), scale);
		}

		glm::mat3 GetNormalTransform() const
		{
			return  glm::mat3(transpose(inverse(GetTransform())));
		}
	};

	// 粒子渲染器
	struct ParticleSystem_Renderer
	{
		// 粒子渲染模式：
		// 广告牌Billboard：始终面向摄像机，常用于火球、闪光
		// 拉伸广告牌StretchedBillboard：根据速度拉伸，常用于子弹拖尾、光束
		// 水平广告牌HorizontalBillboard：世界坐标水平，正面朝上
		// 垂直广告牌VerticalBillboard：世界坐标垂直，正面朝摄像头
		// 网格Mesh：粒子渲染成真实的 3D 模型（如碎石子、弹壳）
		enum class RenderMode { Billboard, StrectchedBillboard, HorizontalBillboard, VerticalBillboard, Mesh, None };
		bool enabled;

		RenderMode renderMode;
		std::vector<int> meshes;
		MaterialLibraryKey materialLibraryKey; // 材质索引

		void AddMesh()
		{
			meshes.emplace_back(0);
		}

		void RemoveMesh(int index)
		{
			if (meshes.size() < index || index == 0)
				return;
			meshes.erase(meshes.begin() + index);
		}
	};

	struct ParticleSystem_RotationOverLifetime
	{
		bool      enabled;
		bool      separateAxes;        // 分离轴，true：3轴角速度，false：z轴角速度
		int       angularVelocityType; // 0: 常量 1: 曲线 2: 两个常数之间的随机值 3: 两条曲线间的随机
		float     angularVelocity1;
		float     angularVelocity2;
		glm::vec3 angularVelocity3D1;
		glm::vec3 angularVelocity3D2;
	};
}