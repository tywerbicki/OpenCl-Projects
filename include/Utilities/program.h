#ifndef UTILITIES_PROGRAM_H
#define UTILITIES_PROGRAM_H

#include <CL/cl.h>

#include <functional>
#include <optional>
#include <span>
#include <string>
#include <vector>


namespace program
{
    struct BinaryCreator;
    struct SourceCreator;


    [[nodiscard]] cl_int GetDevices(cl_program                 program,
                                    std::vector<cl_device_id>& devices);

    [[nodiscard]] cl_int Build(cl_context                                                 context,
                               std::optional<std::reference_wrapper<const BinaryCreator>> binCreator,
                               const SourceCreator&                                       srcCreator,
                               const std::string&                                         clBuildOptions,
                               cl_program&                                                program);

    [[nodiscard]] cl_int CreateKernels(cl_program                   program,
                                       std::span<const std::string> kernelNames,
                                       std::span<cl_kernel>         kernels);
}


#endif // UTILITIES_PROGRAM_H