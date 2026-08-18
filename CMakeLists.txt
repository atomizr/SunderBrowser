cmake_minimum_required(VERSION 3.16)
project(SunderBrowser)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Widgets WebEngineWidgets)

qt_standard_project_setup()

add_executable(SunderBrowser
    src/main.cpp

    src/controllers/BrowserController.h
    src/controllers/BrowserController.cpp

    src/models/SettingsModel.h
    src/models/SettingsModel.cpp

    src/services/SearchService.h
    src/services/SearchService.cpp
    src/services/BackgroundService.h
    src/services/BackgroundService.cpp
    src/services/IconService.h
    src/services/IconService.cpp

    src/ui/MainWindow.h
    src/ui/MainWindow.cpp
    src/ui/BrowserTab.h
    src/ui/BrowserTab.cpp
    src/ui/SettingsDialog.h
    src/ui/SettingsDialog.cpp

    src/utils/UrlUtils.h
    src/utils/UrlUtils.cpp

    app.rc
)

# Копирование папки assets рядом с исполняемым файлом
add_custom_command(TARGET SunderBrowser POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_CURRENT_SOURCE_DIR}/assets
        $<TARGET_FILE_DIR:SunderBrowser>/assets
    COMMENT "Копирование assets в выходную директорию"
)

target_include_directories(SunderBrowser PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)

target_link_libraries(SunderBrowser Qt6::Core Qt6::Widgets Qt6::WebEngineWidgets)

# Копирование зависимостей Qt
find_program(WINDEPLOYQT windeployqt PATHS ${CMAKE_PREFIX_PATH}/bin REQUIRED)
add_custom_command(TARGET SunderBrowser POST_BUILD
    COMMAND ${WINDEPLOYQT} --release --no-compiler-runtime $<TARGET_FILE:SunderBrowser>
    COMMENT "Копирование зависимостей Qt"
)
