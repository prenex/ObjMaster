#ifndef FILE_ASSET_LIBRARY_H
#define FILE_ASSET_LIBRARY_H

#include "AssetLibrary.h"
#include <istream>
#include <fstream>
#include <string>
#include <memory>

namespace ObjMaster {
	/** Basic asset library using c++ file I/O - can be used for input and output too! */
	class FileAssetLibrary : public AssetLibrary, public AssetOutputLibrary {
		std::unique_ptr<std::istream> getAssetStream(const char *path, const char *assetFileName) const;
		std::unique_ptr<std::ostream> getAssetOutputStream(const char *path, const char *assetFileName) const;
	};
}

#endif
