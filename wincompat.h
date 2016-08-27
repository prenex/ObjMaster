#pragma once
#ifndef OBJMASTER_WIN_COMPAT
#define OBJMASTER_WIN_COMPAT

// This file is intended to add various clever or dirty tricks to achieve windows MSVC compatibility...
// Only do these things if the compiler is M$C...
#ifdef _MSC_VER
// Silly hax for getting rid of errors arising from missing proper posix method names on windows MSVC...
#define strdup _strdup
// strtop_r is not present in MSVC, but there is this random method that basically do the same...
#define strtok_r strtok_s
#endif

#endif