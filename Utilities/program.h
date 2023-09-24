#pragma once

#include <CL/cl.h>

#include <filesystem>
#include <string>
#include <vector>

#include "types.h"


namespace program
{

    cl_int GetDevices(const cl_program           program,
                      std::vector<cl_device_id>& devices);

    // TODO: use string views where appropriate.
    cl_int Build(const cl_context             context,
                 const OptionalPathCRef       clBinaryRoot,
                 const OptionalStringCRef     clBinaryName,
                 const std::filesystem::path& clSourceRoot,
                 const std::string&           clBuildOptions,
                 cl_program&                  program);

}