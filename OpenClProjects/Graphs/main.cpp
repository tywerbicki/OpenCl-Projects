#include <CL/cl.h>

#include <array>
#include <iostream>
#include <memory>
#include <vector>


cl_uint DisplayPlatformInfo(const cl_platform_id platformId)
{
    std::array<cl_platform_info, 5> paramNames{
        CL_PLATFORM_PROFILE,
        CL_PLATFORM_VERSION,
        CL_PLATFORM_NAME,
        CL_PLATFORM_VENDOR,
        CL_PLATFORM_EXTENSIONS
    };

    std::array<const char*, 5> paramNameStrs{
        "CL_PLATFORM_PROFILE",
        "CL_PLATFORM_VERSION",
        "CL_PLATFORM_NAME",
        "CL_PLATFORM_VENDOR",
        "CL_PLATFORM_EXTENSIONS"
    };

    for (size_t i = 0; i < paramNames.size(); i++)
    {
        cl_uint result                = CL_SUCCESS;
        size_t  paramValueSizeInBytes = 0;

        // Query size of parameter value.
        result = clGetPlatformInfo(
            platformId,
            paramNames[i],
            0,
            nullptr,
            &paramValueSizeInBytes
        );

        if (result != CL_SUCCESS)
        {
            return result;
        }

        std::unique_ptr<char[]> paramValue{ new char[paramValueSizeInBytes] };

        // Query parameter value.
        result = clGetPlatformInfo(
            platformId,
            paramNames[i],
            paramValueSizeInBytes,
            paramValue.get(),
            nullptr
        );

        if (result != CL_SUCCESS)
        {
            return result;
        }

        std::cout << paramNameStrs[i] << ": " << paramValue << "\n";
    }
}


int main()
{
    cl_uint result     = CL_SUCCESS;
    cl_uint nPlatforms = 0;

    // Query the number of available platforms.
    result = clGetPlatformIDs(0, nullptr, &nPlatforms);

    if (result != CL_SUCCESS)
    {
        return result;
    }

    std::vector<cl_platform_id> platformIds{nPlatforms};

    // Query the available platform ids.
    result = clGetPlatformIDs(nPlatforms, platformIds.data(), nullptr);

    if (result != CL_SUCCESS)
    {
        return result;
    }

    for (size_t i = 0; i < nPlatforms; i++)
    {
        DisplayPlatformInfo(platformIds[i]);
    }
}