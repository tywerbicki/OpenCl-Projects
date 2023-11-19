#ifndef SAXPY_SAXPY_H
#define SAXPY_SAXPY_H

#include <CL/cl.h>

#include <span>


namespace saxpy
{
    [[nodiscard]] cl_int HostExecute(cl_context             context,
                                     cl_command_queue       commandQueue,
                                     cl_kernel              saxpyKernel,
                                     float                  a,
                                     std::span<const float> x,
                                     std::span<float>       y,
                                     cl_event&              event);

    [[nodiscard]] cl_int DeviceExecute(cl_command_queue          commandQueue,
                                       cl_kernel                 saxpyKernel,
                                       std::span<const cl_event> eventsToWaitOn,
                                       cl_event&                 saxpyEvent);
}


#endif // SAXPY_SAXPY_H