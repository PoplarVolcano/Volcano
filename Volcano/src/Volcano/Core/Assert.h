#pragma once

#ifdef VOL_DEBUG
#define VOL_ENABLE_ASSERTS
#endif

#ifdef VOL_ENABLE_ASSERTS
#define VOL_ASSERT_NO_MESSAGE(condition) { if(!(condition)) { VOL_ERROR("Assertion Failed"); __debugbreak(); } }
#define VOL_ASSERT_MESSAGE(condition, ...) { if(!(condition)) { VOL_ERROR("Assertion Failed: {0}", __VA_ARGS__); __debugbreak(); } }

#define VOL_ASSERT_RESOLVE(arg1, arg2, macro, ...) macro
#define VOL_GET_ASSERT_MACRO(...) VOL_EXPAND_VARGS(VOL_ASSERT_RESOLVE(__VA_ARGS__, VOL_ASSERT_MESSAGE, VOL_ASSERT_NO_MESSAGE))

#define VOL_ASSERT(...) VOL_EXPAND_VARGS( VOL_GET_ASSERT_MACRO(__VA_ARGS__)(__VA_ARGS__) )
#define VOL_CORE_ASSERT(...) VOL_EXPAND_VARGS( VOL_GET_ASSERT_MACRO(__VA_ARGS__)(__VA_ARGS__) )
#else
#define VOL_ASSERT(...)
#define VOL_CORE_ASSERT(...)
#endif
