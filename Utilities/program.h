#pragma once

#include <CL/cl.h>

#include <filesystem>
#include <functional>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <vector>


namespace program
{

    cl_int GetDevices(const cl_program           program,
                      std::vector<cl_device_id>& devices);

    cl_int Build(const cl_context                                                         context,
                 const std::optional<std::reference_wrapper<const std::filesystem::path>> clBinaryRoot,
                 const std::optional<std::string_view>                                    clBinaryName,
                 const std::filesystem::path&                                             clSourceRoot,
                 const std::string&                                                       clBuildOptions,
                 cl_program&                                                              program);

    cl_int CreateKernels(const cl_program                   program,
                         const std::span<const std::string> kernelNames,
                         const std::span<cl_kernel>         kernels);

}