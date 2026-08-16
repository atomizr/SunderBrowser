cmake_minimum_required(VERSION 3.16)
project(SunderBrowser)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)   # уже должно быть
set(CMAKE_AUTOUIC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Widgets WebEngineWidgets)

qt_standard_project_setup()

# Добавляем ресурсы и RC-файл
add_executable(SunderBrowser WIN32
    main.cpp
    BrowserWindow.h
    BrowserWindow.cpp
    SettingsDialog.h
    SettingsDialog.cpp
    resources.qrc          # файл ресурсов
    app.rc                 # RC-файл с версией и иконкой
)

target_link_libraries(SunderBrowser Qt6::Core Qt6::Widgets Qt6::WebEngineWidgets)

# Копирование зависимостей Qt
find_program(WINDEPLOYQT windeployqt PATHS ${CMAKE_PREFIX_PATH}/bin REQUIRED)
add_custom_command(TARGET SunderBrowser POST_BUILD
    COMMAND ${WINDEPLOYQT} --release --no-compiler-runtime $<TARGET_FILE:SunderBrowser>
    COMMENT "Копирование зависимостей Qt"
)
