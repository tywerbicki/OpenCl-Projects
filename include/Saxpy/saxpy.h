#ifndef SAXPY_SAXPY_H
#define SAXPY_SAXPY_H

#include <CL/cl.h>

#include <span>


namespace saxpy
{
    [[nodiscard]] cl_int DeviceExecute(cl_command_queue          saxpyQueue,
                                       cl_kernel                 saxpyKernel,
                                       std::span<const cl_event> eventsToWaitOn,
                                       cl_event&                 saxpyComplete);

    void HostExecute(float        a,
                     const float* pXHost,
                     const float* pYHost,
                     float*       pZHost,
                     size_t       len);
}


#endif // SAXPY_SAXPY_H