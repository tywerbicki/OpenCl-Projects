add_library(Saxpy STATIC
                build.h
                saxpy.cpp)

target_link_libraries(Saxpy PRIVATE
                          Defaults
                          OpenCL::OpenCL
                          Utilities)

target_sources(Tests PRIVATE
                   saxpy.test.cpp)

target_link_libraries(Tests PRIVATE
                      Saxpy)