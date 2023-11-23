#include "debug.h"
#include "saxpy.h"

#include <algorithm>


cl_int saxpy::DeviceExecute(const cl_command_queue          saxpyQueue,
                            const cl_kernel                 saxpyKernel,
                            const std::span<const cl_event> eventsToWaitOn,
                            cl_event&                       saxpyComplete)
{
    cl_int result = CL_SUCCESS;

    // TODO: select desirable work group sizes.
    const size_t globalWorkSize = 1024;
    const size_t localWorkSize  = 32;

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