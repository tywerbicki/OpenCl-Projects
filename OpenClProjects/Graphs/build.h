#pragma once

#include <array>
#include <filesystem>
#include <string>


namespace build
{


const std::filesystem::path clSourceRoot   = "OpenCL Source";
const std::filesystem::path clBinariesRoot = "OpenCL Binaries";

constexpr size_t nClSourceFiles = 1;

const std::array<const std::string, nClSourceFiles> clSourceFileNames
{
    "vfadd.cl"
};

#ifdef _DEBUG

const std::string options            = "-cl-opt-disable -Werror -cl-std=CL2.0 -g";
const std::string clBinariesFileName = "OpenClBinaries_Debug.cl.bin";

#else

const std::string options            = "-Werror -cl-std=CL2.0";
const std::string clBinariesFileName = "ProgramBinaries_Release.cl.bin";

#endif // _DEBUG


}