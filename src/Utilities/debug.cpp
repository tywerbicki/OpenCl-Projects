#include "debug.h"


void debug::DisplayOpenClError(const cl_int           error,
                               const std::string_view fileName,
                               const std::string_view callerName,
                               const uint32_t         lineNumber)
{
    std::cerr << "\nOPENCL ERROR\n";

    std::cerr << "File: "       << fileName   << "\n";
    std::cerr << "Function: "   << callerName << "\n";
    std::cerr << "Line: "       << lineNumber << "\n";
    std::cerr << "Error code: " << error      << "\n";
}