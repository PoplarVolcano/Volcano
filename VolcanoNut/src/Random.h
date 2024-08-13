#pragma once
#include<random>
//< random > ：提供了随机数生成相关的类和函数。
//std::mt19937：Mersenne Twister 伪随机数生成器，可以生成高质量的随机数序列。
//std::random_device：用于生成真随机数的设备，通常是硬件随机数生成器。
//std::uniform_int_distribution：生成均匀分布的随机整数。
//std::numeric_limits<uint32_t>::max()：返回 uint32_t 类型的最大值，用于归一化随机数。

class Random
{
public:
	static void Init() {
		s_RandomEngine.seed(std::random_device()());
		// 使用随机设备生成种子，以当前时间为种子
	}
	static float Float() {
		// 生成一个范围在[0,1]之间的随机浮点数
		return (float)s_Distribution(s_RandomEngine) / (float)std::numeric_limits<uint32_t>::max();
		// 通过均匀分布生成随机数，将其归一化到[0,1]之间
	}
private:
	//Mersenne Twister 伪随机数生成器，可以生成高质量的随机数序列。
	static std::mt19937 s_RandomEngine;
	//生成均匀分布的随机整数。
	static std::uniform_int_distribution<std::mt19937::result_type> s_Distribution;
};
