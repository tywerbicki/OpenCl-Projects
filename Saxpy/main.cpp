#include <CL/cl.h>

#include <array>
#include <functional>
#include <iostream>
#include <vector>

#include "build.h"
#include "context.h"
#include "debug.h"
#include "platform.h"
#include "program.h"
#include "saxpy.h"


int main()
{
    cl_int         result   = CL_SUCCESS;
    cl_platform_id platform = nullptr;
    cl_context     context  = nullptr;

    result = context::Create(platform::MostGpus, platform, context);
    OPENCL_RETURN_ON_ERROR(result);

    if (!context)
    {
        MSG_STD_OUT("No OpenCL context was created.");
        return CL_SUCCESS;
    }
    
    cl_program program = nullptr;
    result             = program::Build(context,
                                        std::cref(build::saxpy::clBinaryRoot),
                                        build::saxpy::clBinaryName,
                                        build::saxpy::clSourceRoot,
                                        build::saxpy::options,
                                        program);

    OPENCL_RETURN_ON_ERROR(result);

    result = clUnloadPlatformCompiler(platform);
    OPENCL_RETURN_ON_ERROR(result);

    std::array<cl_kernel, build::saxpy::kernelNames.size()> kernels = {};
    std::vector<cl_device_id>                               devices = {};

    result = program::CreateKernels(program,
                                    build::saxpy::kernelNames,
                                    kernels);

    OPENCL_RETURN_ON_ERROR(result);

    result = context::GetDevices(context, devices);
    OPENCL_RETURN_ON_ERROR(result);

    const cl_kernel    saxpyKernel = kernels[0];
    const cl_device_id saxpyDevice = devices[0];
    cl_event           saxpyEvent  = nullptr;

    const std::array<const cl_queue_properties, 3> queueProperties
    {
        CL_QUEUE_PROPERTIES,
        CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE,
        0
    };

    const cl_command_queue queue = clCreateCommandQueueWithProperties(context,
                                                                      saxpyDevice,
                                                                      queueProperties.data(),
                                                                      &result);

    OPENCL_RETURN_ON_ERROR(result);

    const float              a = 2.0;
    const std::vector<float> x = { 1.0, 2.0,  3.0  };
    std::vector<float>       y = { 5.5, 10.5, 15.5 };

    result = saxpy::HostExecute(context,
                                queue,
                                saxpyKernel,
                                a, x, y,
                                saxpyEvent);

    OPENCL_RETURN_ON_ERROR(result);

    result = clWaitForEvents(1, &saxpyEvent);
    OPENCL_RETURN_ON_ERROR(result);

    for (const auto i : y) std::cout << i << " ";
    std::cout << "\n";

    return CL_SUCCESS;
}