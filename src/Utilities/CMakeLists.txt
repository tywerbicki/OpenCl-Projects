add_library(Utilities STATIC
                context.cpp
                debug.cpp
                device.cpp
                platform.cpp
                program.cpp
                required.h
                settings.h)

target_link_libraries(Utilities PRIVATE
                          Defaults
                          OpenCL::OpenCL)

target_link_libraries(Tests PRIVATE
                          Utilities)