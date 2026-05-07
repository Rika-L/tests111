#pragma once
// Platform compatibility shim for MSVC + MinGW-w64

#include <cstdio>
#include <cstdarg>

// Portable vsnwprintf wrapper (MSVC secure vs C99 standard)
template<size_t N>
inline int StringPrintf(wchar_t (&dest)[N], const wchar_t* format, ...)
{
    va_list args;
    va_start(args, format);
#ifdef _MSC_VER
    int ret = _vsnwprintf_s(dest, N, _TRUNCATE, format, args);
#else
    int ret = vswprintf(dest, N, format, args);
#endif
    va_end(args);
    return ret;
}

// Portable array size macro
#ifndef ARRAY_SIZE
#ifdef _MSC_VER
#define ARRAY_SIZE(arr)  _countof(arr)
#else
#define ARRAY_SIZE(arr)  (sizeof(arr) / sizeof((arr)[0]))
#endif
#endif

// MSVC uses #pragma comment(lib, ...); MinGW uses -l linker flag
#ifdef _MSC_VER
#define PRAGMA_COMMENT_LIB(lib)  __pragma(comment(lib, lib))
#else
#define PRAGMA_COMMENT_LIB(lib)
#endif
