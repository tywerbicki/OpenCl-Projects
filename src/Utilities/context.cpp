#include "context.h"
#include "debug.h"
#include "platform.h"

#include <array>


cl_int context::GetDevices(const cl_context           context,
                           std::vector<cl_device_id>& devices)
{
    cl_int  result   = CL_SUCCESS;
    cl_uint nDevices = 0;

    result = clGetContextInfo(context,
                              CL_CONTEXT_NUM_DEVICES,
                              sizeof(nDevices),
                              &nDevices,
                              nullptr);

    OPENCL_RETURN_ON_ERROR(result);

    devices.resize(nDevices);

    result = clGetContextInfo(context,
                              CL_CONTEXT_DEVICES,
                              devices.size() * sizeof(cl_device_id),
                              devices.data(),
                              nullptr);

    OPENCL_PRINT_ON_ERROR(result);

    return result;
}


cl_int context::Create(platform::UniSelectionStrategy strategy,
                       std::optional<cl_platform_id>& selectedPlatform,
                       std::optional<cl_context>&     context)
{
    selectedPlatform.reset();
    context.reset();

    cl_int                      result              = CL_SUCCESS;
    std::vector<cl_platform_id> conformantPlatforms = {};
    std::vector<cl_device_id>   selectedDevices     = {};

    result = platform::GetAllConformant(conformantPlatforms);
    OPENCL_RETURN_ON_ERROR(result);

    result = strategy(conformantPlatforms, selectedPlatform, selectedDevices);
    OPENCL_RETURN_ON_ERROR(result);

    if (selectedPlatform.has_value())
    {
        const std::array<const cl_context_properties, 3> properties
        {
            CL_CONTEXT_PLATFORM,
            reinterpret_cast<cl_context_properties>(selectedPlatform.value()),
            0
        };

        context = clCreateContext(properties.data(),
                                  static_cast<cl_uint>(selectedDevices.size()),
                                  selectedDevices.data(),
                                  nullptr,
                                  nullptr,
                                  &result);

        OPENCL_PRINT_ON_ERROR(result);
    }
    else
    {
        MSG_STD_OUT("No OpenCL platform was selected.");
    }

    return result;
}