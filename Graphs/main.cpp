#define CL_TARGET_OPENCL_VERSION 300

#include <CL/cl.h>

#include "build.h"
#include "context.h"
#include "debug.h"
#include "platform.h"
#include "program.h"


int main()
{
    cl_int     result  = CL_SUCCESS;
    cl_context context = nullptr;

    result = context::Create(platform::MostGpus, context);
    OPENCL_RETURN_ON_ERROR(result);

    if (!context)
    {
        MSG_STD_OUT("No OpenCL context was created.");
        return CL_SUCCESS;
    }

    cl_program program = nullptr;

    result = program::Build(
        context,
        build::graphs::clBinaryRoot,
        build::graphs::clBinaryName,
        build::graphs::clSourceRoot,
        build::graphs::options,
        program
    );
    OPENCL_RETURN_ON_ERROR(result);
}