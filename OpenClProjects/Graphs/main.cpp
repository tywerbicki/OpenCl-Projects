#define CL_TARGET_OPENCL_VERSION 300

#include <CL/cl.h>

#include <iostream>
#include <vector>

#include "debug.h"
#include "exec.h"


int main()
{
    cl_int                           result    = CL_SUCCESS;
    std::vector<exec::ExecResources> resources = {};

    result = exec::GetResources(resources);
    OPENCL_RETURN_ON_ERROR(result);

    if (resources.size() == 0)
    {
        std::cout << "No suitable OpenCL device was detected." << std::endl;
        return CL_SUCCESS;
    }


}