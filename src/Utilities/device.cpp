#include "debug.h"
#include "device.h"
#include "program_types.h"

#include <array>


cl_int device::GetAllAvailable(const cl_platform_id       platform,
                               std::vector<cl_device_id>& devices)
{
    cl_int  result   = CL_SUCCESS;
    cl_uint nDevices = 0;

    result = clGetDeviceIDs(platform,
                            CL_DEVICE_TYPE_ALL,
                            0,
                            nullptr,
                            &nDevices);

    OPENCL_RETURN_ON_ERROR(result);

    devices.resize(nDevices);

    result = clGetDeviceIDs(platform,
                            CL_DEVICE_TYPE_ALL,
                            nDevices,
                            devices.data(),
                            nullptr);

    OPENCL_PRINT_ON_ERROR(result);

    return result;
}


template<typename ParamType>
cl_int device::QueryParamValue(const cl_device_id   device,
                               const cl_device_info paramName,
                               ParamType&           paramValue)
{
    cl_int result = CL_SUCCESS;

    result = clGetDeviceInfo(device,
                             paramName,
                             sizeof(ParamType),
                             &paramValue,
                             nullptr);

    OPENCL_PRINT_ON_ERROR(result);

    return result;
}


cl_int device::QueryParamValue(const cl_device_id   device,
                               const cl_device_info paramName,
                               std::string&         paramValue)
{
    cl_int result                = CL_SUCCESS;
    size_t paramValueSizeInBytes = 0;

    result = clGetDeviceInfo(device,
                             paramName,
                             0,
                             nullptr,
                             &paramValueSizeInBytes);

    OPENCL_RETURN_ON_ERROR(result);

    // 1. `paramValueSizeInBytes` includes the NULL terminator, hence "-1".
    // 2. This is legal to do from C++11 onwards.
    paramValue.resize(paramValueSizeInBytes - 1);

    result = clGetDeviceInfo(device,
                             paramName,
                             paramValueSizeInBytes,
                             paramValue.data(),
                             nullptr);

    OPENCL_PRINT_ON_ERROR(result);

    return result;
}


cl_int device::DisplayGeneralInfo(const cl_device_id device)
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

        result = QueryParamValue(device,
                                 paramNames[i],
                                 paramValue);

        OPENCL_RETURN_ON_ERROR(result);

        msgStdOut << paramNameStrs[i] << ": " << paramValue << "\n";
    }

    MSG_STD_OUT(msgStdOut.view());

    return result;
}


cl_int device::IsGpu(const cl_device_id device,
                     bool&              isGpu)
{
    cl_int         result     = CL_SUCCESS;
    cl_device_type deviceType = 0;

    result = QueryParamValue(device,
                             CL_DEVICE_TYPE,
                             deviceType);

    OPENCL_RETURN_ON_ERROR(result);

    isGpu = (deviceType == CL_DEVICE_TYPE_GPU);

    return result;
}


cl_int device::GetUniqueId(const cl_device_id device,
                           std::string&       uniqueId)
{
    UNUSED_PARAMETER(device);

    cl_int result = CL_SUCCESS;

    // TODO: create a hash of device characteristics.
    uniqueId = "QuadroP1000";

    return result;
}


cl_int device::GetClBinaryDir(const std::filesystem::path& clBinaryRoot,
                              const cl_device_id           device,
                              std::filesystem::path&       clBinaryDir)
{
    cl_int      result       = CL_SUCCESS;
    std::string deviceVendor = {};
    std::string uniqueId     = {};

    result = QueryParamValue(device,
                             CL_DEVICE_VENDOR,
                             deviceVendor);

    OPENCL_RETURN_ON_ERROR(result);

    result = GetUniqueId(device, uniqueId);
    OPENCL_RETURN_ON_ERROR(result);

    clBinaryDir = clBinaryRoot / deviceVendor / uniqueId;

    return result;
}


cl_int device::GetClBinaryFilePath(const program::BinaryCreator& binCreator,
                                   const cl_device_id            device,
                                   std::filesystem::path&        clBinaryFilePath)
{
    cl_int result = CL_SUCCESS;

    result = GetClBinaryDir(binCreator.clBinaryRoot, device, clBinaryFilePath);
    OPENCL_RETURN_ON_ERROR(result);

    clBinaryFilePath /= binCreator.clBinaryFileName;

    return result;
}