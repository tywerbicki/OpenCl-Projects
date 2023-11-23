#ifndef UTILITIES_PLATFORM_TYPES_H
#define UTILITIES_PLATFORM_TYPES_H

#include <CL/cl.h>

#include <optional>
#include <span>
#include <vector>


namespace platform
{
    using UniSelectionStrategy = cl_int(std::span<const cl_platform_id> platforms,
                                        std::optional<cl_platform_id>&  selectedPlatform,
                                        std::vector<cl_device_id>&      selectedDevices);

    using PolySelectionStrategy = cl_int(std::span<const cl_platform_id>         platforms,
                                         std::vector<cl_platform_id>&            selectedPlatforms,
                                         std::vector<std::vector<cl_device_id>>& selectedDevices);
}


#endif // UTILITIES_PLATFORM_TYPES_H