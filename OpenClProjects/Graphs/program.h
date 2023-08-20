#pragma once

#include <CL/cl.h>

#include <array>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "debug.h"


namespace program
{


constexpr size_t nKernels = 1;

const std::array<const std::string, nKernels> kernelPaths
{
	"vfadd.cl"
};


#ifdef _DEBUG


constexpr char buildOptions[] = "-cl-opt-disable -Werror -cl-std=CL2.0 -g";


#else


constexpr char buildOptions[] = "-Werror -cl-std=CL2.0";


#endif // _DEBUG


cl_int Build(const cl_context context, cl_program& program)
{
	cl_int  result   = CL_SUCCESS;
	cl_uint nDevices = 0;

	// Query number of devices associated with the context.
	result = clGetContextInfo(
		context,
		CL_CONTEXT_NUM_DEVICES,
		sizeof(nDevices),
		&nDevices,
		nullptr
	);
	OPENCL_RETURN_ON_ERROR(result);

	std::vector<cl_device_id> devices(nDevices);

	// Query the devices associated with the context.
	result = clGetContextInfo(
		context,
		CL_CONTEXT_DEVICES,
		devices.size() * sizeof(decltype(devices)::value_type),
		devices.data(),
		nullptr
	);
	OPENCL_RETURN_ON_ERROR(result);

	std::array<std::string, nKernels> kernels = {};

	for (size_t i = 0; i < nKernels; i++)
	{
		std::ifstream kernelIfStream{ kernelPaths[i] };

		if (!kernelIfStream.is_open())
		{
			std::cerr << "Failed to open kernel: " << kernelPaths[i] << "\n";
			return CL_BUILD_PROGRAM_FAILURE;
		}

		std::ostringstream kernelOsStream;
		kernelOsStream << kernelIfStream.rdbuf();

		kernels[i] = std::move(kernelOsStream.str());
	}

	const char* kernelCStrs[nKernels];
	size_t      kernelLens[nKernels];

	for (size_t i = 0; i < nKernels; i++)
	{
		kernelCStrs[i] = kernels[i].c_str();
		kernelLens[i]  = kernels[i].size();
	}

	program = clCreateProgramWithSource(
		context,
		nKernels,
		kernelCStrs,
		kernelLens,
		&result
	);
	OPENCL_RETURN_ON_ERROR(result);

	result = clBuildProgram(
		program,
		nDevices,
		devices.data(),
		buildOptions,
		nullptr,
		nullptr
	);
	
	if (result == CL_BUILD_PROGRAM_FAILURE)
	{
		for (const auto device : devices)
		{
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

			std::cout << paramValue.data() << "\n";
		}
	}
	else
	{
		OPENCL_PRINT_ON_ERROR(result);
	}

	return result;
}


} // program