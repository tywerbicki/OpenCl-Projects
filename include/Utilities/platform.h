#ifndef UTILITIES_PLATFORM_H
#define UTILITIES_PLATFORM_H

#include <CL/cl.h>

#include <optional>
#include <span>
#include <string>
#include <vector>


namespace platform
{
    [[nodiscard]] cl_int GetAllAvailable(std::vector<cl_platform_id>& platforms);

    [[nodiscard]] cl_int QueryParamValue(cl_platform_id   platform,
                                         cl_platform_info paramName,
                                         std::string&     paramValue);

    [[nodiscard]] cl_int DisplayInfo(cl_platform_id platform);

    [[nodiscard]] cl_int IsConformant(cl_platform_id platform,
                                      bool&          isConformant);

    [[nodiscard]] cl_int GetAllConformant(std::vector<cl_platform_id>& conformantPlatforms);

    [[nodiscard]] cl_int MostGpus(std::span<const cl_platform_id> platforms,
                                  std::optional<cl_platform_id>&  selectedPlatform,
                                  std::vector<cl_device_id>&      selectedDevices);
}


#endif // UTILITIES_PLATFORM_H