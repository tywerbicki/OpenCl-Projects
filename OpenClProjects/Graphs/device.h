#pragma once

#include <CL/cl.h>

#include <array>
#include <filesystem>
#include <iostream>
#include <string>
#include <vector>

#include "build.h"
#include "debug.h"
#include "required.h"


namespace device
{


cl_int GetAllAvailable(
    const cl_platform_id       platform,
    std::vector<cl_device_id>& devices)
{
    cl_int  result   = CL_SUCCESS;
    cl_uint nDevices = 0;

    result = clGetDeviceIDs(
        platform,
        CL_DEVICE_TYPE_ALL,
        0,
        nullptr,
        &nDevices
    );
    OPENCL_RETURN_ON_ERROR(result);

    devices.resize(nDevices);

    result = clGetDeviceIDs(
        platform,
        CL_DEVICE_TYPE_ALL,
        nDevices,
        devices.data(),
        nullptr
    );
    OPENCL_PRINT_ON_ERROR(result);

    return result;
}


template<typename ParamType>
cl_int QueryParamValue(
    const cl_device_id   device,
    const cl_device_info paramName,
    ParamType&           paramValue)
{
    cl_int result = CL_SUCCESS;

    result = clGetDeviceInfo(
        device,
        paramName,
        sizeof(ParamType),
        &paramValue,
        nullptr
    );
    OPENCL_PRINT_ON_ERROR(result);

    return result;
}


template<typename ParamType>
cl_int QueryParamValue(
    const cl_device_id      device,
    const cl_device_info    paramName,
    std::vector<ParamType>& paramValue)
{
    cl_int result                = CL_SUCCESS;
    size_t paramValueSizeInBytes = 0;

    result = clGetDeviceInfo(
        device,
        paramName,
        0,
        nullptr,
        &paramValueSizeInBytes
    );
    OPENCL_RETURN_ON_ERROR(result);

    paramValue.resize(paramValueSizeInBytes / sizeof(ParamType));

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


cl_int QueryParamValue(
    const cl_device_id   device,
    const cl_device_info paramName,
    std::string&         paramValue)
{
    cl_int            result        = CL_SUCCESS;
    std::vector<char> paramValueVec = {};

    result = QueryParamValue(
        device,
        paramName,
        paramValueVec
    );
    OPENCL_RETURN_ON_ERROR(result);

    paramValue.assign(
        paramValueVec.data(),
        paramValueVec.size() - 1
    );

    return result;
}


cl_int DisplayGeneralInfo(const cl_device_id device)
{
    const std::array<const cl_device_info, 9> paramNames
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

    const std::array<const std::string, paramNames.size()> paramNameStrs
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

        result = QueryParamValue(
            device,
            paramNames[i],
            paramValue
        );
        OPENCL_RETURN_ON_ERROR(result);

        std::cout << paramNameStrs[i] << ": " << paramValue << "\n";
    }

    return result;
}


cl_int IsConformant(const cl_device_id device, bool& isConformant)
{
    cl_int         result     = CL_SUCCESS;
    cl_device_type deviceType = 0;

    result = QueryParamValue(
        device,
        CL_DEVICE_TYPE,
        deviceType
    );
    OPENCL_RETURN_ON_ERROR(result);

    if (deviceType != required::DeviceType)
    {
        isConformant = false;
        return result;
    }

    isConformant = true;
    return result;
}


cl_int QueryUniqueId(const cl_device_id device, std::string& uniqueId)
{
    cl_int result = CL_SUCCESS;

    // Pretend that we generated a unique identifier for the device.
    // This could be done by hashing some device characteristics.
    uniqueId = "QuadroP1000";

    return result;
}


cl_int GetClBinaryDir(const cl_device_id device, std::filesystem::path& clBinaryDir)
{
    cl_int      result       = CL_SUCCESS;
    std::string deviceVendor = {};

    result = QueryParamValue(
        device,
        CL_DEVICE_VENDOR,
        deviceVendor
    );
    OPENCL_RETURN_ON_ERROR(result);

    std::string uniqueId = {};
    result               = QueryUniqueId(device, uniqueId);
    OPENCL_RETURN_ON_ERROR(result);

    clBinaryDir = build::clBinariesRoot / deviceVendor / uniqueId;

    return result;
}


cl_int GetClBinaryPath(const cl_device_id device, std::filesystem::path& clBinaryPath)
{
    cl_int result = CL_SUCCESS;

    result = GetClBinaryDir(device, clBinaryPath);
    OPENCL_RETURN_ON_ERROR(result);

    clBinaryPath /= build::clBinariesFileName;

    return result;
}


} // device