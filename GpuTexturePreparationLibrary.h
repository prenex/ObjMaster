#include <stdint.h>
#include <string>
#include <vector>
#include "Texture.h"

#ifndef GPU_TEXTURE_PREPARATION_LIBRARY_H
#define GPU_TEXTURE_PREPARATION_LIBRARY_H
namespace ObjMaster {
    /** Abstract base-class GPU-side texture preparation libraries */
    class GpuTexturePreparationLibrary {
	public:
		/** Implement this so that it load the given bitmap onto the GPU and return the 'handle' */
		virtual void loadIntoGPU(Texture &t) const = 0;
		/** Implement this so that it unload the texture data referred by the handle from the GPU */
		virtual void unloadFromGPU(Texture &t) const = 0;
    };
}
#endif
