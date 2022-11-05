cmake_minimum_required(VERSION 3.15)
project(EventBus)
set(CMAKE_CXX_STANDARD 14)

include(conandeps.cmake)

find_package(GTest REQUIRED)
include_directories(${GTest_INCLUDE_DIRS})

add_library(EventBus SHARED src/EventBus.cpp src/EventHandler.cpp)

#target_link_libraries(test ${GTest_LIBS})