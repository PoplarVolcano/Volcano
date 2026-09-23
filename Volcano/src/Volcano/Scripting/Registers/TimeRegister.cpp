#include "volpch.h"
#include "TimeRegister.h"

#include "Volcano/Core/Time.h"

#include <mono/metadata/object.h>

namespace Volcano
{

#define VOL_ADD_INTERNAL_CALL(Name) mono_add_internal_call("Volcano.Time::" #Name, Name)

	static void Time_GetTimeAsFloat(float* outTime)
	{
		*outTime = Time::GetSeconds();
	}

	static void Time_GetDeltaTime(float* outDeltaTime)
	{
		*outDeltaTime = Time::GetDeltaTime();
	}

	static void Time_GetTimeScale(float* outTimeScale)
	{
		*outTimeScale = Time::GetTimeScale();
	}

	static void Time_SetTimeScale(float timeScale)
	{
		Time::SetTimeScale(timeScale);
	}

	void TimeRegister::RegisterFunctions()
	{
		VOL_ADD_INTERNAL_CALL(Time_GetTimeAsFloat);
		VOL_ADD_INTERNAL_CALL(Time_GetDeltaTime);
		VOL_ADD_INTERNAL_CALL(Time_GetTimeScale);
		VOL_ADD_INTERNAL_CALL(Time_SetTimeScale);
	}

}
