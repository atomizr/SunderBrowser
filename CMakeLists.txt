cmake_minimum_required(VERSION 3.16)
project(SimpleBrowser)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Widgets WebEngineWidgets)

qt_standard_project_setup()

add_executable(SimpleBrowser
    main.cpp
    BrowserWindow.h
    BrowserWindow.cpp
    SettingsDialog.h
    SettingsDialog.cpp
)

target_link_libraries(SimpleBrowser Qt6::Core Qt6::Widgets Qt6::WebEngineWidgets)

find_program(WINDEPLOYQT windeployqt PATHS ${CMAKE_PREFIX_PATH}/bin REQUIRED)
add_custom_command(TARGET SimpleBrowser POST_BUILD
    COMMAND ${WINDEPLOYQT} --release --no-compiler-runtime $<TARGET_FILE:SimpleBrowser>
    COMMENT "Копирование зависимостей Qt"
)