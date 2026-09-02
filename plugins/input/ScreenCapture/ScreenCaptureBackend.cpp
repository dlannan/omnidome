


#include "ScreenCaptureBackend.h"

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")

#include <string.h>
#include <stdint.h>
#include <intrin.h>
#include <QOpenGLFunctions>

#define Assert(cond) do { if (!(cond)) __debugbreak(); } while (0)
#define AssertHR(hr) Assert(SUCCEEDED(hr))

namespace omni {
	namespace input {

		Microsoft::WRL::ComPtr<ID3D11Device> ScreenCaptureBackend::d3d11Device_;
		Microsoft::WRL::ComPtr<ID3D11DeviceContext> ScreenCaptureBackend::d3d11Context_;

		ComPtr<IDXGIDevice> ScreenCaptureBackend::dxgiDevice;
		HANDLE ScreenCaptureBackend::dxDevice_ = nullptr;;
		ComPtr<IDXGIAdapter> ScreenCaptureBackend::adapter_ = nullptr;
		bool ScreenCaptureBackend::bDeviceInit = false;

		std::map<int, MonitorCapture> ScreenCaptureBackend::monitors_;

		bool ScreenCaptureBackend::LoadDXInterop()
		{

			wglDXOpenDeviceNV =
				reinterpret_cast<PFNWGLDXOPENDEVICENVPROC>(
					wglGetProcAddress("wglDXOpenDeviceNV"));
			wglDXOpenDeviceNV =
				reinterpret_cast<PFNWGLDXOPENDEVICENVPROC>(
					wglGetProcAddress("wglDXOpenDeviceNV"));

			wglDXRegisterObjectNV =
				reinterpret_cast<PFNWGLDXREGISTEROBJECTNVPROC>(
					wglGetProcAddress("wglDXRegisterObjectNV"));

			wglDXLockObjectsNV =
				reinterpret_cast<PFNWGLDXLOCKOBJECTSNVPROC>(
					wglGetProcAddress("wglDXLockObjectsNV"));

			wglDXUnlockObjectsNV =
				reinterpret_cast<PFNWGLDXUNLOCKOBJECTSNVPROC>(
					wglGetProcAddress("wglDXUnlockObjectsNV"));

			
			wglDXUnregisterObjectNV =
				reinterpret_cast<PFNWGLDXUNREGISTEROBJECTNVPROC>(
					wglGetProcAddress("wglDXUnregisterObjectNV"));

			wglDXCloseDeviceNV =
				reinterpret_cast<PFNWGLDXCLOSEDEVICENVPROC>(
					wglGetProcAddress("wglDXCloseDeviceNV"));

			return wglDXOpenDeviceNV &&
				wglDXRegisterObjectNV &&
				wglDXLockObjectsNV &&
				wglDXUnlockObjectsNV;
		}

		QSize ScreenCaptureBackend::size()
		{
			if (monitors_.size() == 0) return QSize(1, 1);
			if (monitors_.find(monitorTarget_) == monitors_.end()) return QSize(1, 1);
			auto mon = monitors_[monitorTarget_];
			return QSize(mon.width, mon.height);
		}

		unsigned int ScreenCaptureBackend::textureId()
		{
			if (monitors_.size() == 0) return 0;
			if (monitors_.find(monitorTarget_) == monitors_.end()) return 0;
			return monitors_[monitorTarget_].glTexture; 
		}

		int ScreenCaptureBackend::getMonitorCount() const
		{
			if (!adapter_)
				return 0;

			int count = 0;
			ComPtr<IDXGIOutput> output;
			while (adapter_->EnumOutputs(count, &output) != DXGI_ERROR_NOT_FOUND)
			{
				output.Reset();
				++count;
			}
			return count;
		}

		// This internally looks at the monitor list, and gets the next available monitor
		//  id from the vector.
		int ScreenCaptureBackend::GetFreeMonitorId()
		{
			if (!bDeviceInit) CreateDevice();
			int monitorCount = getMonitorCount();
			if (monitorCount > 0) {
				if (monitors_.size() == 0) return 0;
				// Check if each monitor is assigned
				if (monitors_.size() > 0) {
					std::vector<int> used_monitors;
					for (int i = 0; i < monitorCount; i++) {
						if (monitors_.find(i) == monitors_.end())
							return i;
					}
				}
			}
			return -1;
		}

		void ScreenCaptureBackend::RemoveMonitor(int monid)
		{
			auto monitor = monitors_.find(monid);
			if(monitor != monitors_.end()) {
				//DestroyCapture(monitor->second.id);
				monitors_.erase(monitor);
			}
		}

		void ScreenCaptureBackend::CreateDevice()
		{
			// Dont create more than one device!!
			if (dxDevice_ != nullptr) return;

			HRESULT hr;

			ComPtr<ID3D11Device> device;
			ComPtr<ID3D11DeviceContext> context;

			hr = D3D11CreateDevice(
				nullptr,
				D3D_DRIVER_TYPE_HARDWARE,
				nullptr,
				0,
				nullptr,
				0,
				D3D11_SDK_VERSION,
				&device,
				nullptr,
				&context);

			AssertHR(hr);

			hr = device.As(&dxgiDevice);
			AssertHR(hr);

			hr = dxgiDevice->GetAdapter(&adapter_);
			AssertHR(hr);

			d3d11Device_ = device;
			d3d11Context_ = context;

			bDeviceInit = true;
		}

		void ScreenCaptureBackend::CreateDesktopCapture(int id)
		{
			MonitorCapture capture;
			capture.id = id;

			HRESULT hr;

			ComPtr<IDXGIOutput> output;
			hr = adapter_->EnumOutputs(id, &output);
			AssertHR(hr);

			ComPtr<IDXGIOutput1> output1;
			hr = output.As(&output1);
			AssertHR(hr);

			hr = output1->DuplicateOutput( d3d11Device_.Get(), &capture.duplication);
			AssertHR(hr);

			DXGI_OUTDUPL_DESC desc{};
			capture.duplication->GetDesc(&desc);

			capture.width = desc.ModeDesc.Width;
			capture.height = desc.ModeDesc.Height;

			D3D11_TEXTURE2D_DESC texDesc{};
			texDesc.Width = capture.width;
			texDesc.Height = capture.height;
			texDesc.MipLevels = 1;
			texDesc.ArraySize = 1;
			texDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
			texDesc.SampleDesc.Count = 1;
			texDesc.Usage = D3D11_USAGE_DEFAULT;

			hr = d3d11Device_->CreateTexture2D(	&texDesc, nullptr, &capture.texture);
			AssertHR(hr);

			// dxDevice_ can be shared by all monitor textures.
			LoadDXInterop();
			if (!dxDevice_)
			{
				dxDevice_ = wglDXOpenDeviceNV(d3d11Device_.Get());
				Assert(dxDevice_);
			}

			glGenTextures(1, &capture.glTexture);

			capture.dxTexture = wglDXRegisterObjectNV(
				dxDevice_,
				capture.texture.Get(),
				capture.glTexture,
				GL_TEXTURE_2D,
				WGL_ACCESS_READ_ONLY_NV);

			Assert(capture.dxTexture);

			BOOL ok = wglDXLockObjectsNV(dxDevice_,	1, &capture.dxTexture);
			Assert(ok);

			monitors_[id] = (std::move(capture));
			monitorTarget_ = id;
		}

		void ScreenCaptureBackend::CaptureDesktopFrame()
		{
			if (monitors_.size() == 0) return;
			if (monitors_.find(monitorTarget_) == monitors_.end()) return;

			auto& capture = monitors_[monitorTarget_];

			DXGI_OUTDUPL_FRAME_INFO info{};
			ComPtr<IDXGIResource> resource;

			HRESULT hr = capture.duplication->AcquireNextFrame(
				0, &info, &resource);

			if (FAILED(hr))
				return;

			ComPtr<ID3D11Texture2D> resourceTexture;
			hr = resource.As(&resourceTexture);
			AssertHR(hr);

			BOOL ok = wglDXUnlockObjectsNV(
				dxDevice_,
				1,
				&capture.dxTexture);

			Assert(ok);

			d3d11Context_->CopyResource(
				capture.texture.Get(),
				resourceTexture.Get());

			d3d11Context_->Flush();

			ok = wglDXLockObjectsNV(
				dxDevice_,
				1,
				&capture.dxTexture);

			Assert(ok);

			capture.duplication->ReleaseFrame();
		}



		ScreenCaptureBackend::ScreenCaptureBackend()
		{
			captureInitialized_ = false;
		}

		ScreenCaptureBackend::~ScreenCaptureBackend()
		{
		}

		void ScreenCaptureBackend::DestroyCapture(int monitor)
		{
			// All GL/WGL interop operations need the appropriate
			// OpenGL context current here.
			if (monitors_.find(monitor) == monitors_.end()) return;

			auto& capture = monitors_[monitor];
			// If the WGL object is currently locked, unlock it first.
			if (capture.dxTexture)
			{
				BOOL ok = wglDXUnlockObjectsNV(
					dxDevice_,
					1,
					&capture.dxTexture);

				Assert(ok);

				ok = wglDXUnregisterObjectNV(
					dxDevice_,
					capture.dxTexture);

				Assert(ok);

				capture.dxTexture = nullptr;
			}

			if (capture.glTexture)
			{
				glDeleteTextures(1, &capture.glTexture);
				capture.glTexture = 0;
			}

			// Release the D3D texture and duplication.
			capture.texture.Reset();
			capture.duplication.Reset();

			capture.width = 0;
			capture.height = 0;
		}


		bool ScreenCaptureBackend::Init(int monitorid = 0)
		{
			auto* context = QOpenGLContext::currentContext();
			if (!context)
			{
				// No current GL context on this thread.
				return false;
			}
			if (!bDeviceInit)
				CreateDevice();
				
			// If monitor changes then detroy and create again
//			if (monitorid != monitorTarget_)
//				DestroyCapture();

			CreateDesktopCapture(monitorid);
			captureInitialized_ = true;
			return true;
		}

		void ScreenCaptureBackend::CaptureTexture()
		{
			if(captureInitialized_)
				CaptureDesktopFrame();
		}

		bool ScreenCaptureBackend::Capture(QImage& image)
		{
			HDC desktopDC = GetDC(nullptr);
			if (!desktopDC)
				return false;

			const int width = GetSystemMetrics(SM_CXSCREEN);
			const int height = GetSystemMetrics(SM_CYSCREEN);

			HDC memoryDC = CreateCompatibleDC(desktopDC);
			if (!memoryDC)
			{
				ReleaseDC(nullptr, desktopDC);
				return false;
			}

			BITMAPINFO bitmapInfo = {};
			bitmapInfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
			bitmapInfo.bmiHeader.biWidth = width;
			bitmapInfo.bmiHeader.biHeight = -height; // top-down
			bitmapInfo.bmiHeader.biPlanes = 1;
			bitmapInfo.bmiHeader.biBitCount = 32;
			bitmapInfo.bmiHeader.biCompression = BI_RGB;

			void* bits = nullptr;

			HBITMAP bitmap = CreateDIBSection(
				desktopDC,
				&bitmapInfo,
				DIB_RGB_COLORS,
				&bits,
				nullptr,
				0);

			if (!bitmap)
			{
				DeleteDC(memoryDC);
				ReleaseDC(nullptr, desktopDC);
				return false;
			}

			HGDIOBJ oldBitmap = SelectObject(memoryDC, bitmap);

			const bool copied = BitBlt(
				memoryDC,
				0, 0,
				width, height,
				desktopDC,
				0, 0,
				SRCCOPY | CAPTUREBLT);

			if (!copied)
			{
				SelectObject(memoryDC, oldBitmap);
				DeleteObject(bitmap);
				DeleteDC(memoryDC);
				ReleaseDC(nullptr, desktopDC);
				return false;
			}

			// Copy the captured pixels into an owning QImage.
			QImage captured(
				static_cast<uchar*>(bits),
				width,
				height,
				width * 4,
				QImage::Format_ARGB32);

			image = captured.copy();

			SelectObject(memoryDC, oldBitmap);
			DeleteObject(bitmap);
			DeleteDC(memoryDC);
			ReleaseDC(nullptr, desktopDC);

			return !image.isNull();
		}
	}
}
