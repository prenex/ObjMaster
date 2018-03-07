#include "wincompat.h"
#include "FileAssetLibrary.h"
#include "objmasterlog.h"
#include <istream>
#include <fstream>
#include <string>
#include <memory>
#include <cerrno> /* for strerror(errno) */

// The maximum size of error messages to print on error logging
#define ERR_MSG_SIZE 512

#ifdef __clang__
// Clang build support old clang versions without strerror_s
void strerror_secure(char* errMsgHolder, size_t bufLen, int errnum){
	strerror_r(errnum, errMsgHolder, bufLen);
}
#endif

namespace ObjMaster {

	std::unique_ptr<std::istream> FileAssetLibrary::getAssetStream(const char *path, const char *assetFileName) const {
		// We are just concatenating the two values
		const std::string spath(path);
		const std::string sassetFileName(assetFileName);
		const std::string utf8FullPath  = spath + sassetFileName;
		OMLOGI("Opening file at %s ...", utf8FullPath.c_str());
		OMLOGI(" - path %s ...", spath.c_str());
		OMLOGI(" - assetFileName %s ...", sassetFileName.c_str());
		// And returning the ifstream to it
		std::unique_ptr<std::istream> assetStream = std::make_unique<std::ifstream>(std::ifstream(
#ifdef _MSC_VER
			// Rem.: this can fail if the character cannot be represented with 16bit unicodes and this got translated to winapi that only support 16bit UNICODE spaces....
			// Rem.: When cross compiling with MSVC to android and stuff this works at least for ASCII - I could not test what happens in that case :-(
			UtfHelper::utf8_to_utf16(utf8FullPath) // MSVC has override for wstring paths - this is not standard however!
#else
			utf8FullPath				// Every normal operating system handles UTF-8 as-is, so in other cases we just do this
#endif
		));

		if (assetStream->fail()) {
			char errMsgHolder[ERR_MSG_SIZE];
			// We only log about failure. The stream we return will act as empty so we will end up parsing an empty obj
			// If this is not what you wish for, you can implement your own similar asset library on your own...
			strerror_secure(errMsgHolder, ERR_MSG_SIZE, errno);
			OMLOGE("...Cannot open file because: %s", errMsgHolder);
		} else {
			OMLOGI("...Successfully opened %s%s", path, assetFileName);
		}
		return assetStream;
	}

	std::unique_ptr<std::ostream> FileAssetLibrary::getAssetOutputStream(const char *path, const char *assetFileName) const {
		// We are just concatenating the two values
		const std::string spath(path);
		const std::string sassetFileName(assetFileName);
		const std::string utf8FullPath  = spath + sassetFileName;
		OMLOGI("Opening OUTPUT file at %s ...", utf8FullPath.c_str());
		OMLOGI(" - path %s ...", spath.c_str());
		OMLOGI(" - assetFileName %s ...", sassetFileName.c_str());
		// And returning the ifstream to it
		std::unique_ptr<std::ostream> assetStream = std::make_unique<std::ofstream>(std::ofstream(
#ifdef _MSC_VER
			// Rem.: this can fail if the character cannot be represented with 16bit unicodes and this got translated to winapi that only support 16bit UNICODE spaces....
			// Rem.: When cross compiling with MSVC to android and stuff this works at least for ASCII - I could not test what happens in that case :-(
			UtfHelper::utf8_to_utf16(utf8FullPath) // MSVC has override for wstring paths - this is not standard however!
#else
			utf8FullPath				// Every normal operating system handles UTF-8 as-is, so in other cases we just do this
#endif
		));

		if (assetStream->fail()) {
			char errMsgHolder[ERR_MSG_SIZE];
			// We only log about failure. The stream we return will act as empty so we will end up parsing an empty obj
			// If this is not what you wish for, you can implement your own similar asset library on your own...
			strerror_secure(errMsgHolder, ERR_MSG_SIZE, errno);
			OMLOGE("...Cannot open OUTPUT file because: %s", errMsgHolder);
		} else {
			OMLOGI("...Successfully opened %s%s for OUTPUT!", path, assetFileName);
		}
		return assetStream;
	}
} 

