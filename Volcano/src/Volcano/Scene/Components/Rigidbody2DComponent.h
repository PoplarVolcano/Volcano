#pragma once

namespace Volcano
{
	// 刚体
	struct Rigidbody2DComponent
	{
		enum class BodyType { Static = 0, Dynamic, Kinematic };
		BodyType type = BodyType::Static;
		bool fixedRotation = false;

		// 运行时物体的物理对象
		void* RuntimeBody = nullptr;

		Rigidbody2DComponent() = default;
		Rigidbody2DComponent(const Rigidbody2DComponent&) = default;
	};

}