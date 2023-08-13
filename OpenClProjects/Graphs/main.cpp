#define CL_TARGET_OPENCL_VERSION 300

#include <CL/cl.h>

#include <iostream>
#include <vector>

#include "context.h"
#include "debug.h"
#include "program.h"


int main()
{
    cl_int                  result   = CL_SUCCESS;
    std::vector<cl_context> contexts = {};

    result = context::GetAllAvailable(contexts);
    OPENCL_RETURN_ON_ERROR(result);

    if (contexts.size() == 0)
    {
        std::cout << "No suitable OpenCL device was detected." << std::endl;
        return CL_SUCCESS;
    }

    for (const auto context : contexts)
    {
        result = program::Build(context);
        OPENCL_RETURN_ON_ERROR(result);
    }
}