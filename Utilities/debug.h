#pragma once

#include <CL/cl.h>

#include <iostream>


#ifdef _DEBUG


#define OPENCL_PRINT_ON_ERROR(result) if ((result) != CL_SUCCESS) debug::_OpenClDisplayError(result, __FILE__, __LINE__)


#define OPENCL_RETURN_ON_ERROR(result) if ((result) != CL_SUCCESS) \
                                       { \
                                           debug::_OpenClDisplayError(result, __FILE__, __LINE__); \
                                           return result; \
                                       }


#else


#define OPENCL_PRINT_ON_ERROR(result)


#define OPENCL_RETURN_ON_ERROR(result) if ((result) != CL_SUCCESS) return result


#endif // _DEBUG


namespace debug
{


void _OpenClDisplayError(
    const cl_int       error,
    const char* const  fileName,
    const unsigned int lineNumber)
{
    std::cerr << "\nOPENCL_ERROR\n";

    std::cerr << "File: " << fileName   << "\n";
    std::cerr << "Line: " << lineNumber << "\n";

    std::cerr << "Error code: " << error << "\n";
}


} // debug