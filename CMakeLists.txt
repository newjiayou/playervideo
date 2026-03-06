cmake_minimum_required(VERSION 3.16)

project(videoplayer VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ----------------- 添加 FFmpeg 配置 -----------------
# 1. 定义 FFmpeg 路径（相对于项目根目录）
set(FFMPEG_DIR "${CMAKE_SOURCE_DIR}/3rdparty/ffmpeg")

# 2. 包含 FFmpeg 头文件
include_directories(${FFMPEG_DIR}/include)

# 3. 指定 FFmpeg 库文件目录
link_directories(${FFMPEG_DIR}/lib)
# ---------------------------------------------------
find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets Multimedia)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets Multimedia)

set(PROJECT_SOURCES
        main.cpp
        mainwindow.cpp
        mainwindow.h
        mainwindow.ui
        packetqueue.h      # 不要删掉这个
        packetqueue.cpp    # 不要删掉这个
        videowidget.h      # 【新增】必须加这一行
        videowidget.cpp    # 【新增】必须加这一行
        resources.qrc
)
if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(videoplayer
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
    )
# Define target properties for Android with Qt 6 as:
#    set_property(TARGET videoplayer APPEND PROPERTY QT_ANDROID_PACKAGE_SOURCE_DIR
#                 ${CMAKE_CURRENT_SOURCE_DIR}/android)
# For more information, see https://doc.qt.io/qt-6/qt-add-executable.html#target-creation
else()
    if(ANDROID)
        add_library(videoplayer SHARED
            ${PROJECT_SOURCES}
        )
# Define properties for Android with Qt 5 after find_package() calls as:
#    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
    else()
        add_executable(videoplayer
            ${PROJECT_SOURCES}
        )
    endif()
endif()
target_include_directories(videoplayer PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
# 链接 Qt 库
target_link_libraries(videoplayer PRIVATE Qt${QT_VERSION_MAJOR}::Widgets Qt${QT_VERSION_MAJOR}::Multimedia)

# ----------------- 链接 FFmpeg 库 -----------------
target_link_libraries(videoplayer PRIVATE
    avcodec
    avformat
    avutil
    swscale
    swresample
)

# 在 Windows 上自动复制 FFmpeg DLL
if(WIN32)
    file(GLOB FFMPEG_DLLS "${FFMPEG_DIR}/bin/*.dll")
    if(FFMPEG_DLLS)
        add_custom_command(TARGET videoplayer POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
            ${FFMPEG_DLLS}
            $<TARGET_FILE_DIR:videoplayer>
            COMMENT "正在自动复制 FFmpeg DLLs 到输出目录..."
        )
    endif()
endif()

# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
if(${QT_VERSION} VERSION_LESS 6.1.0)
  set(BUNDLE_ID_OPTION MACOSX_BUNDLE_GUI_IDENTIFIER com.example.videoplayer)
endif()
set_target_properties(videoplayer PROPERTIES
    ${BUNDLE_ID_OPTION}
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

include(GNUInstallDirs)
install(TARGETS videoplayer
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(videoplayer)
endif()
