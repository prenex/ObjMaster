#include "FileAssetLibrary.h"
#include <istream>
#include <fstream>
#include <string>
#include <memory>

namespace ObjMaster {
	std::unique_ptr<std::istream> FileAssetLibrary::getAssetStream(const char *path, const char *assetFileName) const {
		// We are just concatenating the two values
		const std::string fullPath = std::string(path) + assetFileName;

		// And returning the ifstream to it
		std::unique_ptr<std::istream> assetStream = std::make_unique<std::ifstream>(std::ifstream(fullPath));
		return assetStream;
	}
} 

