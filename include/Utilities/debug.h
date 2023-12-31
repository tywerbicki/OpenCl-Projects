#ifndef UTILITIES_DEBUG_H
#define UTILITIES_DEBUG_H

#include <CL/cl.h>

#include <iostream>
#include <stdint.h>
#include <string_view>

#define UNUSED_PARAMETER(p) static_cast<void>(p)

#define MSG_STD_OUT(...) debug::DisplayMessage(std::cout, "MESSAGE", __FILE__, __FUNCTION__, __LINE__, __VA_ARGS__)
#define MSG_STD_ERR(...) debug::DisplayMessage(std::cerr, "ERROR"  , __FILE__, __FUNCTION__, __LINE__, __VA_ARGS__)

#ifdef _DEBUG

#define DBG_MSG_STD_OUT(...) debug::DisplayMessage(std::cout, "DEBUG MESSAGE", __FILE__, __FUNCTION__, __LINE__, __VA_ARGS__)
#define DBG_MSG_STD_ERR(...) debug::DisplayMessage(std::cerr, "DEBUG ERROR"  , __FILE__, __FUNCTION__, __LINE__, __VA_ARGS__)

#define DBG_CL_COND_MSG_STD_OUT(result, ...) if ((result) == CL_SUCCESS) DBG_MSG_STD_OUT(__VA_ARGS__)
#define DBG_CL_COND_MSG_STD_ERR(result, ...) if ((result) != CL_SUCCESS) DBG_MSG_STD_ERR(__VA_ARGS__)

#define DBG_BOOL_COND_MSG_STD_OUT(boolExpr, ...) if (boolExpr)    DBG_MSG_STD_OUT(__VA_ARGS__)
#define DBG_BOOL_COND_MSG_STD_ERR(boolExpr, ...) if (!(boolExpr)) DBG_MSG_STD_ERR(__VA_ARGS__)

#define OPENCL_PRINT_ON_ERROR(result) if ((result) != CL_SUCCESS) debug::DisplayOpenClError(result, __FILE__, __FUNCTION__, __LINE__)

#define OPENCL_RETURN_ON_ERROR(result) if ((result) != CL_SUCCESS) \
                                       { \
                                           debug::DisplayOpenClError(result, __FILE__, __FUNCTION__, __LINE__); \
                                           return result; \
                                       }

#elif defined(_RELEASE)

#define DBG_MSG_STD_OUT(...)
#define DBG_MSG_STD_ERR(...)

#define DBG_CL_COND_MSG_STD_OUT(result, ...)
#define DBG_CL_COND_MSG_STD_ERR(result, ...)

#define DBG_BOOL_COND_MSG_STD_OUT(boolExpr, ...)
#define DBG_BOOL_COND_MSG_STD_ERR(boolExpr, ...)

#define OPENCL_PRINT_ON_ERROR(result)

#define OPENCL_RETURN_ON_ERROR(result) if ((result) != CL_SUCCESS) return result

#endif // _RELEASE


namespace debug
{
    template<typename... Args>
    inline void DisplayMessage(std::ostream&          oStream,
                               const std::string_view title,
                               const std::string_view fileName,
                               const std::string_view callerName,
                               const uint32_t         lineNumber,
                               Args&&...              args)
    {
        oStream << "\n" << title << "\n";
    
        oStream << "File: "     << fileName   << "\n";
        oStream << "Function: " << callerName << "\n";
        oStream << "Line: "     << lineNumber << "\n";
    
        (oStream << ... << args) << "\n";
    }

    void DisplayOpenClError(const cl_int           error,
                            const std::string_view fileName,
                            const std::string_view callerName,
                            const uint32_t         lineNumber);
}


#endif // UTILITIES_DEBUG_H