#define CL_TARGET_OPENCL_VERSION 300

#include <CL/cl.h>

#include <iostream>

#include "debug.h"
#include "device.h"
#include "platform.h"


int main()
{
    cl_int         result   = CL_SUCCESS;
    cl_platform_id platform = nullptr;
    cl_device_id   device   = nullptr;

    result = platform::AcquirePlatform(platform);
    OPENCL_RETURN_ON_ERROR(result);
    
    if (platform == nullptr)
    {
        std::cout << "No suitable OpenCL platform was detected." << std::endl;
        return CL_SUCCESS;
    }


}