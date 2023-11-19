cmake_minimum_required(VERSION 3.23)

set(CMAKE_GENERATOR_PLATFORM x64)
set(CMAKE_CONFIGURATION_TYPES Debug Release)

set(CMAKE_SUPPRESS_REGENERATION ON)
set(CMAKE_SKIP_INSTALL_RULES ON)

project(OpenClProjects LANGUAGES CXX)

find_package(OpenCL REQUIRED)

add_library(Defaults INTERFACE)

target_compile_features(Defaults INTERFACE
                            cxx_std_20)

target_compile_definitions(Defaults INTERFACE
                               $<$<CONFIG:Debug>:_DEBUG>
                               $<$<CONFIG:Release>:_RELEASE>)

if (MSVC)
    target_compile_definitions(Defaults INTERFACE
                                   CL_TARGET_OPENCL_VERSION=300
                                   $<$<CONFIG:Debug>:_ALLOW_RTCc_IN_STL>)

    target_compile_options(Defaults INTERFACE
                               /W4 /EHsc
                               $<$<CONFIG:Debug>:/MDd /Od /Ob0 /RTC1 /RTCc>
                               $<$<CONFIG:Release>:/MD /O2 /Ob2 /Oi>)

    # 1. Always ignore the static C++ runtime libraries.
    # 2. Conditionally ignore the incorrect DLL runtime library.
    target_link_options(Defaults INTERFACE
                            /NODEFAULTLIB:libcmt.lib /NODEFAULTLIB:libcmtd.lib
                            $<$<CONFIG:Debug>:/NODEFAULTLIB:msvcrt.lib>
                            $<$<CONFIG:Release>:/NODEFAULTLIB:msvcrtd.lib>)
else()
    # TODO: fully support more compilers and linkers.
    target_compile_options(Defaults INTERFACE
                               -Wall -Wextra)
endif()

include(FetchContent)
include(GoogleTest)

FetchContent_Declare(googletest
                     GIT_REPOSITORY https://github.com/google/googletest.git
                     GIT_TAG b10fad38c4026a29ea6561ab15fc4818170d1c10
                     EXCLUDE_FROM_ALL)

FetchContent_MakeAvailable(googletest)

enable_testing()

add_executable(Tests)

target_link_libraries(Tests PRIVATE
                          Defaults
                          GTest::gtest_main
                          OpenCL::OpenCL)

add_subdirectory(src)
add_subdirectory(include)

gtest_discover_tests(Tests)