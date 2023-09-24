#pragma once

#include <filesystem>
#include <string>


namespace build
{

    namespace graphs
    {
    
        inline extern const std::filesystem::path clSourceRoot = std::filesystem::current_path() / "OpenCL Source";
        inline extern const std::filesystem::path clBinaryRoot = std::filesystem::current_path() / "OpenCL Binaries";
    
#ifdef _DEBUG
        inline extern const std::string options      = "-D DEBUG -cl-opt-disable -Werror -cl-std=CL2.0 -g";
        inline extern const std::string clBinaryName = "graphs_OpenClBinary_Debug.cl.bin";
#else
        inline extern const std::string options      = "-Werror -cl-std=CL2.0";
        inline extern const std::string clBinaryName = "graphs_OpenClBinary_Release.cl.bin";
#endif // _DEBUG
    
    }

}