#include "debug.h"
#include "saxpy.h"

#include <algorithm>
#include <array>


namespace
{
    struct KernelArg
    {
        cl_uint     index;
        size_t      sizeInBytes;
        const void* pValue;
    };
}


cl_int saxpy::EnqueueKernel(const float                     a,
                            const cl_mem                    xDevice,
                            const cl_mem                    yDevice,
                            const cl_mem                    zDevice,
                            const size_t                    len,
                            const cl_command_queue          saxpyQueue,
                            const cl_kernel                 saxpyKernel,
                            const std::span<const cl_event> eventsToWaitOn,
                            cl_event&                       saxpyComplete)
{
    cl_int result = CL_SUCCESS;

    const std::array<KernelArg, 5> kernelArgs =
    { {
            { .index = 0, .sizeInBytes = sizeof(a),       .pValue = &a       },
            { .index = 1, .sizeInBytes = sizeof(xDevice), .pValue = &xDevice },
            { .index = 2, .sizeInBytes = sizeof(yDevice), .pValue = &yDevice },
            { .index = 3, .sizeInBytes = sizeof(zDevice), .pValue = &zDevice },
            { .index = 4, .sizeInBytes = sizeof(len),     .pValue = &len     }
    } };

    for (const KernelArg& arg : kernelArgs)
    {
        result = clSetKernelArg(saxpyKernel,
                                arg.index,
                                arg.sizeInBytes,
                                arg.pValue);

        OPENCL_RETURN_ON_ERROR(result);
    }

    cl_device_id executingDevice = nullptr;
    size_t       workGroupSize   = 0;

    result = clGetCommandQueueInfo(saxpyQueue,
                                   CL_QUEUE_DEVICE,
                                   sizeof(executingDevice),
                                   &executingDevice,
                                   nullptr);

    OPENCL_RETURN_ON_ERROR(result);

    result = clGetKernelWorkGroupInfo(saxpyKernel,
                                      executingDevice,
                                      CL_KERNEL_WORK_GROUP_SIZE,
                                      sizeof(workGroupSize),
                                      &workGroupSize,
                                      nullptr);

    OPENCL_RETURN_ON_ERROR(result);

    const size_t numWorkGroups  = (len + workGroupSize - 1) / workGroupSize;
    const size_t globalWorkSize = numWorkGroups * workGroupSize;

    result = clEnqueueNDRangeKernel(saxpyQueue,
                                    saxpyKernel,
                                    1,
                                    nullptr,
                                    &globalWorkSize,
                                    &workGroupSize,
                                    static_cast<cl_uint>(eventsToWaitOn.size()),
                                    eventsToWaitOn.data(),
                                    &saxpyComplete);

    OPENCL_PRINT_ON_ERROR(result);
    return result;
}


void saxpy::HostExec(const float        a,
                     const float* const pXHost,
                     const float* const pYHost,
                     float* const       pZHost,
                     const size_t       len)
{
    std::transform(pXHost, pXHost + len,
                   pYHost,
                   pZHost,
                   [=](float x, float y) { return (a * x) + y; });
}
