#pragma once

namespace Volcano {

	class ComponentRegister
	{
	public:
		static void TransformComponent_RegisterFunctions();
		static void CameraComponent_RegisterFunctions();
		static void Rigidbody2DComponent_RegisterFunctions();
	};
}