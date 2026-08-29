


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
		Microsoft::WRL::ComPtr<IDXGIOutputDuplication> ScreenCaptureBackend::dxgiDuplication_;
		Microsoft::WRL::ComPtr<ID3D11Texture2D> ScreenCaptureBackend::d3d11Texture_;


		HANDLE ScreenCaptureBackend::dxDevice_ = nullptr;
		HANDLE ScreenCaptureBackend::dxTexture_ = nullptr;


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

			return wglDXOpenDeviceNV &&
				wglDXRegisterObjectNV &&
				wglDXLockObjectsNV &&
				wglDXUnlockObjectsNV;
		}

		void ScreenCaptureBackend::CreateDesktopCapture()
		{
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

			ComPtr<IDXGIDevice> dxgiDevice;
			hr = device.As(&dxgiDevice);
			AssertHR(hr);

			ComPtr<IDXGIAdapter> adapter;
			hr = dxgiDevice->GetAdapter(&adapter);
			AssertHR(hr);

			ComPtr<IDXGIOutput> output;
			hr = adapter->EnumOutputs(0, &output);
			AssertHR(hr);

			ComPtr<IDXGIOutput1> output1;
			hr = output.As(&output1);
			AssertHR(hr);

			hr = output1->DuplicateOutput(device.Get(), &dxgiDuplication_);

			AssertHR(hr);

			DXGI_OUTDUPL_DESC desc{};
			dxgiDuplication_->GetDesc(&desc);

			captureWidth = desc.ModeDesc.Width;
			captureHeight = desc.ModeDesc.Height;
			printf("Desktop Width: %d   Height: %d\n", captureWidth, captureHeight);

			D3D11_TEXTURE2D_DESC texDesc{};
			texDesc.Width = captureWidth;
			texDesc.Height = captureHeight;
			texDesc.MipLevels = 1;
			texDesc.ArraySize = 1;
			texDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
			texDesc.SampleDesc.Count = 1;
			texDesc.Usage = D3D11_USAGE_DEFAULT;

			ComPtr<ID3D11Texture2D> texture;

			hr = device->CreateTexture2D(
				&texDesc,
				nullptr,
				&texture);

			AssertHR(hr);

			LoadDXInterop();
			dxDevice_ = wglDXOpenDeviceNV(device.Get());
			Assert(dxDevice_);

			glGenTextures(1, &openglTexture);

			dxTexture_ = wglDXRegisterObjectNV(
				dxDevice_,
				texture.Get(),
				openglTexture,
				GL_TEXTURE_2D,
				WGL_ACCESS_READ_ONLY_NV);

			Assert(dxTexture_);

			BOOL ok = wglDXLockObjectsNV(
				dxDevice_,
				1,
				&dxTexture_);

			Assert(ok);

			// Keep these alive if they are needed later.
			d3d11Device_ = device;
			d3d11Context_ = context;
			d3d11Texture_ = texture;

			bCapturing = true;
		}


		void ScreenCaptureBackend::CaptureDesktopFrame()
		{
			DXGI_OUTDUPL_FRAME_INFO info{};
			Microsoft::WRL::ComPtr<IDXGIResource> resource;

			HRESULT hr = dxgiDuplication_->AcquireNextFrame(
				0,
				&info,
				&resource);

			if (FAILED(hr))
			{
				// DXGI_ERROR_WAIT_TIMEOUT is normal when no new frame is available.
				return;
			}

			Microsoft::WRL::ComPtr<ID3D11Texture2D> resourceTexture;

			hr = resource.As(&resourceTexture);
			AssertHR(hr);

			BOOL ok = wglDXUnlockObjectsNV(
				dxDevice_,
				1,
				&dxTexture_);

			Assert(ok);

			d3d11Context_->CopyResource(
				d3d11Texture_.Get(),
				resourceTexture.Get());

			d3d11Context_->Flush();

			ok = wglDXLockObjectsNV(
				dxDevice_,
				1,
				&dxTexture_);

			Assert(ok);

			dxgiDuplication_->ReleaseFrame();
		}

		ScreenCaptureBackend::ScreenCaptureBackend()
		{
			bCapturing = false;
			captureInitialized_ = false;
		}

		ScreenCaptureBackend::~ScreenCaptureBackend()
		{
		}

		bool ScreenCaptureBackend::Init()
		{
			auto* context = QOpenGLContext::currentContext();

			if (!context)
			{
				// No current GL context on this thread.
				return false;
			}
			CreateDesktopCapture();
			captureInitialized_ = true;
			return true;
		}

		void ScreenCaptureBackend::CaptureTexture()
		{
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
