#define CL_TARGET_OPENCL_VERSION 300

#include <CL/cl.h>

#include <iostream>
#include <vector>

#include "build.h"
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

    std::vector<cl_program> programs(contexts.size());

    for (size_t i = 0; i < contexts.size(); i++)
    {
        result = program::Build(
            contexts[i],
            build::graphs::clBinaryRoot,
            build::graphs::clBinaryName,
            build::graphs::clSourceRoot,
            build::graphs::clSourceNames,
            build::graphs::options,
            programs[i]
        );
        OPENCL_RETURN_ON_ERROR(result);
    }
}