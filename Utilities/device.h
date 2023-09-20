#pragma once

#include <CL/cl.h>

#include <array>
#include <filesystem>
#include <iostream>
#include <string>
#include <vector>

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

    cl_int             result    = CL_SUCCESS;
    std::ostringstream msgStdOut = {};

    for (size_t i = 0; i < paramNames.size(); i++)
    {
        cl_ulong paramValue = 0;

        result = QueryParamValue(
            device,
            paramNames[i],
            paramValue
        );
        OPENCL_RETURN_ON_ERROR(result);

        msgStdOut << paramNameStrs[i] << ": " << paramValue << "\n";
    }

    MSG_STD_OUT(msgStdOut.view());

    return result;
}


cl_int IsGpu(const cl_device_id device, bool& isGpu)
{
    cl_int         result     = CL_SUCCESS;
    cl_device_type deviceType = 0;

    result = QueryParamValue(
        device,
        CL_DEVICE_TYPE,
        deviceType
    );
    OPENCL_RETURN_ON_ERROR(result);

    isGpu = (deviceType == CL_DEVICE_TYPE_GPU);

    return result;
}


cl_int QueryUniqueId(const cl_device_id device, std::string& uniqueId)
{
    cl_int result = CL_SUCCESS;

    // TODO: create a hash of device characteristics.
    uniqueId = "QuadroP1000";

    return result;
}


cl_int GetClBinaryDir(
    const std::filesystem::path& clBinaryRoot,
    const cl_device_id           device,
    std::filesystem::path&       clBinaryDir)
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

    clBinaryDir = clBinaryRoot / deviceVendor / uniqueId;

    return result;
}


cl_int GetClBinaryPath(
    const std::filesystem::path& clBinaryRoot,
    const cl_device_id           device,
    const std::string&           clBinaryName,
    std::filesystem::path&       clBinaryPath)
{
    cl_int result = CL_SUCCESS;

    result = GetClBinaryDir(clBinaryRoot, device, clBinaryPath);
    OPENCL_RETURN_ON_ERROR(result);

    clBinaryPath /= clBinaryName;

    return result;
}


} // device