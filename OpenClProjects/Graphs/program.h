#pragma once

#include <CL/cl.h>

#include <array>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "debug.h"
#include "context.h"


namespace program
{


const std::string kernelDir = "./Kernels/";

constexpr size_t nKernels = 1;

const std::array<const std::string, nKernels> kernelFileNames
{
	"vfadd.cl"
};

const std::string kernelBinarySuffix = ".clbin";

#ifdef _DEBUG


constexpr char buildOptions[] = "-cl-opt-disable -Werror -cl-std=CL2.0 -g";


#else


constexpr char buildOptions[] = "-Werror -cl-std=CL2.0";


#endif // _DEBUG


cl_int GetBuildLog(
	const cl_program   program,
	const cl_device_id device,
	std::string&       buildLog)
{
	cl_int result                = CL_SUCCESS;
	size_t paramValueSizeInBytes = 0;

	result = clGetProgramBuildInfo(
		program,
		device,
		CL_PROGRAM_BUILD_LOG,
		0,
		nullptr,
		&paramValueSizeInBytes
	);
	OPENCL_RETURN_ON_ERROR(result);

	std::vector<char> paramValue(paramValueSizeInBytes);

	result = clGetProgramBuildInfo(
		program,
		device,
		CL_PROGRAM_BUILD_LOG,
		paramValueSizeInBytes,
		paramValue.data(),
		nullptr
	);
	OPENCL_RETURN_ON_ERROR(result);

	buildLog.assign(
		paramValue.data(),
		paramValueSizeInBytes - 1
	);

	return result;
}


cl_int Build(const cl_context context, cl_program& program)
{
	std::array<std::string, nKernels> kernelsSource = {};

	for (size_t i = 0; i < nKernels; i++)
	{
		const std::string kernelRelPath = kernelDir + kernelFileNames[i];

		// Check if we already have a binary for the kernel.


		std::ifstream kernelIfStream{ kernelRelPath };

		if (!kernelIfStream.is_open())
		{
			std::cerr << "Failed to open kernel: " << kernelFileNames[i] << "\n";
			return CL_BUILD_PROGRAM_FAILURE;
		}

		std::ostringstream kernelOsStream;
		kernelOsStream << kernelIfStream.rdbuf();

		kernelsSource[i] = std::move(kernelOsStream.str());
	}

	const char* kernelsSourceCStr[nKernels];
	size_t      kernelsSourceLen[nKernels];

	for (size_t i = 0; i < nKernels; i++)
	{
		kernelsSourceCStr[i] = kernelsSource[i].c_str();
		kernelsSourceLen[i]  = kernelsSource[i].size();
	}

	cl_int result = CL_SUCCESS;

	program = clCreateProgramWithSource(
		context,
		nKernels,
		kernelsSourceCStr,
		kernelsSourceLen,
		&result
	);
	OPENCL_RETURN_ON_ERROR(result);

	std::vector<cl_device_id> devices = {};
	result                            = context::GetDevices(context, devices);
	OPENCL_RETURN_ON_ERROR(result);

	result = clBuildProgram(
		program,
		static_cast<cl_uint>(devices.size()),
		devices.data(),
		buildOptions,
		nullptr,
		nullptr
	);

	if (result == CL_BUILD_PROGRAM_FAILURE)
	{
		for (const auto device : devices)
		{
			cl_build_status buildStatus = CL_BUILD_NONE;

			result = clGetProgramBuildInfo(
				program,
				device,
				CL_PROGRAM_BUILD_STATUS,
				sizeof(buildStatus),
				&buildStatus,
				nullptr
			);
			OPENCL_RETURN_ON_ERROR(result);

			if (buildStatus != CL_BUILD_SUCCESS)
			{
				std::string buildLog = {};
				result               = GetBuildLog(program, device, buildLog);
				OPENCL_RETURN_ON_ERROR(result);

				std::cout << buildLog;
			}
		}
	}
	else
	{
		OPENCL_PRINT_ON_ERROR(result);
	}

	return result;
}


} // program