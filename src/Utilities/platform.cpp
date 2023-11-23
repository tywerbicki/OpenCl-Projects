#include "debug.h"
#include "device.h"
#include "platform.h"
#include "required.h"
#include "settings.h"

#include <array>
#include <sstream>


cl_int platform::GetAllAvailable(std::vector<cl_platform_id>& platforms)
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


cl_int platform::QueryParamValue(const cl_platform_id   platform,
                                 const cl_platform_info paramName,
                                 std::string&           paramValue)
{
    cl_int result                = CL_SUCCESS;
    size_t paramValueSizeInBytes = 0;

    result = clGetPlatformInfo(platform,
                               paramName,
                               0,
                               nullptr,
                               &paramValueSizeInBytes);

    OPENCL_RETURN_ON_ERROR(result);

    // 1. `paramValueSizeInBytes` includes the NULL terminator, hence "-1".
    // 2. This is legal to do from C++11 onwards.
    paramValue.resize(paramValueSizeInBytes - 1);

    result = clGetPlatformInfo(platform,
                               paramName,
                               paramValueSizeInBytes,
                               paramValue.data(),
                               nullptr);

    OPENCL_PRINT_ON_ERROR(result);

    return result;
}


cl_int platform::DisplayInfo(const cl_platform_id platform)
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
        result = QueryParamValue(platform,
                                 paramNames[i],
                                 paramValue);

        OPENCL_RETURN_ON_ERROR(result);

        msgStdOut << paramNameStrs[i] << ": " << paramValue << "\n";
    }

    MSG_STD_OUT(msgStdOut.view());

    return result;
}


cl_int platform::IsConformant(const cl_platform_id platform,
                              bool&                isConformant)
{
    cl_int      result          = CL_SUCCESS;
    std::string platformProfile = {};

    result = platform::QueryParamValue(platform,
                                       CL_PLATFORM_PROFILE,
                                       platformProfile);

    OPENCL_RETURN_ON_ERROR(result);

    if (platformProfile != required::PlatformProfile)
    {
        isConformant = false;
        return result;
    }

    isConformant = true;
    return result;
}


cl_int platform::GetAllConformant(std::vector<cl_platform_id>& conformantPlatforms)
{
    cl_int                      result             = CL_SUCCESS;
    std::vector<cl_platform_id> availablePlatforms = {};

    result = platform::GetAllAvailable(availablePlatforms);
    OPENCL_RETURN_ON_ERROR(result);

    conformantPlatforms.reserve(availablePlatforms.size());

    for (const cl_platform_id platform : availablePlatforms)
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


cl_int platform::MostGpus(const std::span<const cl_platform_id> platforms,
                          std::optional<cl_platform_id>&        selectedPlatform,
                          std::vector<cl_device_id>&            selectedDevices)
{
    selectedPlatform.reset();
    selectedDevices.resize(0);

    cl_int result = CL_SUCCESS;

    for (const cl_platform_id platform : platforms)
    {
        std::vector<cl_device_id> availableDevices = {};
        std::vector<cl_device_id> conformantGpus   = {};

        result = device::GetAllAvailable(platform, availableDevices);
        OPENCL_RETURN_ON_ERROR(result);

        for (const cl_device_id device : availableDevices)
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