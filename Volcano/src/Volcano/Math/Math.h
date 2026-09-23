#pragma once

#include "Volcano/Core/Core.h"

#include <glm/glm.hpp>

namespace Volcano::Math {

	bool VOL_API DecomposeTransform(const glm::mat4& transform, glm::vec3& translation, glm::vec3& rotation, glm::vec3& scale);


}