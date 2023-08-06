#define CL_TARGET_OPENCL_VERSION 300

#include <CL/cl.h>

#include <algorithm>
#include <unordered_set>

#include "debug.h"
#include "platform.h"
#include "require.h"


int main()
{
    const auto platformIds = platform::GetAvailablePlatforms();

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

    }

    std::cout << require::requiredPlatformProfile << std::endl;

    


}