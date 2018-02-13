//
// Created by rthier on 2016.04.11..
// std redirection added on 2018.02.12 - backported because of debugging meeds for some unity projects at work
//

#ifndef NFTSIMPLEPROJ_OBJMASTERLOG_H
#define NFTSIMPLEPROJ_OBJMASTERLOG_H

#include <cstdio>
#define  OMLOG_TAG    "ObjMaster"
// Change these according to the target architecture for the ObjMaster library
// We just use stderr as default

// Change these according to your needs (if you are using std redirection as in the example below)
#define __OUTFILE "debug_out.txt"
#define __ERRFILE "debug_err.txt"
#define __WITHOUT_FLUSH_NO 5

/** Add std redirection here for your use cases if needed */
// Rem.: See example code here for one-time redirection
// Rem.: If you enable this, you also better enable an fflush at least sometimes as seen here...
// Rem.: This is much safer than just implementing different loggers for OMLOG* as this way random stdout and stderr gets logged too!
static inline void stdredirect() {
	/*
	// Simple redirect for std out to support native code logging on unity projects
	// built for example to aid us win10 devices where things go to /dev/null otherwise...
	static bool redirected = false;
	static int flushCounter = __WITHOUT_FLUSH_NO;
	if (!redirected) {
		// These files should be autoclosed by the OS in this simple solution...
		// TODO: it might be nicer(?) to buffer stuff in memory and only open files
		//       to write out a whole buffer in append mode and then close that!
		FILE *out, *err;
		freopen_s(&out, __OUTFILE, "a", stdout);
		freopen_s(&err, __ERRFILE, "a", stderr);
		redirected = true;
	}

	// Ensure there are some flushing from time to time
	if (--flushCounter < 0) {
		flushCounter = __WITHOUT_FLUSH_NO;
		fflush(stdout);
		fflush(stderr);
	}
	*/
}

#define  OMLOGD(...) stdredirect(); fprintf(stdout, "D "); fprintf(stdout, __VA_ARGS__); fprintf(stdout, "\n")
#define  OMLOGI(...) stdredirect(); fprintf(stdout, "I "); fprintf(stdout, __VA_ARGS__); fprintf(stdout, "\n")
#define  OMLOGW(...) stdredirect(); fprintf(stderr, "W "); fprintf(stderr, __VA_ARGS__); fprintf(stderr, "\n")
#define  OMLOGE(...) stdredirect(); fprintf(stderr, "E "); fprintf(stderr, __VA_ARGS__); fprintf(stderr, "\n")

#endif //NFTSIMPLEPROJ_OBJMASTERLOG_H
