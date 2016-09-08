#ifndef DX_GPU_TEXTURE_PREPARATION_LIBRARY_H
#define DX_GPU_TEXTURE_PREPARATION_LIBRARY_H

#include <d3d11_4.h>
#include "../GpuTexturePreparationLibrary.h"
#include "../Texture.h"

namespace ObjMasterExt {

	class Dx11GpuTexturePreparationLibrary : public ObjMaster::GpuTexturePreparationLibrary {
	public:
		/** Load the given bitmap onto the GPU texture memory and return 'handle' */
		virtual void loadIntoGPU(ObjMaster::Texture &t) const;
		/** Unload the texture data bound to the given handle */
		virtual void unloadFromGPU(ObjMaster::Texture &t) const;

		/**
		 * Set the direc3d device used for preparation.
		 * - Until this becomes non-nullptr all operations are just NO-OPs!
		 * - The provider is handling the device, ownership is not transferred!
		 */
		// I think shared_ptr is meaningless here as even in case we would refer to
		// the device in that way to it, we would only refer to an invalid device in
		// the bad cases, which is almost as bad as doing the random things...
		// TODO: Bad DX11 architecture here in ObjMaster...
		static void use3dDevice(ID3D11Device *device);
	private:
		static ID3D11Device *pd3dDevice; // Don't forget to initialize this!
	};
}

#endif
