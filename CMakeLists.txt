cmake_minimum_required(VERSION 3.30)
project(Keyboard_Trainer)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 COMPONENTS
        Core
        Gui
        Widgets
        REQUIRED)

find_package(CURL REQUIRED)
include_directories(${CURL_INCLUDE_DIRS})

add_executable(Keyboard_Trainer main.cpp
        "AI json-request/api.cpp"
        "AI json-request/api.h"
        src/languages.h
        window.cpp
        window.h
)

target_link_libraries(Keyboard_Trainer
        Qt::Core Qt::Gui Qt::Widgets
        ${CURL_LIBRARIES}
)

