
#ifndef OMNI_INPUT_SCREENCAPTUREBACKEND_H_
#define OMNI_INPUT_SCREENCAPTUREBACKEND_H_

#include <QImage>
#include <QSize>

#define WIN32_LEAN_AND_MEAN
#define COBJMACROS

#include <windows.h>

#include <d3d11.h>
#include <dxgi1_2.h>
#include <wrl/client.h>

#include <GL/gl.h>
#include <GL/glcorearb.h> // get from https://www.khronos.org/registry/OpenGL/api/GL/glcorearb.h
#include <GL/wglext.h>    // get from https://www.khronos.org/registry/OpenGL/api/GL/wglext.h
// also put platform.h in KHR folder from https://www.khronos.org/registry/EGL/api/KHR/khrplatform.h

using Microsoft::WRL::ComPtr;

namespace omni {
	namespace input {

		struct MonitorCapture
		{
			Microsoft::WRL::ComPtr<IDXGIOutputDuplication> duplication;
			Microsoft::WRL::ComPtr<ID3D11Texture2D> texture;

			HANDLE dxTexture = nullptr;
			GLuint glTexture = 0;

			int width = 0;
			int height = 0;
			int	id = 0;
		};


		class ScreenCaptureBackend
		{
		public:
			ScreenCaptureBackend();
			~ScreenCaptureBackend();

			// Is the capture dxdevice been initialized? This only needs to happen once per app run.
			bool IsDeviceInit() { return bDeviceInit; }

			// Is the capture initialized (ie texture created and ready to capture)
			bool IsInitialized() { return captureInitialized_; }

			// Tell capture its initialized state (may be needed from plugin changes)
			void SetInitialized(bool init) { captureInitialized_ = init; }
			
			// Get a monitor that is available - returns -1 if it failed or no monitors left
			int GetFreeMonitorId();
			// Remove a monitor capture upon deactivation
			void RemoveMonitor(int monid);

			// Init the capture system - create device if needed and textures
			bool Init(int monitorid);
			// Capture a frame to the gltexture
			void CaptureTexture();

			// Get the gl texture for a monitor capture
			unsigned int textureId();

			// Get the size of the capture texture
			QSize size();

		private:

			// Get the number of available monitors
			int getMonitorCount() const;

			// Create the dxdevice and adaptor objects
			void CreateDevice();
			// Shutdown the capture layers and textures
			void DestroyCapture(int monitor);
			// Load in the NV extensions for capturing
			bool LoadDXInterop();
			// Capture the currently set monitor to a gltexture
			void CaptureDesktopFrame();
			//  Create the capture texture and runtime objects needed
			void CreateDesktopCapture(int id);

			// Old method of using GDI screen capture - deprecated. Kept in case of need
			bool Capture(QImage& image);


			// Capture calls used to for texture copying
			PFNWGLDXOPENDEVICENVPROC wglDXOpenDeviceNV = nullptr;
			PFNWGLDXREGISTEROBJECTNVPROC wglDXRegisterObjectNV = nullptr;
			PFNWGLDXLOCKOBJECTSNVPROC wglDXLockObjectsNV = nullptr;
			PFNWGLDXUNLOCKOBJECTSNVPROC wglDXUnlockObjectsNV = nullptr;
			PFNWGLDXUNREGISTEROBJECTNVPROC wglDXUnregisterObjectNV = nullptr;
			PFNWGLDXCLOSEDEVICENVPROC wglDXCloseDeviceNV = nullptr;


			static Microsoft::WRL::ComPtr<ID3D11Device> d3d11Device_;
			static Microsoft::WRL::ComPtr<ID3D11DeviceContext> d3d11Context_;
			
			static ComPtr<IDXGIDevice> dxgiDevice;
			static ComPtr<IDXGIAdapter> adapter_;

			static std::map<int, MonitorCapture> monitors_;
			int currentMonitor_ = 0;

			static HANDLE dxDevice_;
			
			// The monitor id to use when capturing
			int monitorTarget_ = 0;

			static bool bDeviceInit;
			bool captureInitialized_ = false;
		};
	}
}


#endif /* OMNI_INPUT_SCREENCAPTUREBACKEND_H_ */

