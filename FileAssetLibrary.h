#ifndef FILE_ASSET_LIBRARY_H
#define FILE_ASSET_LIBRARY_H

#include "AssetLibrary.h"
#include <istream>
#include <fstream>
#include <string>
#include <memory>

namespace ObjMaster {
	/** Basic asset library using c++ file I/O */
	class FileAssetLibrary : public AssetLibrary {
		std::unique_ptr<std::istream> getAssetStream(const char *path, const char *assetFileName) const;
	};
}

#endif
