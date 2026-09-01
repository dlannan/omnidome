
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
		};


		class ScreenCaptureBackend
		{
		public:
			ScreenCaptureBackend();
			~ScreenCaptureBackend();

			int getMonitorCount() const;

			bool IsDeviceInit() { return bDeviceInit; }
			bool IsInitialized() { return captureInitialized_; }
			void SetInitialized(bool init) { captureInitialized_ = init; }
			bool Init(int monitorid);
			bool Capture(QImage& image);

			void CaptureTexture();

			unsigned int textureId() { return monitors_[monitorTarget_].glTexture; }
			QSize size();

		private:

			void CreateDevice();
			void DestroyCapture(int monitor);
			bool LoadDXInterop();
			void CreateDesktopCapture(int id);
			void CaptureDesktopFrame();

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

			static std::vector<MonitorCapture> monitors_;
			int currentMonitor_ = 0;

			static HANDLE dxDevice_;
			
			int monitorTarget_ = 0;

			static bool bDeviceInit;
			bool captureInitialized_ = false;

			uint32_t captureWidth;
			uint32_t captureHeight;
		};
	}
}


#endif /* OMNI_INPUT_SCREENCAPTUREBACKEND_H_ */

