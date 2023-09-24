#include "debug.h"


void debug::_DisplayOpenClError(const cl_int      error,
                                const char* const fileName,
                                const char* const callerName,
                                const uint32_t    lineNumber)
{
    std::cerr << "\nOPENCL ERROR\n";
    
    std::cerr << "File: "       << fileName   << "\n";
    std::cerr << "Function: "   << callerName << "\n";
    std::cerr << "Line: "       << lineNumber << "\n";
    std::cerr << "Error code: " << error      << "\n";
}