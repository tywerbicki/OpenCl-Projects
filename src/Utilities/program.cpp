#include "context.h"
#include "debug.h"
#include "device.h"
#include "program.h"
#include "program_types.h"
#include "settings.h"

#include <cassert>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <stdint.h>
#include <system_error>


namespace
{
    cl_int CreateFromBinary(const cl_context              context,
                            const program::BinaryCreator& binCreator,
                            std::optional<cl_program>&    program)
    {
        program.reset();

        cl_int                    result  = CL_SUCCESS;
        std::vector<cl_device_id> devices = {};

        result = context::GetDevices(context, devices);
        OPENCL_RETURN_ON_ERROR(result);
    
        std::vector<std::vector<unsigned char>> clBinaries(devices.size());
        uint32_t nClBinariesAcquired = 0;
    
        for (size_t i = 0; i < devices.size(); i++)
        {
            std::filesystem::path clBinaryFilePath = {};
            std::error_code       ec               = {};

            result = device::GetClBinaryFilePath(binCreator, devices[i], clBinaryFilePath);
            OPENCL_RETURN_ON_ERROR(result);
    
            const bool clBinaryExists = std::filesystem::exists(clBinaryFilePath, ec);
    
            if (!ec && clBinaryExists)
            {
                std::ifstream clBinaryIfStream(clBinaryFilePath,
                                               std::ios_base::in | std::ios_base::binary | std::ios_base::ate);
    
                if (!clBinaryIfStream.is_open())
                {
                    MSG_STD_ERR("Failed to open input stream for OpenCL program binary: ", clBinaryFilePath);
                    break;
                }
    
                const auto clBinaryEndPos   = clBinaryIfStream.tellg();
                clBinaryIfStream.seekg(std::ios_base::beg);
                const auto clBinaryBeginPos = clBinaryIfStream.tellg();
    
                if (clBinaryIfStream.fail())
                {
                    MSG_STD_ERR("Failed to determine size in bytes of OpenCL program binary: ", clBinaryFilePath);
                    break;
                }
    
                clBinaries[i].resize(clBinaryEndPos - clBinaryBeginPos);
    
                clBinaryIfStream.read(reinterpret_cast<char*>(clBinaries[i].data()),
                                      clBinaries[i].size());
                
                if (clBinaryIfStream.fail())
                {
                    MSG_STD_ERR("Failed to read OpenCL program binary from file: ", clBinaryFilePath);
                    break;
                }
    
                DBG_MSG_STD_OUT("Context ", context, " acquired OpenCL binary file: ", clBinaryFilePath);
    
                nClBinariesAcquired++;
            }
            else
            {
                DBG_MSG_STD_OUT("Must create program for context ", context, " from source: ", clBinaryFilePath, " does not exist.");
                break;
            }
        }

        if (nClBinariesAcquired == devices.size())
        {
            std::vector<const unsigned char*> clBinaryPtrs(    nClBinariesAcquired);
            std::vector<size_t>               clBinarySizes(   nClBinariesAcquired);
            std::vector<cl_int>               clBinaryStatuses(nClBinariesAcquired);

            for (size_t i = 0; i < nClBinariesAcquired; i++)
            {
                clBinaryPtrs[i]  = clBinaries[i].data();
                clBinarySizes[i] = clBinaries[i].size();
            }

            program = clCreateProgramWithBinary(context,
                                                static_cast<cl_uint>(devices.size()),
                                                devices.data(),
                                                clBinarySizes.data(),
                                                clBinaryPtrs.data(),
                                                clBinaryStatuses.data(),
                                                &result);

#ifdef _DEBUG
            if (result == CL_SUCCESS)
            {
                DBG_MSG_STD_OUT("Successfully created program ", program.value(), " from binary for context: ", context);
            }
            else
            {
                for (size_t i = 0; i < devices.size(); i++)
                {
                    if (clBinaryStatuses[i] != CL_SUCCESS)
                    {
                        std::string uniqueId = {};

                        const cl_int dbgResult = device::GetUniqueId(devices[i], uniqueId);
                        OPENCL_RETURN_ON_ERROR(dbgResult);

                        DBG_MSG_STD_ERR("Error loading program binary for ", uniqueId, ": ", clBinaryStatuses[i]);
                    }
                }

                return result;
            }
#endif // _DEBUG

            OPENCL_RETURN_ON_ERROR(result);
        }
        else
        {
            // TODO: add debug message saying not all binaries were found.
        }

        return result;
    }


    cl_int CreateFromSource(const cl_context              context,
                            const program::SourceCreator& srcCreator,
                            cl_program&                   program)
    {
        std::vector<std::string> clSource      = {};
        std::vector<const char*> clSourceCStrs = {};
        std::vector<size_t>      clSourceSizes = {};

        for (const std::string& clSourceFileName : srcCreator.clSourceFileNames)
        {
            const std::filesystem::path clSourceFilePath = srcCreator.clSourceRoot / clSourceFileName;

            const std::ifstream clSourceIfStream(clSourceFilePath);

            if (!clSourceIfStream.is_open())
            {
                MSG_STD_ERR("Failed to open OpenCL source text input stream for source file: ", clSourceFilePath);
                return CL_BUILD_PROGRAM_FAILURE;
            }

            std::ostringstream clSourceOsStream = {};
            clSourceOsStream << clSourceIfStream.rdbuf();

            if (clSourceOsStream.fail())
            {
                MSG_STD_ERR("Failed to read OpenCL source text for source file: ", clSourceFilePath);
                return CL_BUILD_PROGRAM_FAILURE;
            }

            clSource.push_back(std::move(clSourceOsStream.str()));
            clSourceCStrs.push_back(clSource.back().c_str());
            clSourceSizes.push_back(clSource.back().size());
    
            DBG_MSG_STD_OUT("Context ", context, " acquired OpenCL source file: ", clSourceFilePath);
        }

        cl_int result = CL_SUCCESS;

        program = clCreateProgramWithSource(context,
                                            static_cast<cl_uint>(clSourceCStrs.size()),
                                            clSourceCStrs.data(),
                                            clSourceSizes.data(),
                                            &result);

        OPENCL_PRINT_ON_ERROR(result);
    
        DBG_CL_COND_MSG_STD_OUT(result, "Successfully created program ", program, " from source for context: ", context);

        return result;
    }


    cl_int GetBuildStatus(const cl_program   program,
                          const cl_device_id device,
                          cl_build_status&   buildStatus)
    {
        cl_int result = CL_SUCCESS;

        result = clGetProgramBuildInfo(program,
                                       device,
                                       CL_PROGRAM_BUILD_STATUS,
                                       sizeof(buildStatus),
                                       &buildStatus,
                                       nullptr);

        OPENCL_PRINT_ON_ERROR(result);

        return result;
    }


    cl_int GetBuildLog(const cl_program   program,
                       const cl_device_id device,
                       std::string&       buildLog)
    {
        cl_int result                = CL_SUCCESS;
        size_t paramValueSizeInBytes = 0;

        result = clGetProgramBuildInfo(program,
                                       device,
                                       CL_PROGRAM_BUILD_LOG,
                                       0,
                                       nullptr,
                                       &paramValueSizeInBytes);

        OPENCL_RETURN_ON_ERROR(result);

        // 1. `paramValueSizeInBytes` includes the NULL terminator, hence "-1".
        // 2. This is legal to do from C++11 onwards.
        buildLog.resize(paramValueSizeInBytes - 1);

        result = clGetProgramBuildInfo(program,
                                       device,
                                       CL_PROGRAM_BUILD_LOG,
                                       paramValueSizeInBytes,
                                       buildLog.data(),
                                       nullptr);

        OPENCL_PRINT_ON_ERROR(result);

        return result;
    }


    cl_int StoreBinaries(const cl_program              program,
                         const program::BinaryCreator& binCreator)
    {
        cl_int                    result  = CL_SUCCESS;
        std::vector<cl_device_id> devices = {};
    
        result = program::GetDevices(program, devices);
        OPENCL_RETURN_ON_ERROR(result);
    
        std::vector<std::vector<unsigned char>> clBinaries(   devices.size());
        std::vector<unsigned char*>             clBinaryPtrs( devices.size());
        std::vector<size_t>                     clBinarySizes(devices.size());
    
        result = clGetProgramInfo(program,
                                  CL_PROGRAM_BINARY_SIZES,
                                  clBinarySizes.size() * sizeof(size_t),
                                  clBinarySizes.data(),
                                  nullptr);

        OPENCL_RETURN_ON_ERROR(result);
    
        for (size_t i = 0; i < clBinaries.size(); i++)
        {
            clBinaries[i].resize(clBinarySizes[i]);
            clBinaryPtrs[i] = clBinaries[i].data();
        }
    
        result = clGetProgramInfo(program,
                                  CL_PROGRAM_BINARIES,
                                  clBinaryPtrs.size() * sizeof(unsigned char*),
                                  clBinaryPtrs.data(),
                                  nullptr);

        OPENCL_RETURN_ON_ERROR(result);
    
        uint32_t nClBinariesPersisted = 0;
    
        for (size_t i = 0; i < devices.size(); i++)
        {
            std::filesystem::path clBinaryDir = {};
    
            result = device::GetClBinaryDir(binCreator.clBinaryRoot, devices[i], clBinaryDir);
            OPENCL_RETURN_ON_ERROR(result);
    
            try
            {
                std::filesystem::create_directories(clBinaryDir);
            }
            catch (const std::filesystem::filesystem_error& e)
            {
                MSG_STD_ERR("Failed to create directory ", binCreator.clBinaryRoot, " to store program binary ",
                    binCreator.clBinaryFileName, ": ", e.what(), " (error code: ", e.code(), ")");
                break;
            }

            const std::filesystem::path clBinaryFilePath = clBinaryDir / binCreator.clBinaryFileName;
    
            std::ofstream clBinaryOfStream(clBinaryFilePath,
                                           std::ios_base::out | std::ios_base::binary | std::ios_base::trunc);
    
            if (!clBinaryOfStream.is_open())
            {
                MSG_STD_ERR("Failed to open output stream for OpenCL program binary: ", clBinaryFilePath);
                break;
            }
    
            clBinaryOfStream.write(reinterpret_cast<const char*>(clBinaries[i].data()),
                                   clBinaries[i].size());
    
            if (clBinaryOfStream.fail())
            {
                MSG_STD_ERR("Failed to write OpenCL program binary to file: ", clBinaryFilePath);
                break;
            }
    
            nClBinariesPersisted++;
        }
    
        DBG_BOOL_COND_MSG_STD_OUT(nClBinariesPersisted == clBinaries.size(),
                                  "Successfully persisted all binaries for program: ", program);
    
        return result;
    }
}


cl_int program::GetDevices(const cl_program           program,
                           std::vector<cl_device_id>& devices)
{
    cl_int  result   = CL_SUCCESS;
    cl_uint nDevices = 0;

    result = clGetProgramInfo(program,
                              CL_PROGRAM_NUM_DEVICES,
                              sizeof(nDevices),
                              &nDevices,
                              nullptr);

    OPENCL_RETURN_ON_ERROR(result);

    devices.resize(nDevices);

    result = clGetProgramInfo(program,
                              CL_PROGRAM_DEVICES,
                              devices.size() * sizeof(cl_device_id),
                              devices.data(),
                              nullptr);

    OPENCL_PRINT_ON_ERROR(result);

    return result;
}


cl_int program::Build(const cl_context                                                 context,
                      const std::optional<std::reference_wrapper<const BinaryCreator>> binCreator,
                      const SourceCreator&                                             srcCreator,
                      const std::string&                                               clBuildOptions,
                      cl_program&                                                      program)
{
    cl_int                    result                      = CL_SUCCESS;
    std::optional<cl_program> programCreatedFromBinary    = std::nullopt;
    const bool                programBinaryCachingEnabled = settings::enableProgramBinaryCaching &&
                                                            binCreator.has_value();

    if (programBinaryCachingEnabled)
    {
        result = CreateFromBinary(context, binCreator.value(), programCreatedFromBinary);
        OPENCL_RETURN_ON_ERROR(result);
    }

    if (settings::forceCreateProgramFromSource || !programCreatedFromBinary.has_value())
    {
        result = CreateFromSource(context, srcCreator, program);
        OPENCL_RETURN_ON_ERROR(result);
    }
    else
    {
        program = programCreatedFromBinary.value();
    }

    std::vector<cl_device_id> devices = {};

    result = context::GetDevices(context, devices);
    OPENCL_RETURN_ON_ERROR(result);

    result = clBuildProgram(program,
                            static_cast<cl_uint>(devices.size()),
                            devices.data(),
                            clBuildOptions.c_str(),
                            nullptr,
                            nullptr);

    switch (result)
    {
    case CL_BUILD_PROGRAM_FAILURE:
    {
        for (const cl_device_id device : devices)
        {
            cl_build_status buildStatus = CL_BUILD_NONE;

            result = GetBuildStatus(program, device, buildStatus);
            OPENCL_RETURN_ON_ERROR(result);

            if (buildStatus != CL_BUILD_SUCCESS)
            {
                MSG_STD_ERR("Failed to build program ", program, " for device: ", device);

                std::string buildLog = {};

                result = GetBuildLog(program, device, buildLog);
                OPENCL_RETURN_ON_ERROR(result);

                MSG_STD_ERR(buildLog);
            }
        }

        return CL_BUILD_PROGRAM_FAILURE;
    }

    case CL_SUCCESS:
    {
        DBG_MSG_STD_OUT("Successfully built program ", program, " for context: ", context);

        if (programBinaryCachingEnabled && !programCreatedFromBinary.has_value())
        {
            result = StoreBinaries(program, binCreator.value());
            OPENCL_RETURN_ON_ERROR(result);
        }

        break;
    }

    default:
        OPENCL_PRINT_ON_ERROR(result);
        return result;
    }

    return result;
}


cl_int program::CreateKernels(const cl_program                   program,
                              const std::span<const std::string> kernelNames,
                              const std::span<cl_kernel>         kernels)
{
    assert(kernelNames.size() == kernels.size());

    cl_int result = CL_SUCCESS;

    for (size_t i = 0; i < kernelNames.size(); i++)
    {
        kernels[i] = clCreateKernel(program,
                                    kernelNames[i].c_str(),
                                    &result);

        OPENCL_RETURN_ON_ERROR(result);

        DBG_MSG_STD_OUT("Successfully created ", kernelNames[i], " kernel for program: ", program);
    }

    return result;
}