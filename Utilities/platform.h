#pragma once

#include <CL/cl.h>

#include <span>
#include <string>
#include <vector>


namespace platform
{

    using SelectionStrategy = cl_int(const std::span<cl_platform_id> platforms,
                                     cl_platform_id&                 selectedPlatform,
                                     std::vector<cl_device_id>&      selectedDevices);


    cl_int GetAllAvailable(std::vector<cl_platform_id>& platforms);

    template<typename ParamType>
    cl_int QueryParamValue(const cl_platform_id    platform,
                           const cl_platform_info  paramName,
                           std::vector<ParamType>& paramValue);

    cl_int QueryParamValue(const cl_platform_id   platform,
                           const cl_platform_info paramName,
                           std::string&           paramValue);

    cl_int DisplayInfo(const cl_platform_id platform);

    cl_int IsConformant(const cl_platform_id platform,
                        bool&                isConformant);

    cl_int GetAllConformant(std::vector<cl_platform_id>& conformantPlatforms);

    cl_int MostGpus(const std::span<cl_platform_id> platforms,
                    cl_platform_id&                 selectedPlatform,
                    std::vector<cl_device_id>&      selectedDevices);

}