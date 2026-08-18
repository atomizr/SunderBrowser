cmake_minimum_required(VERSION 3.16)
project(SunderBrowser)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Widgets WebEngineWidgets)

# -------------------------------------------------------------------
# Подключение spdlog (header-only) 
# Предполагаем, что заголовки лежат в 3rdparty/spdlog/include/
# Если структура иная — отредактируйте путь ниже.
# -------------------------------------------------------------------
set(SPDLOG_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/spdlog)

# Попробуем наиболее вероятный путь
if(EXISTS ${SPDLOG_ROOT}/include/spdlog/spdlog.h)
    set(SPDLOG_INCLUDE_DIR ${SPDLOG_ROOT}/include)
elseif(EXISTS ${SPDLOG_ROOT}/spdlog/spdlog.h)
    set(SPDLOG_INCLUDE_DIR ${SPDLOG_ROOT})
elseif(EXISTS ${SPDLOG_ROOT}/spdlog.h)
    set(SPDLOG_INCLUDE_DIR ${SPDLOG_ROOT})
else()
    message(FATAL_ERROR "Cannot find spdlog headers. Expected in ${SPDLOG_ROOT}/include/spdlog/spdlog.h or similar.")
endif()

message(STATUS "spdlog include dir: ${SPDLOG_INCLUDE_DIR}")
include_directories(${SPDLOG_INCLUDE_DIR})
# -------------------------------------------------------------------

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
    src/utils/Logger.h
    src/utils/Logger.cpp

    app.rc
)

add_custom_command(TARGET SunderBrowser POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_CURRENT_SOURCE_DIR}/assets
        $<TARGET_FILE_DIR:SunderBrowser>/assets
    COMMENT "Copying assets to output directory"
)

target_include_directories(SunderBrowser PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_link_libraries(SunderBrowser Qt6::Core Qt6::Widgets Qt6::WebEngineWidgets)

find_program(WINDEPLOYQT windeployqt PATHS ${CMAKE_PREFIX_PATH}/bin REQUIRED)
add_custom_command(TARGET SunderBrowser POST_BUILD
    COMMAND ${WINDEPLOYQT} --release --no-compiler-runtime $<TARGET_FILE:SunderBrowser>
    COMMENT "Copying Qt dependencies"
)
