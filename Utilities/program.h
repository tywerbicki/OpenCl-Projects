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

    [[nodiscard]] cl_int GetDevices(cl_program                 program,
                                    std::vector<cl_device_id>& devices);

    [[nodiscard]] cl_int Build(cl_context                                                         context,
                               std::optional<std::reference_wrapper<const std::filesystem::path>> clBinaryRoot,
                               std::optional<std::string_view>                                    clBinaryName,
                               const std::filesystem::path&                                       clSourceRoot,
                               const std::string&                                                 clBuildOptions,
                               cl_program&                                                        program);

    [[nodiscard]] cl_int CreateKernels(cl_program                   program,
                                       std::span<const std::string> kernelNames,
                                       std::span<cl_kernel>         kernels);

}