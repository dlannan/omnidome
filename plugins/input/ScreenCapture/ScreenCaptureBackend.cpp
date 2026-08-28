

#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <d3d11.h>
#include <dxgi1_2.h>

#include <GL/gl.h>
//#include <GL/glcorearb.h> // get from https://www.khronos.org/registry/OpenGL/api/GL/glcorearb.h
//#include <GL/wglext.h>    // get from https://www.khronos.org/registry/OpenGL/api/GL/wglext.h
// also put platform.h in KHR folder from https://www.khronos.org/registry/EGL/api/KHR/khrplatform.h

#include <string.h>
#include <stdint.h>
#include <intrin.h>

#include "ScreenCaptureBackend.h"

#include <windows.h>

ScreenCaptureBackend::ScreenCaptureBackend()
{
}

ScreenCaptureBackend::~ScreenCaptureBackend()
{
}

bool ScreenCaptureBackend::capture(QImage& image)
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

