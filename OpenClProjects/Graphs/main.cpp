#define CL_TARGET_OPENCL_VERSION 300

#include <CL/cl.h>

#include <iostream>
#include <stdlib.h>
#include <string>
#include <unordered_set>
#include <vector>

#include "debug.h"
#include "platform.h"
#include "required.h"


int main()
{
    cl_int                      result      = CL_SUCCESS;
    std::vector<cl_platform_id> platformIds = {};
   
    result = platform::GetAvailablePlatforms(platformIds);
    OPENCL_RETURN_ON_ERROR(result);

    #ifdef _DEBUG

    for (const auto platformId : platformIds)
    {
        result = platform::DisplayPlatformInfo(platformId);
        OPENCL_RETURN_ON_ERROR(result);
    }

    #endif // _DEBUG

    // Choose the first available platform that meets our requirements.
    cl_platform_id    platform        = nullptr;
    std::vector<char> platformProfile = {};
    std::vector<char> platformName    = {};

    for (const auto platformId : platformIds)
    {
        // Query profile of candidate platform.
        result = platform::QueryPlatformParamValue(
            platformId,
            CL_PLATFORM_PROFILE,
            platformProfile
        );
        OPENCL_RETURN_ON_ERROR(result);

        // Query name of candidate platform.
        result = platform::QueryPlatformParamValue(
            platformId,
            CL_PLATFORM_NAME,
            platformName
        );
        OPENCL_RETURN_ON_ERROR(result);

        if (
            (platformProfile.data() == required::PlatformProfile) &&
            (
                platformName.data() == required::PlatformNameNvda ||
                platformName.data() == required::PlatformNameAmd
            )
        )
        {
            platform = platformId;
            break;
        }
    }

    if (platform == nullptr)
    {
        std::cout << "No suitable OpenCL platform was detected." << std::endl;
        return CL_SUCCESS;
    }


}