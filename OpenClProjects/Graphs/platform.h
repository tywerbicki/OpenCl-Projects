#pragma once

#include <CL/cl.h>

#include <array>
#include <iostream>
#include <vector>

#include "debug.h"
#include "device.h"


namespace platform
{


cl_int GetAvailablePlatforms(std::vector<cl_platform_id>& platformIds)
{
    cl_int  result     = CL_SUCCESS;
    cl_uint nPlatforms = 0;

    // Query the number of available platforms.
    result = clGetPlatformIDs(0, nullptr, &nPlatforms);
    OPENCL_RETURN_ON_ERROR(result);

    platformIds.resize(nPlatforms);

    // Query the available platform ids.
    result = clGetPlatformIDs(nPlatforms, platformIds.data(), nullptr);
    OPENCL_PRINT_ON_ERROR(result);

    return result;
}


template<typename ParamType>
cl_int QueryPlatformParamValue(
    const cl_platform_id    platformId,
    const cl_platform_info  paramName,
    std::vector<ParamType>& paramValue)
{
    cl_int result                = CL_SUCCESS;
    size_t paramValueSizeInBytes = 0;

    // Query size of parameter value.
    result = clGetPlatformInfo(
        platformId,
        paramName,
        0,
        nullptr,
        &paramValueSizeInBytes
    );
    OPENCL_RETURN_ON_ERROR(result);

    paramValue.resize(paramValueSizeInBytes / sizeof(ParamType));

    // Query parameter value.
    result = clGetPlatformInfo(
        platformId,
        paramName,
        paramValueSizeInBytes,
        paramValue.data(),
        nullptr
    );
    OPENCL_PRINT_ON_ERROR(result);

    return result;
}


cl_int DisplayPlatformInfo(const cl_platform_id platformId)
{
    const std::array<cl_platform_info, 5> paramNames
    {
        CL_PLATFORM_PROFILE,
        CL_PLATFORM_VERSION,
        CL_PLATFORM_NAME,
        CL_PLATFORM_VENDOR,
        CL_PLATFORM_EXTENSIONS
    };

    const std::array<const char*, paramNames.size()> paramNameStrs
    {
        "CL_PLATFORM_PROFILE",
        "CL_PLATFORM_VERSION",
        "CL_PLATFORM_NAME",
        "CL_PLATFORM_VENDOR",
        "CL_PLATFORM_EXTENSIONS"
    };

    cl_int result = CL_SUCCESS;

    for (size_t i = 0; i < paramNames.size(); i++)
    {
        std::vector<char> paramValue;

        result = QueryPlatformParamValue(
            platformId,
            paramNames[i],
            paramValue
        );
        OPENCL_RETURN_ON_ERROR(result);

        std::cout << paramNameStrs[i] << ": ";
        for (const auto k : paramValue) std::cout << k;
        std::cout << "\n";
    }

    device::DisplayAllDevices(platformId);

    return result;
}


} // platform