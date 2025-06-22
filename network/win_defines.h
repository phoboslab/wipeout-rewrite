
#pragma once

#if defined(WIN32)

#ifndef socklen_t
typedef int socklen_t;
#endif

#ifndef ssize_t
#ifdef _WIN64
typedef __int64 ssize_t;
#else
typedef long ssize_t;
#endif
#endif

#endif
