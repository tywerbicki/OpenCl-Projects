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

	std::vector<cl_device_id> devices{ nDevices };

	// Query the devices associated with the context.
	result = clGetContextInfo(
		context,
		CL_CONTEXT_DEVICES,
		devices.size() * sizeof(decltype(devices)::value_type),
		devices.data(),
		nullptr
	);
	OPENCL_RETURN_ON_ERROR(result);

	constexpr size_t nKernels = 1;

	const std::array<const std::string, nKernels> kernelPaths
	{
		"vfadd.cl"
	};

	std::array<std::string, nKernels> kernels;

	for (size_t i = 0; i < nKernels; i++)
	{
		std::ifstream f{ kernelPaths[i] };

		if (!f.is_open())
		{
			std::cerr << "Failed to open kernel: " << kernelPaths[i] << "\n";
			return CL_BUILD_PROGRAM_FAILURE;
		}

		std::ostringstream kernelOsStream;
		kernelOsStream << f.rdbuf();

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

	return result;
}


} // program