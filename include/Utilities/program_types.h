#ifndef UTILITIES_PROGRAM_TYPES_H
#define UTILITIES_PROGRAM_TYPES_H

#include <filesystem>
#include <string>
#include <vector>


namespace program
{
    struct BinaryCreator
    {
        std::filesystem::path clBinaryRoot;
        std::string           clBinaryFileName;
    };

    struct SourceCreator
    {
        std::filesystem::path    clSourceRoot;
        std::vector<std::string> clSourceFileNames;
    };
}


#endif // UTILITIES_PROGRAM_TYPES_H