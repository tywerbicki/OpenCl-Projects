#ifndef UTILITIES_DEVICE_H
#define UTILITIES_DEVICE_H

#include <CL/cl.h>

#include <filesystem>
#include <string>
#include <vector>


namespace program
{
    struct BinaryCreator;
}


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

    [[nodiscard]] cl_int GetUniqueId(cl_device_id device,
                                     std::string& uniqueId);

    [[nodiscard]] cl_int GetClBinaryDir(const std::filesystem::path& clBinaryRoot,
                                        cl_device_id                 device,
                                        std::filesystem::path&       clBinaryDir);

    [[nodiscard]] cl_int GetClBinaryFilePath(const program::BinaryCreator& binCreator,
                                             cl_device_id                  device,
                                             std::filesystem::path&        clBinaryFilePath);
}


#endif // UTILITIES_DEVICE_H