#pragma once

#include <array>
#include <filesystem>
#include <string>


namespace build
{


namespace graphs
{


const std::filesystem::path clSourceRoot = std::filesystem::current_path() / "OpenCL Source";
const std::filesystem::path clBinaryRoot = "OpenCL Binaries";

const std::array<const std::string, 1> clSourceNames
{
    "vfadd.cl"
};

#ifdef _DEBUG

const std::string options      = "-D DEBUG -cl-opt-disable -Werror -cl-std=CL2.0 -g";
const std::string clBinaryName = "graphs_OpenClBinary_Debug.cl.bin";

#else

const std::string options      = "-Werror -cl-std=CL2.0";
const std::string clBinaryName = "graphs_OpenClBinary_Release.cl.bin";

#endif // _DEBUG


} // graphs


} // build