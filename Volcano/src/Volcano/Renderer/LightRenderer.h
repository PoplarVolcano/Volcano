#pragma once

#include "Volcano/Core/Core.h"
#include "glm/glm.hpp"

namespace Volcano {

	// 在 glsl 中，vec3 占用 12 字节，但数组元素之间的偏移按 16 字节对齐，
	// 所以 C++ 结构体必须手动填充 pad 字段，否则数据错位。

	// 结构体大小必须是 16 字节的倍数（std430 要求），80 满足。
	// 定向光（平行光）
	struct DirectionalLight
	{
		glm::vec3 direction;     // 偏移  0
		float     pad1;          // 偏移 12
		glm::vec3 ambient;       // 偏移 16
		float     pad2;          // 偏移 28
		glm::vec3 diffuse;       // 偏移 32
		float     pad3;          // 偏移 44
		glm::vec3 specular;      // 偏移 48
		int       shadowEnabled; // 偏移 60
		// 总大小：64 字节（正好 4 个 16 字节块）
	};

	// 点光源
	struct PointLight
	{
		glm::vec3 position;      // 偏移 0
		float     pad0;          // 偏移 12
		glm::vec3 ambient;       // 偏移 16
		float     pad1;          // 偏移 28
		glm::vec3 diffuse;       // 偏移 32
		float     pad2;          // 偏移 44
		glm::vec3 specular;      // 偏移 48
		float     constant;      // 偏移 60
		float     linear;        // 偏移 64
		float     quadratic;     // 偏移 68
		float     radius;        // 偏移 72
		int       shadowEnabled; // 偏移 76
		// 总大小：80 字节（正好 5 个 16 字节块）
	};

	struct PointLightShadowData
	{
		glm::mat4 lightSpaceMatrices[6];
		glm::vec3 position;
		float radius;
	};

	// 聚光(手电筒Flashlight)
	struct SpotLight
	{
		glm::vec3 position;      // 偏移 0
		float     pad0;          // 偏移 12
		glm::vec3 direction;     // 偏移 16
		float     pad1;          // 偏移 28
		glm::vec3 ambient;       // 偏移 32
		float     pad2;          // 偏移 44
		glm::vec3 diffuse;       // 偏移 48
		float     pad3;          // 偏移 60
		glm::vec3 specular;      // 偏移 64
		float     constant;      // 偏移 76
		float     linear;        // 偏移 80
		float     quadratic;     // 偏移 84
		float     cutoff;        // 偏移 88
		float     outerCutoff;   // 偏移 92
		float     radius;        // 偏移 96
		int       shadowEnabled; // 偏移 100
		float     pad[2];        // 偏移 104，补到112
		// 总大小：112 字节（正好 7 个 16 字节块）
	};

	class LightRenderer
	{
	public:
		// 获取单例实例（线程安全）
		static LightRenderer& GetInstance() {
			static LightRenderer instance; // C++11 保证线程安全初始化
			return instance;
		}

		// 禁止拷贝和赋值
		LightRenderer(const LightRenderer&) = delete;
		LightRenderer& operator=(const LightRenderer&) = delete;

		DirectionalLight* GetDirectionalLightBuffer() { return m_DirectionalLightBuffer; }
		PointLight* GetPointLightBuffer() { return m_PointLightBuffer; }
		SpotLight* GetSpotLightBuffer() { return m_SpotLightBuffer; }

	public:
		static const uint32_t MaxDirectionalLight = 4;
		static const uint32_t MaxPointLight       = 256;
		static const uint32_t MaxSpotLight        = 128;

	private:
		LightRenderer();
		~LightRenderer();

	private:
		DirectionalLight* m_DirectionalLightBuffer;
		PointLight* m_PointLightBuffer;
		SpotLight* m_SpotLightBuffer;
	};
}