#pragma once

#include <CL/cl.h>

#include <algorithm>
#include <array>
#include <iostream>
#include <vector>

#include "debug.h"


namespace device
{


cl_int DisplayGeneralDeviceInfo(const cl_device_id deviceId)
{
    const std::array<cl_device_info, 9> paramNames
    {
        CL_DEVICE_TYPE,
        CL_DEVICE_MAX_COMPUTE_UNITS,
        CL_DEVICE_MAX_WORK_ITEM_DIMENSIONS,
        CL_DEVICE_PREFERRED_VECTOR_WIDTH_INT,
        CL_DEVICE_PREFERRED_VECTOR_WIDTH_LONG,
        CL_DEVICE_PREFERRED_VECTOR_WIDTH_FLOAT,
        CL_DEVICE_PREFERRED_VECTOR_WIDTH_DOUBLE,
        CL_DEVICE_GLOBAL_MEM_SIZE,
        CL_DEVICE_HOST_UNIFIED_MEMORY
    };

    const std::array<const char*, paramNames.size()> paramNameStrs
    {
        "CL_DEVICE_TYPE",
        "CL_DEVICE_MAX_COMPUTE_UNITS",
        "CL_DEVICE_MAX_WORK_ITEM_DIMENSIONS",
        "CL_DEVICE_PREFERRED_VECTOR_WIDTH_INT",
        "CL_DEVICE_PREFERRED_VECTOR_WIDTH_LONG",
        "CL_DEVICE_PREFERRED_VECTOR_WIDTH_FLOAT",
        "CL_DEVICE_PREFERRED_VECTOR_WIDTH_DOUBLE",
        "CL_DEVICE_GLOBAL_MEM_SIZE",
        "CL_DEVICE_HOST_UNIFIED_MEMORY"
    };

    cl_int result = CL_SUCCESS;

    for (size_t i = 0; i < paramNames.size(); i++)
    {
        cl_ulong paramValue = 0;

        // Query parameter value.
        result = clGetDeviceInfo(
            deviceId,
            paramNames[i],
            sizeof(paramValue),
            &paramValue,
            nullptr
        );
        OPENCL_RETURN_ON_ERROR(result);

        std::cout << paramNameStrs[i] << ": " << paramValue << "\n";
    }

    return result;
}


cl_int DisplayAllDevices(const cl_platform_id platformId)
{
    cl_int  result   = CL_SUCCESS;
    cl_uint nDevices = 0;

    // Query number of available devices.
    result = clGetDeviceIDs(
        platformId,
        CL_DEVICE_TYPE_ALL,
        0,
        nullptr,
        &nDevices
    );
    OPENCL_RETURN_ON_ERROR(result);

    std::vector<cl_device_id> deviceIds{nDevices};

    // Query ids of all available devices.
    result = clGetDeviceIDs(
        platformId,
        CL_DEVICE_TYPE_ALL,
        nDevices,
        deviceIds.data(),
        nullptr
    );
    OPENCL_RETURN_ON_ERROR(result);

    std::for_each(
        deviceIds.cbegin(),
        deviceIds.cend(),
        DisplayGeneralDeviceInfo
    );

    return result;
}


} // device