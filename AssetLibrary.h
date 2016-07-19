//
// Created by rthier on 2016.04.11..
//

#ifndef NFTSIMPLEPROJ_ASSETLIBRARY_H
#define NFTSIMPLEPROJ_ASSETLIBRARY_H

#include <istream> // std::istream header
#include <memory>

/**
 * An abstract class for loosely coupled asset loading. An android application should use the
 * AssetManager or fopen while a webgl application might use some other technique etc. etc.
 */
class AssetLibrary {
public:
    /** Pure virtual function for getting a stream to the given asset(path contains '/' in the end) */
    	virtual std::unique_ptr<std::istream> getAssetStream(const char *path,
                                const char *assetFileName) const = 0;
};


#endif //NFTSIMPLEPROJ_ASSETLIBRARY_H
