#pragma once

#include "Volcano/Scene/SceneCamera.h"

namespace Volcano {

	struct CameraComponent
	{
		bool enabled = true;

		SceneCamera Camera;
		bool primary = true;
		bool fixedAspectRatio = true;

		CameraComponent() = default;
		CameraComponent(const CameraComponent&) = default;
	};
}