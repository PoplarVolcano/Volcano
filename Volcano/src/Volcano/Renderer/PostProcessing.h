#pragma once

#include "Volcano/Core/Core.h"

namespace Volcano
{
	
	enum class VOL_API PostProcessingFlags
	{
		NONE      = 0,
		INVERSION = 1 << 0, // 反相
		GRAYSCALE = 1 << 1, // 灰度
		KERNEL    = 1 << 2, // 核效果
	};

	enum class VOL_API KernelFlags
	{
		NONE           = 0,
		SHARPEN        = 1 << 0, // 锐化
		BLUR           = 1 << 1, // 模糊
		EDGE_DETECTION = 1 << 2  // 边缘检测
	};

	class VOL_API PostProcessing
	{
	public:
	
	    static PostProcessing& GetInstance()
	    {
	    	static PostProcessing instance; // 第一次调用时初始化，线程安全
	    	return instance;
	    }

		PostProcessing(const PostProcessing&) = delete;
		PostProcessing& operator=(const PostProcessing&) = delete;

		PostProcessing(PostProcessing&&) = delete;
		PostProcessing& operator=(PostProcessing&&) = delete;

		uint32_t& GetPostProcessingFlags() { return m_PostProcessingFlags; }
		const uint32_t& GetPostProcessingFlags() const { return m_PostProcessingFlags; }

		inline bool HasPostProcessingFlag(uint32_t flags, PostProcessingFlags flag)
		{
			return (flags & static_cast<uint32_t>(flag)) != 0;
		}

		inline bool HasPostProcessingFlag(PostProcessingFlags flag)
		{
			return HasPostProcessingFlag(m_PostProcessingFlags, flag);
		}

		uint32_t& GetKernelFlags() { return m_KernelFlags; }
		const uint32_t& GetKernelFlags() const { return m_KernelFlags; }

		inline bool HasKernelFlag(uint32_t flags, KernelFlags flag)
		{
			return (flags & static_cast<uint32_t>(flag)) != 0;
		}

		inline bool HasKernelFlag(KernelFlags flag)
		{
			return HasKernelFlag(m_KernelFlags, flag);
		}

	private:
		PostProcessing()
			: m_PostProcessingFlags(0)
		{
		}

		~PostProcessing() = default;

	private:
		uint32_t m_PostProcessingFlags;
		uint32_t m_KernelFlags;
	};
}