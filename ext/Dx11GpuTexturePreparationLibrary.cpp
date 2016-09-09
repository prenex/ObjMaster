#include "Dx11GpuTexturePreparationLibrary.h"
#include <d3d11_4.h>

#include "../objmasterlog.h"

// TODO: I guess we should make it possible to batch-load a lot of textures and free a lot at once for better performance...
// TODO: Remove some of the darker magic?
namespace ObjMasterExt {
		// Need to define this. The header only declares...
		ID3D11Device *Dx11GpuTexturePreparationLibrary::pd3dDevice; // Don't forget to initialize this!

		/** Load the given bitmap onto the GPU texture memory and return 'handle' */
		void Dx11GpuTexturePreparationLibrary::loadIntoGPU(ObjMaster::Texture &t) const {
			// In case of DirectX, we need a p3dDevice for operating with!
			// Sadly the API is not really designed this way so we use this
			// approach of having a static class-variable that users set
			// before any usage...
			if (Dx11GpuTexturePreparationLibrary::pd3dDevice == nullptr) {
				OMLOGE("Dx11GpuTexturePreparationLibrary - Not using any provided ID3DDevice - doing a NO-OP!!!");
				return;
			}

			// The bitmap is empty when it is not loaded
			// in that case we just don't do anything
			if(t.bitmap.size() > 0) {
				// TODO: Not the most sane check for RGBA as it does not consider byte ordering!
				if (t.bytepp != 4) {
					// TODO: Currently supporting only RGBA - need at least RGB->RGBA conversion!!!
					// just do a nop otherwise!
					OMLOGE("Dx11GpuTexturePreparationLibrary  - Byte per pixel(%d) is not supported: need RGBA!", t.bytepp);
					return;
				}

				// Create texture descriptor
				D3D11_TEXTURE2D_DESC desc;
				memset(&desc, 0, sizeof(desc));
				desc.Width = t.width;
				desc.Height = t.heigth;
				desc.MipLevels = 1;
				desc.ArraySize = 1;
				desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
				desc.SampleDesc.Count = 1;
				desc.Usage = D3D11_USAGE_DEFAULT;
				desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
				desc.CPUAccessFlags = 0; //D3D11_CPU_ACCESS_WRITE;
				desc.MiscFlags = 0;

				// Create the texture resource
				// This points to the bitmap data.
				// The data vector will not change
				// while loading the image
				// so we can use it like this!
				D3D11_SUBRESOURCE_DATA texResData;
				texResData.pSysMem = &t.bitmap[0];
				// This defines the distance pitch between rows
				// So it is width* bytePerPixel as we need in bytes!
				texResData.SysMemPitch = t.width * t.bytepp;
				texResData.SysMemSlicePitch = 0;

				// Create texture
				HRESULT hr;
				//ID3D11Texture2D *pTexture = nullptr;
				ID3D11Texture2D *pTexture = NULL;
				//hr = pd3dDevice->CreateTexture2D(&desc, &texResData, &pTexture);
				hr = pd3dDevice->CreateTexture2D(&desc, &texResData, &pTexture);
				if (FAILED(hr)) {
					OMLOGE("Dx11GpuTexturePreparationLibrary - Failed to create Texture2D! (err:%d)", hr);
					return;
				}

				// Try to create the shader resource view out of this texture...
				D3D11_SHADER_RESOURCE_VIEW_DESC SRVDesc;
				memset(&SRVDesc, 0, sizeof(SRVDesc));
				SRVDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
				SRVDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
				SRVDesc.Texture2D.MipLevels = 1;
				ID3D11ShaderResourceView* textureView;
				hr = pd3dDevice->CreateShaderResourceView(pTexture, &SRVDesc, &textureView);
				if (FAILED(hr)) {
					OMLOGE("Dx11GpuTexturePreparationLibrary - Cannot create shader resource view for texture! (err:%d)", hr);
					return;
				}

				// Set the handle to hold the pointer.
				t.handle = reinterpret_cast<uintptr_t>(textureView);

				// Release texture. If I understand it well enough, we only need the view from now on...
				// this idea is stolen from the code at: https://msdn.microsoft.com/en-us/library/windows/desktop/ff476904%28v=vs.85%29.aspx
				// (Title of link: How to initialize texture from a file)
				if (pTexture != nullptr) {
					pTexture->Release();
				}

				OMLOGD("Dx11GpuTexturePreparationLibrary - Loaded %d bytes to the GPU!", (int)t.bitmap.size());
				OMLOGD("Dx11GpuTexturePreparationLibrary - Texture(%d) loaded!", t.handle);
			}
		}

		/** Unload the texture data bound to the given handle (glDeleteTexture()) */
		void Dx11GpuTexturePreparationLibrary::unloadFromGPU(ObjMaster::Texture &t) const {
			// In case of DirectX, we need a p3dDevice for operating on
			// Sadly the API is not really designed this way so we use this
			// approach of having a static class-variable that users set
			// before any usage...
			if (Dx11GpuTexturePreparationLibrary::pd3dDevice == nullptr) {
				OMLOGE("Dx11GpuTexturePreparationLibrary - Not using any provided ID3DDevice - doing a NO-OP!!!");
				return;
			}

			// Only do anything if the handle is a valid pointer!
			if (reinterpret_cast<ID3D11Texture2D*>(t.handle) != nullptr) {
				OMLOGD("Dx11GpuTexturePreparationLibrary - Releasing texture (%d)!", t.handle);
				// Delete the texture for the given handle
				// The handle is a pointer to the texture resource
				// in this DirectX version! Because of this, we can
				// release the texture like this!
				ID3D11ShaderResourceView *handleAsPtr = (reinterpret_cast<ID3D11ShaderResourceView*>(t.handle));
				// Rem.: The latter is for systems where nullptr != 0
				//  - in the first run the handle is initialized to plain zero!
				if (handleAsPtr != nullptr &&handleAsPtr != 0) {
					handleAsPtr->Release();
				}
				OMLOGD("Dx11GpuTexturePreparationLibrary - Released texture (%d)!", t.handle);

				// The nullptr means that there is no texture!
				// Better use this for compatibility than 0...
				// Rem.: Directly casting the nullptr is not enabled...
				ID3D11Texture2D* nuller = nullptr;
				t.handle = reinterpret_cast<uintptr_t>(nuller);
			}
		}

		// Need to call this so that we can use it in texture operations
		// and keep the original objmaster architecture... This is not 
		// the cleanest, but otherwise refactor would be needed!
		void Dx11GpuTexturePreparationLibrary::use3dDevice(ID3D11Device *p3dDevice) {
			pd3dDevice = p3dDevice;
		}
}
