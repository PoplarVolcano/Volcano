#pragma once

#include "glm/glm.hpp"

namespace Volcano
{

	// 方块碰撞体
	struct BoxCollider2DComponent
	{
		bool enabled = true;

		glm::vec2 offset = { 0.0f, 0.0f };
		glm::vec2 size = { 0.5f, 0.5f };

		// TODO:移到物理材质
		// 密度,0是静态的物理
		float density = 1.0f;
		// 摩擦力
		float friction = 0.5f;
		// 弹力，0不会弹跳，1无限弹跳
		float restitution = 0.5f;
		// 复原速度阈值，超过这个速度的碰撞就会被恢复原状（会反弹）。
		float restitutionThreshold = 0.5f;

		// 运行时候由于物理，每一帧的上述参数可能会变，所以保存为对象,但未使用
		void* runtimeFixture = nullptr;

		BoxCollider2DComponent() = default;
		BoxCollider2DComponent(const BoxCollider2DComponent&) = default;
	};

}