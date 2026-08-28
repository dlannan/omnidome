
include(${CMAKE_SOURCE_DIR}/cmake/plugin.cmake)

set(SOURCES
        ScreenCapture.h
        ScreenCapture.cpp
        ScreenCaptureWidget.h
        ScreenCaptureWidget.cpp
	ScreenCapturePreview.h
        ScreenCapturePreview.cpp
	ScreenCaptureBackend.h
	ScreenCaptureBackend.cpp
        ScreenCapture.qrc
)

omni_plugin(plugin_input_ScreenCapture "${SOURCES}")

