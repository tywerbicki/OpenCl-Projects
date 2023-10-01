#pragma once

#include <CL/cl.h>

#include <span>


namespace saxpy
{

    cl_int HostExecute(cl_context             context,
                       cl_command_queue       commandQueue,
                       cl_kernel              saxpyKernel,
                       float                  a,
                       std::span<const float> x,
                       std::span<float>       y,
                       cl_event&              event);

    cl_int DeviceExecute(cl_command_queue          commandQueue,
                         cl_kernel                 saxpyKernel,
                         std::span<const cl_event> eventsToWaitOn,
                         cl_event&                 saxpyEvent);

}