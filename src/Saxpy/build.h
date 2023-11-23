#ifndef SAXPY_BUILD_H
#define SAXPY_BUILD_H

#include "program_types.h"

#include <array>
#include <filesystem>
#include <string>


namespace build::saxpy
{
    inline extern const std::filesystem::path clBinaryRoot = std::filesystem::current_path() / "Saxpy_CL_Binaries";

    inline extern const std::array<const std::string, 1> clKernelNames
    {
        "saxpy"
    };

    inline extern const program::BinaryCreator binaryCreator
    {
        .clBinaryRoot = std::filesystem::current_path() / "Saxpy_CL_Binaries",
#ifdef _DEBUG
        .clBinaryFileName = "saxpy_ClBinary_Debug.cl.bin",
#elif defined(_RELEASE)
        .clBinaryFileName = "saxpy_ClBinary_Release.cl.bin",
#endif // _RELEASE
    };

    inline extern const program::SourceCreator sourceCreator
    {
        .clSourceRoot      = std::filesystem::path(__FILE__).remove_filename(),
        .clSourceFileNames = {"saxpy.cl"}
    };

#ifdef _DEBUG
    inline extern const std::string options = "-D _DEBUG -cl-opt-disable -Werror -cl-std=CL2.0 -g";
#elif defined(_RELEASE)
    inline extern const std::string options = "-D _RELEASE -Werror -cl-std=CL2.0";
#endif // _RELEASE
}


#endif // SAXPY_BUILD_H