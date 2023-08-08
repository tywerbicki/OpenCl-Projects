#pragma once

#include <CL/cl.h>

#include <algorithm>
#include <array>
#include <iostream>
#include <vector>

#include "debug.h"


namespace device
{


cl_int GetAvailableDevices(
    const cl_platform_id       platform,
    std::vector<cl_device_id>& deviceIds)
{
    cl_int  result   = CL_SUCCESS;
    cl_uint nDevices = 0;

    // Query number of available devices.
    result = clGetDeviceIDs(
        platform,
        CL_DEVICE_TYPE_ALL,
        0,
        nullptr,
        &nDevices
    );
    OPENCL_RETURN_ON_ERROR(result);

    deviceIds.resize(nDevices);

    // Query ids of all available devices.
    result = clGetDeviceIDs(
        platform,
        CL_DEVICE_TYPE_ALL,
        nDevices,
        deviceIds.data(),
        nullptr
    );
    OPENCL_PRINT_ON_ERROR(result);

    return result;
}


template<typename ParamType>
cl_int QueryDeviceParamValue(
    const cl_device_id      device,
    const cl_device_info    paramName,
    std::vector<ParamType>& paramValue)
{
    cl_int result                = CL_SUCCESS;
    size_t paramValueSizeInBytes = 0;

    // Query size of parameter value.
    result = clGetDeviceInfo(
        device,
        paramName,
        0,
        nullptr,
        &paramValueSizeInBytes
    );
    OPENCL_RETURN_ON_ERROR(result);

    paramValue.resize(paramValueSizeInBytes / sizeof(ParamType));

    // Query parameter value.
    result = clGetDeviceInfo(
        device,
        paramName,
        paramValueSizeInBytes,
        paramValue.data(),
        nullptr
    );
    OPENCL_PRINT_ON_ERROR(result);

    return result;
}


template<typename ParamType>
cl_int QueryDeviceParamValue(
    const cl_device_id   device,
    const cl_device_info paramName,
    ParamType&           paramValue)
{

}


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


cl_int AcquireDevice(
    const cl_platform_id platform,
    cl_device_id&        device)
{
    cl_int                      result  = CL_SUCCESS;
    std::vector<cl_device_id> deviceIds = {};

    result = GetAvailableDevices(platform, deviceIds);
    OPENCL_RETURN_ON_ERROR(result);

    #ifdef _DEBUG

    for (const auto deviceId : deviceIds)
    {
        result = platform::DisplayPlatformInfo(platformId);
        OPENCL_RETURN_ON_ERROR(result);
    }

    #endif // _DEBUG
}



} // device