#include "debug.h"
#include "saxpy.h"

#include <array>
#include <cassert>
#include <stdint.h>
#include <tuple>


namespace
{
    enum saxpyDeviceBuffers : uint32_t
    {
        x = 0,
        y,
        count,
    };

    struct KernelArg
    {
        cl_uint     index;
        size_t      sizeInBytes;
        const void* value;
    };

    constexpr cl_uint saxpyNKernelArgs = 3;
}


cl_int saxpy::HostExecute(const cl_context             context,
                          const cl_command_queue       commandQueue,
                          const cl_kernel              saxpyKernel,
                          const float                  a,
                          const std::span<const float> x,
                          const std::span<float>       y,
                          cl_event&                    event)
{
    assert(x.size() == y.size());

    cl_int                                          result             = CL_SUCCESS;
    std::array<cl_event, saxpyDeviceBuffers::count> deviceBufferMaps   = {};
    std::array<cl_event, saxpyDeviceBuffers::count> hostToDeviceWrites = {};
    cl_event                                        deviceExecute      = nullptr;

    cl_mem xDeviceBuffer = clCreateBuffer(context,
                                          CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR | CL_MEM_HOST_WRITE_ONLY,
                                          x.size_bytes(),
                                          const_cast<float*>(x.data()),
                                          &result);

    OPENCL_RETURN_ON_ERROR(result);

    void* xDevBufMappedForWrite = clEnqueueMapBuffer(commandQueue,
                                                     xDeviceBuffer,
                                                     CL_FALSE,
                                                     CL_MAP_WRITE,
                                                     0,
                                                     x.size_bytes(),
                                                     0,
                                                     nullptr,
                                                     &deviceBufferMaps[saxpyDeviceBuffers::x],
                                                     &result);

    OPENCL_RETURN_ON_ERROR(result);

    cl_mem yDeviceBuffer = clCreateBuffer(context,
                                          CL_MEM_READ_WRITE | CL_MEM_USE_HOST_PTR,
                                          y.size_bytes(),
                                          y.data(),
                                          &result);

    OPENCL_RETURN_ON_ERROR(result);

    void* yDevBufMappedForWrite = clEnqueueMapBuffer(commandQueue,
                                                     yDeviceBuffer,
                                                     CL_FALSE,
                                                     CL_MAP_WRITE,
                                                     0,
                                                     y.size_bytes(),
                                                     0,
                                                     nullptr,
                                                     &deviceBufferMaps[saxpyDeviceBuffers::y],
                                                     &result);

    OPENCL_RETURN_ON_ERROR(result);

    result = clWaitForEvents(1, &deviceBufferMaps[saxpyDeviceBuffers::x]);
    OPENCL_RETURN_ON_ERROR(result);

    std::memcpy(xDevBufMappedForWrite, x.data(), x.size_bytes());

    result = clEnqueueUnmapMemObject(commandQueue,
                                     xDeviceBuffer,
                                     xDevBufMappedForWrite,
                                     0,
                                     nullptr,
                                     &hostToDeviceWrites[saxpyDeviceBuffers::x]);

    OPENCL_RETURN_ON_ERROR(result);

    result = clWaitForEvents(1, &deviceBufferMaps[saxpyDeviceBuffers::y]);
    OPENCL_RETURN_ON_ERROR(result);

    std::memcpy(yDevBufMappedForWrite, y.data(), y.size_bytes());

    result = clEnqueueUnmapMemObject(commandQueue,
                                     yDeviceBuffer,
                                     yDevBufMappedForWrite,
                                     0,
                                     nullptr,
                                     &hostToDeviceWrites[saxpyDeviceBuffers::y]);

    OPENCL_RETURN_ON_ERROR(result);

    const std::array<KernelArg, saxpyNKernelArgs> saxpyKernelArgs =
    { {
            { .index = 0, .sizeInBytes = sizeof(float) , .value = &a            },
            { .index = 1, .sizeInBytes = sizeof(cl_mem), .value = &xDeviceBuffer},
            { .index = 2, .sizeInBytes = sizeof(cl_mem), .value = &yDeviceBuffer}
    } };

    for (cl_uint i = 0; i < saxpyNKernelArgs; i++)
    {
        result = clSetKernelArg(saxpyKernel,
                                saxpyKernelArgs[i].index,
                                saxpyKernelArgs[i].sizeInBytes,
                                saxpyKernelArgs[i].value);

        OPENCL_RETURN_ON_ERROR(result);
    }

    result = DeviceExecute(commandQueue,
                           saxpyKernel,
                           hostToDeviceWrites,
                           deviceExecute);

    OPENCL_RETURN_ON_ERROR(result);

    void* yDevBufMappedForRead = clEnqueueMapBuffer(commandQueue,
                                                    yDeviceBuffer,
                                                    CL_TRUE,
                                                    CL_MAP_READ,
                                                    0,
                                                    y.size_bytes(),
                                                    1,
                                                    &deviceExecute,
                                                    nullptr,
                                                    &result);

    OPENCL_RETURN_ON_ERROR(result);

    std::memcpy(y.data(), yDevBufMappedForRead, y.size_bytes());

    result = clEnqueueUnmapMemObject(commandQueue,
                                     yDeviceBuffer,
                                     yDevBufMappedForRead,
                                     0,
                                     nullptr,
                                     &event);

    OPENCL_RETURN_ON_ERROR(result);

    result = clReleaseMemObject(xDeviceBuffer);
    OPENCL_RETURN_ON_ERROR(result);
    result = clReleaseMemObject(yDeviceBuffer);
    OPENCL_RETURN_ON_ERROR(result);

    result = clReleaseEvent(deviceBufferMaps[saxpyDeviceBuffers::x]);
    OPENCL_RETURN_ON_ERROR(result);
    result = clReleaseEvent(deviceBufferMaps[saxpyDeviceBuffers::y]);
    OPENCL_RETURN_ON_ERROR(result);
    result = clReleaseEvent(hostToDeviceWrites[saxpyDeviceBuffers::x]);
    OPENCL_RETURN_ON_ERROR(result);
    result = clReleaseEvent(hostToDeviceWrites[saxpyDeviceBuffers::y]);
    OPENCL_RETURN_ON_ERROR(result);
    result = clReleaseEvent(deviceExecute);
    OPENCL_PRINT_ON_ERROR(result);

    return result;
}


cl_int saxpy::DeviceExecute(const cl_command_queue          commandQueue,
                            const cl_kernel                 saxpyKernel,
                            const std::span<const cl_event> eventsToWaitOn,
                            cl_event&                       saxpyEvent)
{
    cl_int result = CL_SUCCESS;

    // TODO: select desirable work group sizes.
    const size_t globalWorkSize = 1024;
    const size_t localWorkSize  = 32;

    result = clEnqueueNDRangeKernel(commandQueue,
                                    saxpyKernel,
                                    1,
                                    nullptr,
                                    &globalWorkSize,
                                    &localWorkSize,
                                    static_cast<cl_uint>(eventsToWaitOn.size()),
                                    eventsToWaitOn.data(),
                                    &saxpyEvent);

    OPENCL_PRINT_ON_ERROR(result);
    return result;
}