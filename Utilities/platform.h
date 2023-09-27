#pragma once

#include <CL/cl.h>

#include <span>
#include <string>
#include <vector>


namespace platform
{

    using UniSelectionStrategy = cl_int(std::span<const cl_platform_id> platforms,
                                        cl_platform_id&                 selectedPlatform,
                                        std::vector<cl_device_id>&      selectedDevices);

    using PolySelectionStrategy = cl_int(std::span<const cl_platform_id>        platforms,
                                         std::vector<cl_platform_id>&           selectedPlatforms,
                                         std::vector<std::vector<cl_device_id>> selectedDevices);


    [[nodiscard]] cl_int GetAllAvailable(std::vector<cl_platform_id>& platforms);

    [[nodiscard]] cl_int QueryParamValue(cl_platform_id   platform,
                                         cl_platform_info paramName,
                                         std::string&     paramValue);

    [[nodiscard]] cl_int DisplayInfo(cl_platform_id platform);

    [[nodiscard]] cl_int IsConformant(cl_platform_id platform,
                                      bool&          isConformant);

    [[nodiscard]] cl_int GetAllConformant(std::vector<cl_platform_id>& conformantPlatforms);

    [[nodiscard]] cl_int MostGpus(std::span<const cl_platform_id> platforms,
                                  cl_platform_id&                 selectedPlatform,
                                  std::vector<cl_device_id>&      selectedDevices);

}