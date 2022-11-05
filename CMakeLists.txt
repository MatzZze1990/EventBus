cmake_minimum_required(VERSION 3.15)
project(EventBus)
set(CMAKE_CXX_STANDARD 14)
SET(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)
include(conandeps.cmake)

find_package(GTest REQUIRED)
include_directories(${GTest_INCLUDE_DIRS})
include_directories(${CMAKE_SOURCE_DIR})

add_library(EventBus SHARED src/EventBus.cpp src/EventHandler.cpp)
add_executable(test test/EventBusTest.cpp)

target_link_libraries(test EventBus ${GTest_LIBS})
if (UNIX)
    target_link_libraries(EventBus pthread)
endif()