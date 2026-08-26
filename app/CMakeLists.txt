
find_package(Qt5 COMPONENTS Widgets REQUIRED)

set(SOURCES
    proj/ChannelCorrectionParameters.h
    proj/ChannelCorrectionParameters.cpp
    proj/ColorCorrectionGraph.h
    proj/ColorCorrectionGraph.cpp
    proj/TitleBar.h
    proj/TitleBar.cpp
    proj/Tuning.h
    proj/Tuning.cpp
    proj/TuningLayout.h
    proj/TuningLayout.cpp
    proj/TuningList.h
    proj/TuningList.cpp
    About.cpp
    About.h
    AboutGL.cpp
    AboutGL.h
    Application.h
    Application.cpp
    Blend.h
    Blend.cpp
    BrushPreview.h
    BrushPreview.cpp
    Canvas.h
    Canvas.cpp
    ColorCorrection.h
    ColorCorrection.cpp
    DockWidget.h
    DockWidget.cpp
    Export.h
    Export.cpp
    FullscreenTuningGLView.h
    FullscreenTuningGLView.cpp
    Input.h
    Input.cpp
    MainWindow.h
    MainWindow.cpp
    Mapping.h
    Mapping.cpp
    omnidome.cpp
    OutputPreview.h
    OutputPreview.cpp
    PluginList.h
    PluginList.cpp
    RecentSessions.h
    RecentSessions.cpp
    Scene.h
    Scene.cpp
    SceneGLView.h
    SceneGLView.cpp
    ScreenItem.h
    ScreenItem.cpp
    ScreenSetup.h
    ScreenSetup.cpp
    ToolBar.cpp
    ToolBar.h
    TuningGLView.h
    TuningGLView.cpp
    Warp.h
    Warp.cpp
    rc/omnidome.qrc
    )



add_executable(omnidome ${SOURCES})
target_sources(omnidome PRIVATE omnidome.rc)

target_include_directories(omnidome
    PUBLIC
        ../lib/include
        ../widgetlib/include
    PRIVATE
        .
)

target_link_libraries(omnidome
    PUBLIC
        Qt5::Widgets
        omni
        omniwidget
)



