#include <stdint.h>
#include <string>
#include <vector>
#include "Texture.h"

#ifndef TEXTURE_PREPARATION_LIBRARY_H
#define TEXTURE_PREPARATION_LIBRARY_H
namespace ObjMaster {
    /** Abstract base-class client-side texture preparation libraries */
    class TexturePreparationLibrary {
	public:
		/**
		  * Implement this function so that it load the given texture
		  * file (path+name) into a new texture object.
		  * The method should not retain any resources after its run
		  * and should close any I/O related streams - in other words
		  * this method should work in a stateless way!
		  */
		virtual Texture loadIntoMemory(const char *path,
					   const char *textureFileName) const = 0;
    };
}
#endif
