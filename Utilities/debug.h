#pragma once

#include <CL/cl.h>

#include <iostream>
#include <stdint.h>


#define MSG_STD_OUT(...) debug::_DisplayMessage(std::cout, "MESSAGE", __FILE__, __FUNCTION__, __LINE__, __VA_ARGS__)
#define MSG_STD_ERR(...) debug::_DisplayMessage(std::cerr, "ERROR"  , __FILE__, __FUNCTION__, __LINE__, __VA_ARGS__)


#ifdef _DEBUG


#define DBG_MSG_STD_OUT(...) debug::_DisplayMessage(std::cout, "DEBUG MESSAGE", __FILE__, __FUNCTION__, __LINE__, __VA_ARGS__)
#define DBG_MSG_STD_ERR(...) debug::_DisplayMessage(std::cerr, "DEBUG ERROR"  , __FILE__, __FUNCTION__, __LINE__, __VA_ARGS__)

#define DBG_CL_COND_MSG_STD_OUT(result, ...) if ((result) == CL_SUCCESS) DBG_MSG_STD_OUT(__VA_ARGS__)
#define DBG_CL_COND_MSG_STD_ERR(result, ...) if ((result) != CL_SUCCESS) DBG_MSG_STD_ERR(__VA_ARGS__)

#define DBG_BOOL_COND_MSG_STD_OUT(boolExpr, ...) if (boolExpr)    DBG_MSG_STD_OUT(__VA_ARGS__)
#define DBG_BOOL_COND_MSG_STD_ERR(boolExpr, ...) if (!(boolExpr)) DBG_MSG_STD_ERR(__VA_ARGS__)


#define OPENCL_PRINT_ON_ERROR(result) if ((result) != CL_SUCCESS) debug::_OpenClDisplayError(result, __FILE__, __FUNCTION__, __LINE__)


#define OPENCL_RETURN_ON_ERROR(result) if ((result) != CL_SUCCESS) \
                                       { \
                                           debug::_OpenClDisplayError(result, __FILE__, __FUNCTION__, __LINE__); \
                                           return result; \
                                       }


#else


#define DBG_MSG_STD_OUT(...)
#define DBG_MSG_STD_ERR(...)

#define DBG_CL_COND_MSG_STD_OUT(result, ...)
#define DBG_CL_COND_MSG_STD_ERR(result, ...)

#define DBG_BOOL_COND_MSG_STD_OUT(boolExpr, ...)
#define DBG_BOOL_COND_MSG_STD_ERR(boolExpr, ...)


#define OPENCL_PRINT_ON_ERROR(result)


#define OPENCL_RETURN_ON_ERROR(result) if ((result) != CL_SUCCESS) return result


#endif // _DEBUG


namespace debug
{


template<typename... Args>
void _DisplayMessage(
    std::ostream&     oStream,
    const char* const title,
    const char* const fileName,
    const char* const callerName,
    const uint32_t    lineNumber,
    Args&&...         args)
{
    oStream << "\n" << title << "\n";

    oStream << "File: "     << fileName   << "\n";
    oStream << "Function: " << callerName << "\n";
    oStream << "Line: "     << lineNumber << "\n";

    (oStream << ... << args) << "\n";
}


void _OpenClDisplayError(
    const cl_int      error,
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


} // debug