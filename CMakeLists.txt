cmake_minimum_required(VERSION 3.24)
set(CMAKE_PROJECT_TOP_LEVEL_INCLUDES "${CMAKE_SOURCE_DIR}/conan_provider.cmake")
project(EventBus)
set(CMAKE_CXX_STANDARD 14)

if(WIN32)
    SET(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)
endif (WIN32)
find_package(GTest REQUIRED)
include_directories(${GTest_INCLUDE_DIRS})

add_library(EventBus SHARED src/EventBus.cpp src/EventHandler.cpp)
add_executable(test test/EventBusTest.cpp)
target_include_directories(EventBus PUBLIC include)

target_link_libraries(test EventBus GTest::gtest GTest::gtest_main)
if (UNIX)
    target_link_libraries(EventBus pthread)
endif()

install(TARGETS EventBus)
install(DIRECTORY include/EventBus DESTINATION include)