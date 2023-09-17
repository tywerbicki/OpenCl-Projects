#pragma once

#include <CL/cl.h>

#include <array>
#include <iostream>
#include <string>
#include <vector>

#include "debug.h"
#include "required.h"
#include "settings.h"


namespace platform
{


using SelectionStrategy = cl_int(
    const std::vector<cl_platform_id>& platforms,
    cl_platform_id&                    selectedPlatform,
    std::vector<cl_device_id>&         selectedDevices);


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


cl_int GetAllConformant(std::vector<cl_platform_id>& conformantPlatforms)
{
    cl_int                      result             = CL_SUCCESS;
    std::vector<cl_platform_id> availablePlatforms = {};

    result = platform::GetAllAvailable(availablePlatforms);
    OPENCL_RETURN_ON_ERROR(result);

    conformantPlatforms.reserve(availablePlatforms.size());

    for (const auto platform : availablePlatforms)
    {
        if (settings::displayPlatformInfo)
        {
            result = platform::DisplayInfo(platform);
            OPENCL_RETURN_ON_ERROR(result);
        }

        bool platformIsConformant = false;

        result = platform::IsConformant(platform, platformIsConformant);
        OPENCL_RETURN_ON_ERROR(result);

        if (platformIsConformant)
        {
            conformantPlatforms.push_back(platform);
        }
    }

    return result;
}


cl_int MostGpus(
    const std::vector<cl_platform_id>& platforms,
    cl_platform_id&                    selectedPlatform,
    std::vector<cl_device_id>&         selectedDevices)
{
    cl_int result = CL_SUCCESS;

    for (const auto platform : platforms)
    {
        std::vector<cl_device_id> availableDevices = {};
        std::vector<cl_device_id> conformantGpus   = {};

        result = device::GetAllAvailable(platform, availableDevices);
        OPENCL_RETURN_ON_ERROR(result);

        for (const auto device : availableDevices)
        {
            if (settings::displayGeneralDeviceInfo)
            {
                result = device::DisplayGeneralInfo(device);
                OPENCL_RETURN_ON_ERROR(result);
            }

            bool deviceIsGpu = false;

            result = device::IsGpu(device, deviceIsGpu);
            OPENCL_RETURN_ON_ERROR(result);

            if (deviceIsGpu)
            {
                conformantGpus.push_back(device);
            }
        }

        if (conformantGpus.size() > selectedDevices.size())
        {
            selectedPlatform = platform;
            selectedDevices  = std::move(conformantGpus);
        }
    }

    return result;
}


} // platform