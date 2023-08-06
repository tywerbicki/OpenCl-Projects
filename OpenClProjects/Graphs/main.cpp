#define CL_TARGET_OPENCL_VERSION 300

#include <CL/cl.h>

#include <algorithm>
#include <iostream>
#include <stdlib.h>
#include <unordered_set>

#include "debug.h"
#include "platform.h"
#include "required.h"


int main()
{
    cl_int result = CL_SUCCESS;

    const auto platformIds = platform::GetAvailablePlatforms(&result);

    #ifdef _DEBUG
    
    std::for_each(
        platformIds.cbegin(),
        platformIds.cend(),
        platform::DisplayPlatformInfo
    );

    #endif // _DEBUG

    // Choose the first available platform that meets our requirements.
    cl_platform_id platform = nullptr;

    for (const auto platformId : platformIds)
    {
        // Query profile of candidate platform.
        const auto platformProfile = platform::QueryPlatformParamValue<char>(
            platformId,
            CL_PLATFORM_PROFILE,
            &result
        );

        OPENCL_CHECK_ERROR(result);

        if (required::PlatformProfile == platformProfile.get())
        {
            platform = platformId;
            break;
        }
    }

    if (platform == nullptr)
    {
        std::cout << "No suitable OpenCL platform was detected." << std::endl;
        return EXIT_SUCCESS;
    }


}