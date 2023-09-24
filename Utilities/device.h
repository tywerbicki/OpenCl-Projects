#pragma once

#include <CL/cl.h>

#include <filesystem>
#include <string>
#include <string_view>
#include <vector>


namespace device
{

    cl_int GetAllAvailable(const cl_platform_id       platform,
                           std::vector<cl_device_id>& devices);

    template<typename ParamType>
    cl_int QueryParamValue(const cl_device_id   device,
                           const cl_device_info paramName,
                           ParamType&           paramValue);

    template<typename ParamType>
    cl_int QueryParamValue(const cl_device_id      device,
                           const cl_device_info    paramName,
                           std::vector<ParamType>& paramValue);

    cl_int QueryParamValue(const cl_device_id   device,
                           const cl_device_info paramName,
                           std::string&         paramValue);

    cl_int DisplayGeneralInfo(const cl_device_id device);

    cl_int IsGpu(const cl_device_id device,
                 bool&              isGpu);

    cl_int QueryUniqueId(const cl_device_id device,
                         std::string&       uniqueId);

    cl_int GetClBinaryDir(const std::filesystem::path& clBinaryRoot,
                          const cl_device_id           device,
                          std::filesystem::path&       clBinaryDir);

    cl_int GetClBinaryPath(const std::filesystem::path& clBinaryRoot,
                           const cl_device_id           device,
                           const std::string_view       clBinaryName,
                           std::filesystem::path&       clBinaryPath);

}