#pragma once

#include <CL/cl.h>

#include <filesystem>
#include <string>
#include <string_view>
#include <vector>


namespace device
{

    [[nodiscard]] cl_int GetAllAvailable(cl_platform_id             platform,
                                         std::vector<cl_device_id>& devices);

    template<typename ParamType>
    [[nodiscard]] cl_int QueryParamValue(cl_device_id   device,
                                         cl_device_info paramName,
                                         ParamType&     paramValue);

    [[nodiscard]] cl_int QueryParamValue(cl_device_id   device,
                                         cl_device_info paramName,
                                         std::string&   paramValue);

    [[nodiscard]] cl_int DisplayGeneralInfo(cl_device_id device);

    [[nodiscard]] cl_int IsGpu(cl_device_id device,
                               bool&        isGpu);

    [[nodiscard]] cl_int QueryUniqueId(cl_device_id device,
                                       std::string& uniqueId);

    [[nodiscard]] cl_int GetClBinaryDir(const std::filesystem::path& clBinaryRoot,
                                        cl_device_id                 device,
                                        std::filesystem::path&       clBinaryDir);

    [[nodiscard]] cl_int GetClBinaryPath(const std::filesystem::path& clBinaryRoot,
                                         cl_device_id                 device,
                                         std::string_view             clBinaryName,
                                         std::filesystem::path&       clBinaryPath);

}