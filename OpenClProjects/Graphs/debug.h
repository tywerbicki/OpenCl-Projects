#pragma once

#include <CL/cl.h>

#include <iostream>
#include <stdlib.h>


#define OPENCL_CHECK_ERROR(result) debug::_OpenClCheckError(result, __FILE__, __LINE__)


namespace debug
{


inline void _OpenClCheckError(
    const cl_int       result,
    const char* const  fileName,
    const unsigned int lineNumber)
{
    if (result != CL_SUCCESS)
    {
        std::cerr << "\nOPENCL_ERROR\n";

        std::cerr << "File: " << fileName   << "\n";
        std::cerr << "Line: " << lineNumber << "\n";

        std::cerr << "Error code: " << result << "\n";

        std::exit(EXIT_FAILURE);
    }
}


} // debug