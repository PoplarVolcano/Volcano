#pragma once
#include<random>
//< random > ���ṩ�������������ص���ͺ�����
//std::mt19937��Mersenne Twister α��������������������ɸ���������������С�
//std::random_device��������������������豸��ͨ����Ӳ���������������
//std::uniform_int_distribution�����ɾ��ȷֲ������������
//std::numeric_limits<uint32_t>::max()������ uint32_t ���͵����ֵ�����ڹ�һ���������

class Random
{
public:
	static void Init() {
		s_RandomEngine.seed(std::random_device()());
		// ʹ������豸�������ӣ��Ե�ǰʱ��Ϊ����
	}
	static float Float() {
		// ����һ����Χ��[0,1]֮������������
		return (float)s_Distribution(s_RandomEngine) / (float)std::numeric_limits<uint32_t>::max();
		// ͨ�����ȷֲ�����������������һ����[0,1]֮��
	}
private:
	//Mersenne Twister α��������������������ɸ���������������С�
	static std::mt19937 s_RandomEngine;
	//���ɾ��ȷֲ������������
	static std::uniform_int_distribution<std::mt19937::result_type> s_Distribution;
};
