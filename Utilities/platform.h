#pragma once

#include <CL/cl.h>

#include <array>
#include <iostream>
#include <string>
#include <vector>

#include "debug.h"
#include "required.h"


namespace platform
{


cl_int GetAllAvailable(std::vector<cl_platform_id>& platforms)
{
    cl_int  result     = CL_SUCCESS;
    cl_uint nPlatforms = 0;

    result = clGetPlatformIDs(0, nullptr, &nPlatforms);
    OPENCL_RETURN_ON_ERROR(result);

    platforms.resize(nPlatforms);

    result = clGetPlatformIDs(nPlatforms, platforms.data(), nullptr);
    OPENCL_PRINT_ON_ERROR(result);

    return result;
}


template<typename ParamType>
cl_int QueryParamValue(
    const cl_platform_id    platform,
    const cl_platform_info  paramName,
    std::vector<ParamType>& paramValue)
{
    cl_int result                = CL_SUCCESS;
    size_t paramValueSizeInBytes = 0;

    result = clGetPlatformInfo(
        platform,
        paramName,
        0,
        nullptr,
        &paramValueSizeInBytes
    );
    OPENCL_RETURN_ON_ERROR(result);

    paramValue.resize(paramValueSizeInBytes / sizeof(ParamType));

    result = clGetPlatformInfo(
        platform,
        paramName,
        paramValueSizeInBytes,
        paramValue.data(),
        nullptr
    );
    OPENCL_PRINT_ON_ERROR(result);

    return result;
}


cl_int QueryParamValue(
    const cl_platform_id   platform,
    const cl_platform_info paramName,
    std::string&           paramValue)
{
    cl_int            result        = CL_SUCCESS;
    std::vector<char> paramValueVec = {};

    result = QueryParamValue(
        platform,
        paramName,
        paramValueVec
    );
    OPENCL_RETURN_ON_ERROR(result);

    paramValue.assign(
        paramValueVec.data(),
        paramValueVec.size() - 1
    );

    return result;
}


cl_int DisplayInfo(const cl_platform_id platform)
{
    const std::array<const cl_platform_info, 5> paramNames
    {
        CL_PLATFORM_PROFILE,
        CL_PLATFORM_VERSION,
        CL_PLATFORM_NAME,
        CL_PLATFORM_VENDOR,
        CL_PLATFORM_EXTENSIONS
    };

    const std::array<const std::string, paramNames.size()> paramNameStrs
    {
        "CL_PLATFORM_PROFILE",
        "CL_PLATFORM_VERSION",
        "CL_PLATFORM_NAME",
        "CL_PLATFORM_VENDOR",
        "CL_PLATFORM_EXTENSIONS"
    };

    cl_int             result     = CL_SUCCESS;
    std::string        paramValue = {};
    std::ostringstream msgStdOut  = {};

    for (size_t i = 0; i < paramNames.size(); i++)
    {
        result = QueryParamValue(
            platform,
            paramNames[i],
            paramValue
        );
        OPENCL_RETURN_ON_ERROR(result);

        msgStdOut << paramNameStrs[i] << ": " << paramValue << "\n";
    }

    MSG_STD_OUT(msgStdOut.view());

    return result;
}


cl_int IsConformant(const cl_platform_id platform, bool& isConformant)
{
    cl_int      result          = CL_SUCCESS;
    std::string platformProfile = {};
    std::string platformName    = {};

    result = platform::QueryParamValue(
        platform,
        CL_PLATFORM_PROFILE,
        platformProfile
    );
    OPENCL_RETURN_ON_ERROR(result);

    if (platformProfile != required::PlatformProfile)
    {
        isConformant = false;
        return result;
    }

    result = platform::QueryParamValue(
        platform,
        CL_PLATFORM_NAME,
        platformName
    );
    OPENCL_RETURN_ON_ERROR(result);

    if (platformName != required::PlatformNameNvda &&
        platformName != required::PlatformNameAmd)
    {
        isConformant = false;
        return result;
    }

    isConformant = true;
    return result;
}


} // platform