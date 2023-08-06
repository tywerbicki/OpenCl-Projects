#pragma once

#include <CL/cl.h>

#include <array>
#include <memory>
#include <iostream>
#include <vector>

#include "debug.h"
#include "device.h"


namespace platform
{


std::vector<cl_platform_id> GetAvailablePlatforms(cl_int* const result_out)
{
    cl_int  result     = CL_SUCCESS;
    cl_uint nPlatforms = 0;

    // Query the number of available platforms.
    result = clGetPlatformIDs(0, nullptr, &nPlatforms);

    OPENCL_CHECK_ERROR(result);

    std::vector<cl_platform_id> platformIds{nPlatforms};

    // Query the available platform ids.
    result = clGetPlatformIDs(nPlatforms, platformIds.data(), nullptr);

    OPENCL_CHECK_ERROR(result);
    *result_out = result;

    return platformIds;
}


template<typename ParamType>
std::unique_ptr<ParamType> QueryPlatformParamValue(
    const cl_platform_id   platformId,
    const cl_platform_info paramName,
    cl_int* const          result_out)
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

    OPENCL_CHECK_ERROR(result);

    std::unique_ptr<ParamType> paramValue{ new ParamType[paramValueSizeInBytes / sizeof(ParamType)] };

    // Query parameter value.
    result = clGetPlatformInfo(
        platformId,
        paramName,
        paramValueSizeInBytes,
        paramValue.get(),
        nullptr
    );

    OPENCL_CHECK_ERROR(result);
    *result_out = result;

    return paramValue;
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
        const auto paramValue = QueryPlatformParamValue<char>(
            platformId,
            paramNames[i],
            &result
        );

        OPENCL_CHECK_ERROR(result);

        std::cout << paramNameStrs[i] << ": " << paramValue << "\n";
    }

    device::DisplayAllDevices(platformId);

    return result;
}


} // platform