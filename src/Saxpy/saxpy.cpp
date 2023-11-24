#include "debug.h"
#include "saxpy.h"

#include <algorithm>


cl_int saxpy::DeviceExecute(const cl_command_queue          saxpyQueue,
                            const cl_kernel                 saxpyKernel,
                            const std::span<const cl_event> eventsToWaitOn,
                            cl_event&                       saxpyComplete)
{
    cl_int       result                         = CL_SUCCESS;
    cl_device_id executingDevice                = nullptr;
    size_t       preferredWorkGroupSizeMultiple = 0;

    result = clGetCommandQueueInfo(saxpyQueue,
                                   CL_QUEUE_DEVICE,
                                   sizeof(executingDevice),
                                   &executingDevice,
                                   nullptr);

    OPENCL_RETURN_ON_ERROR(result);

    result = clGetKernelWorkGroupInfo(saxpyKernel,
                                      executingDevice,
                                      CL_KERNEL_PREFERRED_WORK_GROUP_SIZE_MULTIPLE,
                                      sizeof(preferredWorkGroupSizeMultiple),
                                      &preferredWorkGroupSizeMultiple,
                                      nullptr);

    OPENCL_RETURN_ON_ERROR(result);

    // The multipliers should be tuned per-hardware and per-saxpy-length.
    const size_t localWorkSize  = preferredWorkGroupSizeMultiple * 2;
    const size_t globalWorkSize = localWorkSize * 16;

    result = clEnqueueNDRangeKernel(saxpyQueue,
                                    saxpyKernel,
                                    1,
                                    nullptr,
                                    &globalWorkSize,
                                    &localWorkSize,
                                    static_cast<cl_uint>(eventsToWaitOn.size()),
                                    eventsToWaitOn.data(),
                                    &saxpyComplete);

    OPENCL_PRINT_ON_ERROR(result);
    return result;
}


void saxpy::HostExecute(const float        a,
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