#include "volpch.h"

#include "spdlog/sinks/stdout_color_sinks.h"

namespace Volcano {

	std::shared_ptr<spdlog::logger> Log::s_CoreLogger;
	std::shared_ptr<spdlog::logger> Log::s_ClientLogger;

	void Log::Init() {
		//����log��ʽ����ɫ��ʱ�������־������־��Ϣ
		spdlog::set_pattern("%^[%T] %n: %v%$");
		// ���ڴ���һ�����̵߳����������̨�� `logger` ����
		// ������logger���������
		s_CoreLogger = spdlog::stdout_color_mt("Volcano");
		/*
		- `trace`����ͼ������ڼ�¼һЩϵͳ����״̬�͵�����Ϣ��
		- `debug`�����ڼ�¼������Ϣ��������ֵ�ȣ���Ҫ��Ϊ�˷������Ա���Գ���
		- `info`�����ڼ�¼һЩ��Ҫ����Ϣ����������������˳��������ļ����ء��������ӵȡ�
		- `warn`�����ڱ�ʾĳЩ�����ص��쳣�򾯸���Ϣ������Ӱ��������С�
		- `error`�����ڱ�ʾ������һЩ���󣬲����ܻ�Ӱ�������������С�
		- `critical`����߼��𣬱�ʾ�����˷ǳ����ص����⣬���ܻᵼ�³��������ϵͳ������
		*/
		s_CoreLogger->set_level(spdlog::level::trace);

		s_ClientLogger = spdlog::stdout_color_mt("APP");
		s_ClientLogger->set_level(spdlog::level::trace);
	}
}