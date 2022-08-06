#pragma once

#define PUBLIC_API __attribute__ ((visibility ("default")))

#ifdef __cplusplus
extern "C"
{
#endif

PUBLIC_API int get();
PUBLIC_API void set(int n);
PUBLIC_API void set2(int n);

#ifdef __cplusplus
}
#endif
