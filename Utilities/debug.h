#pragma once

#include <CL/cl.h>

#include <iostream>
#include <stdint.h>


#define MSG_STD_OUT(...) debug::_DisplayMessage(std::cout, __FILE__, __LINE__, __VA_ARGS__)
#define MSG_STD_ERR(...) debug::_DisplayMessage(std::cerr, __FILE__, __LINE__, __VA_ARGS__)


#ifdef _DEBUG


#define DBG_MSG_STD_OUT(...) MSG_STD_OUT(__VA_ARGS__)
#define DBG_MSG_STD_ERR(...) MSG_STD_ERR(__VA_ARGS__)


#define OPENCL_PRINT_ON_ERROR(result) if ((result) != CL_SUCCESS) debug::_OpenClDisplayError(result, __FILE__, __LINE__)


#define OPENCL_RETURN_ON_ERROR(result) if ((result) != CL_SUCCESS) \
                                       { \
                                           debug::_OpenClDisplayError(result, __FILE__, __LINE__); \
                                           return result; \
                                       }


#else


#define DBG_MSG_STD_OUT(...)
#define DBG_MSG_STD_ERR(...)


#define OPENCL_PRINT_ON_ERROR(result)


#define OPENCL_RETURN_ON_ERROR(result) if ((result) != CL_SUCCESS) return result


#endif // _DEBUG


namespace debug
{


template<typename... Args>
void _DisplayMessage(
    std::ostream&     oStream,
    const char* const fileName,
    const uint32_t    lineNumber,
    Args&&...         args)
{
    oStream << "\nDEBUG MESSAGE\n";

    oStream << "File: " << fileName   << "\n";
    oStream << "Line: " << lineNumber << "\n";

    (oStream << ... << args) << "\n";
}


void _OpenClDisplayError(
    const cl_int      error,
    const char* const fileName,
    const uint32_t    lineNumber)
{
    std::cerr << "\nOPENCL ERROR\n";

    std::cerr << "File: " << fileName   << "\n";
    std::cerr << "Line: " << lineNumber << "\n";

    std::cerr << "Error code: " << error << "\n";
}


} // debug