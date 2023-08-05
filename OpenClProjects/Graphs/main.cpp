#define CL_TARGET_OPENCL_VERSION 300

#include <CL/cl.h>

#include <algorithm>
#include <array>
#include <iostream>
#include <memory>
#include <stdlib.h>
#include <string>
#include <unordered_set>
#include <vector>


inline void _OpenClCheckError(
    const cl_int       result,
    const char* const  fileName,
    const unsigned int lineNumber)
{
    if (result != CL_SUCCESS)
    {
        std::cerr << "\nOPENCL_ERROR\n";

        std::cerr << "File: " << fileName   << "\n";
        std::cerr << "Line: " << lineNumber << "\n";

        std::cerr << "Error code: " << result << "\n";

        std::exit(EXIT_FAILURE);
    }
}

#define OPENCL_CHECK_ERROR(result) _OpenClCheckError(result, __FILE__, __LINE__)


cl_int DisplayGeneralDeviceInfo(const cl_device_id deviceId)
{
    const std::array<cl_device_info, 9> paramNames{
        CL_DEVICE_TYPE,
        CL_DEVICE_MAX_COMPUTE_UNITS,
        CL_DEVICE_MAX_WORK_ITEM_DIMENSIONS,
        CL_DEVICE_PREFERRED_VECTOR_WIDTH_INT,
        CL_DEVICE_PREFERRED_VECTOR_WIDTH_LONG,
        CL_DEVICE_PREFERRED_VECTOR_WIDTH_FLOAT,
        CL_DEVICE_PREFERRED_VECTOR_WIDTH_DOUBLE,
        CL_DEVICE_GLOBAL_MEM_SIZE,
        CL_DEVICE_HOST_UNIFIED_MEMORY
    };

    const std::array<const char*, 9> paramNameStrs{
        "CL_DEVICE_TYPE",
        "CL_DEVICE_MAX_COMPUTE_UNITS",
        "CL_DEVICE_MAX_WORK_ITEM_DIMENSIONS",
        "CL_DEVICE_PREFERRED_VECTOR_WIDTH_INT",
        "CL_DEVICE_PREFERRED_VECTOR_WIDTH_LONG",
        "CL_DEVICE_PREFERRED_VECTOR_WIDTH_FLOAT",
        "CL_DEVICE_PREFERRED_VECTOR_WIDTH_DOUBLE",
        "CL_DEVICE_GLOBAL_MEM_SIZE",
        "CL_DEVICE_HOST_UNIFIED_MEMORY"
    };

    cl_int result = CL_SUCCESS;

    for (size_t i = 0; i < paramNames.size(); i++)
    {
        cl_ulong paramValue = 0;

        // Query parameter value.
        result = clGetDeviceInfo(
            deviceId,
            paramNames[i],
            sizeof(paramValue),
            &paramValue,
            nullptr
        );

        OPENCL_CHECK_ERROR(result);

        std::cout << paramNameStrs[i] << ": " << paramValue << "\n";
    }

    return result;
}


cl_int DisplayAllDevices(const cl_platform_id platformId)
{
    cl_int  result   = CL_SUCCESS;
    cl_uint nDevices = 0;

    // Query number of available devices.
    result = clGetDeviceIDs(
        platformId,
        CL_DEVICE_TYPE_ALL,
        0,
        nullptr,
        &nDevices
    );

    OPENCL_CHECK_ERROR(result);

    std::vector<cl_device_id> deviceIds{nDevices};

    // Query ids of all available devices.
    result = clGetDeviceIDs(
        platformId,
        CL_DEVICE_TYPE_ALL,
        nDevices,
        deviceIds.data(),
        nullptr
    );

    OPENCL_CHECK_ERROR(result);

    std::for_each(
        deviceIds.cbegin(),
        deviceIds.cend(),
        DisplayGeneralDeviceInfo
    );

    return result;
}


cl_int DisplayPlatformInfo(const cl_platform_id platformId)
{
    const std::array<cl_platform_info, 5> paramNames{
        CL_PLATFORM_PROFILE,
        CL_PLATFORM_VERSION,
        CL_PLATFORM_NAME,
        CL_PLATFORM_VENDOR,
        CL_PLATFORM_EXTENSIONS
    };

    const std::array<const char*, 5> paramNameStrs{
        "CL_PLATFORM_PROFILE",
        "CL_PLATFORM_VERSION",
        "CL_PLATFORM_NAME",
        "CL_PLATFORM_VENDOR",
        "CL_PLATFORM_EXTENSIONS"
    };

    cl_int result = CL_SUCCESS;

    for (size_t i = 0; i < paramNames.size(); i++)
    {
        size_t paramValueSizeInBytes = 0;

        // Query size of parameter value.
        result = clGetPlatformInfo(
            platformId,
            paramNames[i],
            0,
            nullptr,
            &paramValueSizeInBytes
        );

        OPENCL_CHECK_ERROR(result);

        std::unique_ptr<char[]> paramValue{ new char[paramValueSizeInBytes] };

        // Query parameter value.
        result = clGetPlatformInfo(
            platformId,
            paramNames[i],
            paramValueSizeInBytes,
            paramValue.get(),
            nullptr
        );

        OPENCL_CHECK_ERROR(result);

        std::cout << paramNameStrs[i] << ": " << paramValue << "\n";
    }

    DisplayAllDevices(platformId);

    return result;
}


std::vector<cl_platform_id> GetAvailablePlatforms()
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

    return platformIds;
}


int main()
{
    const auto platformIds = GetAvailablePlatforms();

    #ifdef _DEBUG
    
    std::for_each(
        platformIds.cbegin(),
        platformIds.cend(),
        DisplayPlatformInfo
    );

    #endif // _DEBUG

    // Specify application-required platform information.
    const std::string requiredProfile = "FULL_PROFILE";

    const std::unordered_set<std::string> requiredPlatformNames = {
        "NVIDIA CUDA",
        "AMD ROCM"
    };


}